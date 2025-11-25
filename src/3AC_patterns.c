#include "3AC_patterns.h"

#include <stdbool.h>
#include <stdio.h>

#include "error.h"
#include "expr_parser.h"
#include "helper.h"
#include "parser.h"
#include "scanner.h"
#include "semantic.h"
#include "symtable.h"

void generate_program_entrypoint(ThreeACList *list) {
    emit(NO_OP, NULL, NULL, NULL, list);
    Operand* startJumpLabel = create_operand_from_label("%start");
    emit(OP_JUMP, startJumpLabel, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);

    emit_comment("####################", list);
    emit_comment("Program entry point", list);
    emit_comment("####################", list);
    
    emit(OP_LABEL, startJumpLabel, NULL, NULL , list);
    Operand* mainLabel = create_operand_from_label("main$0%func");
    emit(OP_CREATEFRAME, NULL, NULL, NULL, list);
    emit(OP_PUSHFRAME, NULL, NULL, NULL, list);
    emit(OP_CALL, mainLabel, NULL, NULL, list);
    emit(OP_POPFRAME, NULL, NULL, NULL, list);
    
    Operand *exitValue = create_operand_from_constant_int(0);
    emit(OP_EXIT, exitValue, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);
}

void generate_eof(ThreeACList *list) {
    // This can be used for any cleanup or finalization code.
    // For now, it will also handle built-in function implementations.
}

void generate_if_else(FILE *file, tToken *currentToken, tSymTableStack *stack, bool in_loop) {
    get_next_token(file, currentToken); // consume 'if'
    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("If statement condition", &threeACcode);
    parse_expression(file, currentToken, stack);

    Operand* expr_val_if = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, expr_val_if, NULL, NULL, &threeACcode);
    emit(OP_POPS, expr_val_if, NULL, NULL, &threeACcode);
    generate_truthiness_check(&threeACcode, expr_val_if);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    char *label1_str = threeAC_create_label(&threeACcode);
    Operand *label1 = create_operand_from_label(label1_str);

    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, label1, NULL, NULL, &threeACcode);

    emit_comment("If-block", &threeACcode);
    parse_block(file, currentToken, stack, false);
    skip_optional_eol(currentToken, file);

    if ((*currentToken)->type == T_KW_ELSE) {
        get_next_token(file, currentToken); // consume 'else'
        char *label2_str = threeAC_create_label(&threeACcode);
        Operand *label2 = create_operand_from_label(label2_str);

        emit(OP_JUMP, label2, NULL, NULL, &threeACcode);
        emit(OP_LABEL, label1, NULL, NULL, &threeACcode);

        emit_comment("Else-block", &threeACcode);
        parse_block(file, currentToken, stack, false);
        emit(OP_LABEL, label2, NULL, NULL, &threeACcode);
        free(label2_str);
    } else {
        emit(OP_LABEL, label1, NULL, NULL, &threeACcode);
    }

    free(label1_str);
    emit_comment("If statement end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
}

void generate_while(FILE *file, tToken *currentToken, tSymTableStack *stack) {
    get_next_token(file, currentToken);
    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("While loop start", &threeACcode);

    char *loop_start_label_str = threeAC_create_label(&threeACcode);
    char *loop_end_label_str = threeAC_create_label(&threeACcode);
    Operand *loop_start_label = create_operand_from_label(loop_start_label_str);
    Operand *loop_end_label = create_operand_from_label(loop_end_label_str);

    emit(OP_LABEL, loop_start_label, NULL, NULL, &threeACcode);
    emit_comment("While condition", &threeACcode);

    parse_expression(file, currentToken, stack);

    Operand* expr_val_while = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, expr_val_while, NULL, NULL, &threeACcode);
    emit(OP_POPS, expr_val_while, NULL, NULL, &threeACcode);
    generate_truthiness_check(&threeACcode, expr_val_while);

    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, loop_end_label, NULL, NULL, &threeACcode);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    emit_comment("While body", &threeACcode);
    parse_block(file, currentToken, stack, false);

    emit(OP_JUMP, loop_start_label, NULL, NULL, &threeACcode);
    emit(OP_LABEL, loop_end_label, NULL, NULL, &threeACcode);

    free(loop_start_label_str);
    free(loop_end_label_str);
    emit_comment("While loop end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
}

void generate_function_call(tSymTable *g_symtable, tSymTableStack *stack, tToken *func_token, FILE *file, tToken *currentToken) {
    // This is a simplified version. The full logic from parse_function_call needs to be adapted.
    // For now, this is a placeholder.
}

void generate_return(FILE *file, tToken *currentToken, tSymTableStack *stack) {
    get_next_token(file, currentToken);

    if ((*currentToken)->type != T_EOL) {
        parse_expression(file, currentToken, stack);
        Operand* retval_var = create_operand_from_variable("%retval", false);
        emit(OP_POPS, retval_var, NULL, NULL, &threeACcode);
        emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
    } else {
        emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
    }
}

tDataType generate_ifj_write(tSymTableStack *stack) {
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.write call", &threeACcode);

    Operand* write_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, write_arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, write_arg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, write_arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    Operand* after_checking = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, after_checking, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, write_arg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, after_checking, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, write_arg, write_arg, NULL, &threeACcode);
    emit(OP_LABEL, after_checking, NULL, NULL, &threeACcode);

    emit(OP_WRITE, write_arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_nil(), NULL, NULL, &threeACcode);

    return TYPE_NULL;
}

tDataType generate_ifj_read_str(tSymTableStack *stack) {
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.read_str call", &threeACcode);

    Operand* result_var = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, result_var, NULL, NULL, &threeACcode);
    emit(OP_READ, result_var, create_operand_from_type("string"), NULL, &threeACcode);
    emit(OP_PUSHS, result_var, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}


tDataType generate_ifj_read_num(tSymTableStack *stack) {
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.read_num call", &threeACcode);


    Operand* result_var = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, result_var, NULL, NULL, &threeACcode);
    emit(OP_READ, result_var, create_operand_from_type("float"), NULL, &threeACcode);

    Operand* result_is_not_int_or_null = create_operand_from_label(threeAC_create_label(&threeACcode));
    
    emit(OP_PUSHS, result_var, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, result_is_not_int_or_null, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, result_var, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, result_is_not_int_or_null, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, result_var, result_var, NULL, &threeACcode);

    emit(OP_LABEL, result_is_not_int_or_null, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, result_var, NULL, NULL, &threeACcode);
    
    return TYPE_NUM;
}

tDataType generate_ifj_strcmp(tSymTableStack *stack) {
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.strcmp call", &threeACcode);

    Operand *s2_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, s2_arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, s2_arg, NULL, NULL, &threeACcode);

    Operand *s1_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, s1_arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, s1_arg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, s1_arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2_arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    Operand* label_type_error = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_continue_strcmp = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);
    emit(OP_JUMP, label_continue_strcmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_type_error, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_continue_strcmp, NULL, NULL, &threeACcode);

    Operand* result_cmp = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, result_cmp, NULL, NULL, &threeACcode);

    Operand* label_equal = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_less = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_greater = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_end_cmp = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, s1_arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2_arg, NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, label_equal, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, s1_arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2_arg, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, label_less, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_greater, NULL, NULL, &threeACcode);
    emit(OP_MOVE, result_cmp, create_operand_from_constant_int(1), NULL, &threeACcode);
    emit(OP_JUMP, label_end_cmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_equal, NULL, NULL, &threeACcode);
    emit(OP_MOVE, result_cmp, create_operand_from_constant_int(0), NULL, &threeACcode);
    emit(OP_JUMP, label_end_cmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_less, NULL, NULL, &threeACcode);
    emit(OP_MOVE, result_cmp, create_operand_from_constant_int(-1), NULL, &threeACcode);
    emit(OP_JUMP, label_end_cmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_end_cmp, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, result_cmp, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}


tDataType generate_ifj_ord(tSymTableStack *stack) {
        emit(NO_OP, NULL, NULL, NULL, &threeACcode);
        emit_comment("Ifj.ord call", &threeACcode);


        Operand *i_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, i_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, i_arg, NULL, NULL, &threeACcode);

        Operand *s_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, s_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, s_arg, NULL, NULL, &threeACcode);
        
        Operand* type_i = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, type_i, NULL, NULL, &threeACcode);
        emit(OP_TYPE, type_i, i_arg, NULL, &threeACcode);

        Operand* label_type_error = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_continue_ord = create_operand_from_label(threeAC_create_label(&threeACcode));

        emit(OP_PUSHS, s_arg, NULL, NULL, &threeACcode);
        emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);

        emit(OP_PUSHS, type_i, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_continue_ord, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, type_i, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);
        emit(OP_FLOAT2INT, i_arg, i_arg, NULL, &threeACcode);
        emit(OP_JUMP, label_continue_ord, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_type_error, NULL, NULL, &threeACcode);
        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL, &threeACcode);

        emit(OP_JUMP, label_continue_ord, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_continue_ord, NULL, NULL, &threeACcode);

        Operand* len_s = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, len_s, NULL, NULL, &threeACcode);
        emit(OP_STRLEN, len_s, s_arg, NULL, &threeACcode);

        Operand* result_ord = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, result_ord, NULL, NULL, &threeACcode);

        Operand* label_return_zero = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_end_ord = create_operand_from_label(threeAC_create_label(&threeACcode));

        // (len_s == 0)
        emit(OP_PUSHS, len_s, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_zero, NULL, NULL, &threeACcode);

        // Check if i < 0
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
        emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_zero, NULL, NULL, &threeACcode);

        // Check if i >= len_s
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, len_s, NULL, NULL, &threeACcode);
        emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_zero, NULL, NULL, &threeACcode);

        emit(OP_STRI2INT, result_ord, s_arg, i_arg, &threeACcode);
        emit(OP_JUMP, label_end_ord, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_return_zero, NULL, NULL, &threeACcode);
        emit(OP_MOVE, result_ord, create_operand_from_constant_int(0), NULL, &threeACcode);

        emit(OP_LABEL, label_end_ord, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, result_ord, NULL, NULL, &threeACcode);

        return TYPE_NUM; 
}

tDataType generate_ifj_floor(tSymTableStack *stack) {
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.floor call", &threeACcode);

    Operand* label_is_int = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_is_not_num = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_end_floor = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand* arg_val = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, arg_val, NULL, NULL, &threeACcode);
    emit(OP_POPS, arg_val, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, arg_val, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_string("int"),  NULL,NULL, &threeACcode);
    emit(OP_JUMPIFEQS, label_is_int, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, arg_val, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_string("float"),  NULL,NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, label_is_not_num, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, arg_val, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INTS, NULL, NULL, NULL, &threeACcode); 

    emit(OP_JUMP, label_end_floor, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_is_int, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, arg_val, NULL, NULL, &threeACcode);

    emit(OP_JUMP, label_end_floor, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_is_not_num, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_end_floor, NULL, NULL, &threeACcode);


    return TYPE_NUM;
}

tDataType generate_ifj_str(tSymTableStack *stack) {
        emit(NO_OP, NULL, NULL, NULL, &threeACcode);
        emit_comment("Ifj.str call", &threeACcode);
        
        Operand* arg_val = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, arg_val, NULL, NULL, &threeACcode);
        emit(OP_POPS, arg_val, NULL, NULL, &threeACcode);

        Operand* result_str = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, result_str, NULL, NULL, &threeACcode);

        Operand* type_check_var = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, type_check_var, NULL, NULL, &threeACcode);

        Operand* label_end = create_operand_from_label(threeAC_create_label(&threeACcode));

        Operand* label_is_string = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_is_int = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_is_float = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_is_nil = create_operand_from_label(threeAC_create_label(&threeACcode));


        emit(OP_TYPE, type_check_var, arg_val, NULL, &threeACcode);

        emit(OP_JUMPIFEQ, label_is_nil, arg_val, create_operand_from_constant_nil(), &threeACcode);
        emit(OP_JUMPIFEQ, label_is_string, type_check_var, create_operand_from_constant_string("string"), &threeACcode);
        emit(OP_JUMPIFEQ, label_is_int, type_check_var, create_operand_from_constant_string("int"), &threeACcode);
        emit(OP_JUMPIFEQ, label_is_float, type_check_var, create_operand_from_constant_string("float"), &threeACcode);

        emit(OP_MOVE, result_str, create_operand_from_constant_string(""), NULL, &threeACcode); // Empty string or error
        emit(OP_JUMP, label_end, NULL, NULL, &threeACcode);

        // Handle string
        emit(OP_LABEL, label_is_string, NULL, NULL, &threeACcode);
        emit(OP_MOVE, result_str, arg_val, NULL, &threeACcode);
        emit(OP_JUMP, label_end, NULL, NULL, &threeACcode);

        // Handle int
        emit(OP_LABEL, label_is_int, NULL, NULL, &threeACcode);
        emit(OP_INT2STR, result_str, arg_val, NULL, &threeACcode);
        emit(OP_JUMP, label_end, NULL, NULL, &threeACcode);

        // Handle float
        emit(OP_LABEL, label_is_float, NULL, NULL, &threeACcode);
        emit(OP_FLOAT2STR, result_str, arg_val, NULL, &threeACcode);
        emit(OP_JUMP, label_end, NULL, NULL, &threeACcode);

        // Handle nil
        emit(OP_LABEL, label_is_nil, NULL, NULL, &threeACcode);
        emit(OP_MOVE, result_str, create_operand_from_constant_string("null"), NULL, &threeACcode);
        emit(OP_JUMP, label_end, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_end, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, result_str, NULL, NULL, &threeACcode);

        return TYPE_STRING;
}

tDataType generate_ifj_length(tSymTableStack *stack) {
        emit(NO_OP, NULL, NULL, NULL, &threeACcode);
        emit_comment("Ifj.length call", &threeACcode);

        Operand* str_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, str_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, str_arg, NULL, NULL, &threeACcode);

        emit(OP_PUSHS, str_arg, NULL, NULL, &threeACcode);
        emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

        Operand* label_continue_length = create_operand_from_label(threeAC_create_label(&threeACcode));

        emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_continue_length, NULL, NULL, &threeACcode);

        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_continue_length, NULL, NULL, &threeACcode);
      
        Operand* result_len = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, result_len, NULL, NULL, &threeACcode);

        emit(OP_STRLEN, result_len, str_arg, NULL, &threeACcode);
        emit(OP_PUSHS, result_len, NULL, NULL, &threeACcode);

        return TYPE_NUM;
}

tDataType generate_ifj_substring(tSymTableStack *stack) {
        emit(NO_OP, NULL, NULL, NULL, &threeACcode);
        emit_comment("Ifj.substring call", &threeACcode);

        Operand *j_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, j_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, j_arg, NULL, NULL, &threeACcode);

        Operand *i_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, i_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, i_arg, NULL, NULL, &threeACcode);

        Operand *s_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, s_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, s_arg, NULL, NULL, &threeACcode);

        Operand* type_i = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        Operand* type_j = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, type_i, NULL, NULL, &threeACcode);
        emit(OP_DEFVAR, type_j, NULL, NULL, &threeACcode);
        emit(OP_TYPE, type_i, i_arg, NULL, &threeACcode);
        emit(OP_TYPE, type_j, j_arg, NULL, &threeACcode);

        emit(OP_PUSHS, s_arg, NULL, NULL, &threeACcode);
        emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

        Operand* label_param_type_error = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_continue_substring = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_check_j_end = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_check_i_end = create_operand_from_label(threeAC_create_label(&threeACcode));

        emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_param_type_error, NULL, NULL, &threeACcode);

        emit(OP_JUMPIFEQ, label_check_j_end, type_j, create_operand_from_constant_string("int"), &threeACcode);
        emit(OP_JUMPIFNEQ, label_param_type_error, type_j, create_operand_from_constant_string("float"), &threeACcode);
        emit(OP_PUSHS, j_arg, NULL, NULL, &threeACcode);
        emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_param_type_error, NULL, NULL, &threeACcode);
        emit(OP_FLOAT2INT, j_arg, j_arg, NULL, &threeACcode);
        emit(OP_LABEL, label_check_j_end, NULL, NULL, &threeACcode);
      
        emit(OP_JUMPIFEQ, label_continue_substring, type_i, create_operand_from_constant_string("int"), &threeACcode);
        emit(OP_JUMPIFNEQ, label_param_type_error, type_i, create_operand_from_constant_string("float"), &threeACcode);
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_param_type_error, NULL, NULL, &threeACcode);
        emit(OP_FLOAT2INT, i_arg, i_arg, NULL, &threeACcode);
        emit(OP_LABEL, label_check_i_end, NULL, NULL, &threeACcode);

        emit(OP_JUMP, label_continue_substring, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_param_type_error, NULL, NULL, &threeACcode);
        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_continue_substring, NULL, NULL, &threeACcode);

        // Get length of s
        Operand* len_s = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, len_s, NULL, NULL, &threeACcode);
        emit(OP_STRLEN, len_s, s_arg, NULL, &threeACcode);

        // Labels for null return
        Operand* label_return_null = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_end_substring = create_operand_from_label(threeAC_create_label(&threeACcode));

        // Boundary checks
        // i < 0
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
        emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_null, NULL, NULL, &threeACcode);

        // j < 0
        emit(OP_PUSHS, j_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
        emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_null, NULL, NULL, &threeACcode);

        // i > j
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, j_arg, NULL, NULL, &threeACcode);
        emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_null, NULL, NULL, &threeACcode);

        // i >= Ifj.length(s)  => NOT (i < Ifj.length(s))
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, len_s, NULL, NULL, &threeACcode);
        emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_null, NULL, NULL, &threeACcode);

        // j > Ifj.length(s)
        emit(OP_PUSHS, j_arg, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, len_s, NULL, NULL, &threeACcode);
        emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_return_null, NULL, NULL, &threeACcode);

        Operand* result_str = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, result_str, NULL, NULL, &threeACcode);
        emit(OP_MOVE, result_str, create_operand_from_constant_string(""), NULL, &threeACcode);

        Operand* loop_counter = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, loop_counter, NULL, NULL, &threeACcode);
        emit(OP_MOVE, loop_counter, i_arg, NULL, &threeACcode);

        Operand* current_char = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, current_char, NULL, NULL, &threeACcode);

        Operand* loop_start_label_sub = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* loop_end_label_sub = create_operand_from_label(threeAC_create_label(&threeACcode));

        emit(OP_LABEL, loop_start_label_sub, NULL, NULL, &threeACcode);

        emit(OP_PUSHS, loop_counter, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, j_arg, NULL, NULL, &threeACcode);
        emit(OP_LTS, NULL, NULL, NULL, &threeACcode); // Result (bool) is on stack
        emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, loop_end_label_sub, NULL, NULL, &threeACcode);

        emit(OP_GETCHAR, current_char, s_arg, loop_counter, &threeACcode);
        emit(OP_CONCAT, result_str, result_str, current_char, &threeACcode);

        emit(OP_ADD, loop_counter, loop_counter, create_operand_from_constant_int(1), &threeACcode);
        emit(OP_JUMP, loop_start_label_sub, NULL, NULL, &threeACcode);
        emit(OP_LABEL, loop_end_label_sub, NULL, NULL, &threeACcode);

        emit(OP_PUSHS, result_str, NULL, NULL, &threeACcode);
        emit(OP_JUMP, label_end_substring, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_return_null, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_nil(), NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_end_substring, NULL, NULL, &threeACcode);


        return TYPE_STRING;
}

tDataType generate_ifj_chr(tSymTableStack *stack) {
        emit(NO_OP, NULL, NULL, NULL, &threeACcode);
        emit_comment("Ifj.chr call", &threeACcode);

        Operand *i_arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, i_arg, NULL, NULL, &threeACcode);
        emit(OP_POPS, i_arg, NULL, NULL, &threeACcode);

        Operand* type_i = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, type_i, NULL, NULL, &threeACcode);
        emit(OP_TYPE, type_i, i_arg, NULL, &threeACcode);

        Operand* label_type_error = create_operand_from_label(threeAC_create_label(&threeACcode));
        Operand* label_continue_chr = create_operand_from_label(threeAC_create_label(&threeACcode));

        emit(OP_PUSHS, type_i, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFEQS, label_continue_chr, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, type_i, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
        emit(OP_JUMPIFNEQS, label_type_error, NULL, NULL, &threeACcode);
        emit(OP_FLOAT2INT, i_arg, i_arg, NULL, &threeACcode);
        emit(OP_JUMP, label_continue_chr, NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_type_error, NULL, NULL, &threeACcode);
        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL, &threeACcode);

        emit(OP_LABEL, label_continue_chr, NULL, NULL, &threeACcode);

        emit(OP_PUSHS, i_arg, NULL, NULL, &threeACcode);
        emit(OP_INT2CHARS, NULL, NULL, NULL, &threeACcode);

        return TYPE_STRING;
}

void generate_truthiness_check(ThreeACList *list, Operand *expr_val) {
    Operand* final_bool_result = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, final_bool_result, NULL, NULL, list);

    Operand* label_is_null = create_operand_from_label(threeAC_create_label(list));
    Operand* label_is_bool = create_operand_from_label(threeAC_create_label(list));
    Operand* label_is_other = create_operand_from_label(threeAC_create_label(list));
    Operand* label_end_truthiness = create_operand_from_label(threeAC_create_label(list));

    // Check if null
    emit(OP_JUMPIFEQ, label_is_null, expr_val, create_operand_from_constant_nil(), list);

    // Check if boolean (using TYPE instruction)
    Operand* type_check_var = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type_check_var, NULL, NULL, list);
    emit(OP_TYPE, type_check_var, expr_val, NULL, list);

    emit(OP_JUMPIFEQ, label_is_bool, type_check_var, create_operand_from_constant_string("bool"), list);

    // If not null and not bool, it's true
    emit(OP_JUMP, label_is_other, NULL, NULL, list);

    // Case: is null
    emit(OP_LABEL, label_is_null, NULL, NULL, list);
    emit(OP_MOVE, final_bool_result, create_operand_from_constant_bool(false), NULL, list);
    emit(OP_JUMP, label_end_truthiness, NULL, NULL, list);

    // Case: is boolean
    emit(OP_LABEL, label_is_bool, NULL, NULL, list);
    emit(OP_MOVE, final_bool_result, expr_val, NULL, list);
    emit(OP_JUMP, label_end_truthiness, NULL, NULL, list);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, label_is_other, NULL, NULL, list);
    emit(OP_MOVE, final_bool_result, create_operand_from_constant_bool(true), NULL, list);
    emit(OP_JUMP, label_end_truthiness, NULL, NULL, list);

    emit(OP_LABEL, label_end_truthiness, NULL, NULL, list);
    emit(OP_PUSHS, final_bool_result, NULL, NULL, list); // Push the final boolean result
}

void generate_add_op(ThreeACList *list) {
    Operand* op2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    Operand* op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

    Operand* type1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type1, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);

    Operand* type2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type2, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    Operand* end_add_label = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand* num_add_label_check = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* num_add_label_type = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand* type_error_label = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, num_add_label_check, NULL, NULL, &threeACcode);

    Operand* result_str = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, result_str, NULL, NULL, &threeACcode);
    emit(OP_CONCAT, result_str, op1, op2, &threeACcode);
    emit(OP_PUSHS, result_str, NULL, NULL, &threeACcode);
    emit(OP_JUMP, end_add_label, NULL, NULL, &threeACcode);

    emit(OP_LABEL, num_add_label_check, NULL, NULL, &threeACcode);

    Operand* operand1_check_end = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand1_check_end, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, type_error_label, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);

    emit(OP_LABEL, operand1_check_end, NULL, NULL, &threeACcode);

    Operand* operand2_check_end = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand2_check_end, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, type_error_label, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);

    emit(OP_LABEL, operand2_check_end, NULL, NULL, &threeACcode);

    emit(OP_LABEL, num_add_label_type, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_ADDS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, end_add_label, NULL, NULL, &threeACcode);

    emit(OP_LABEL, type_error_label, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL, &threeACcode);
  
    emit(OP_LABEL, end_add_label, NULL, NULL, &threeACcode);
}

void generate_mult_op(ThreeACList *list) {
    Operand* op2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    Operand* op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

    Operand* type1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type1, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);

    Operand* type2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type2, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    Operand* end_mult_label = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* num_mult_label_check = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* num_mult_label_type = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* type_error_label = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ORS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ORS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);

    emit(OP_ORS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);

    emit(OP_JUMPIFNEQS, num_mult_label_check, NULL, NULL, &threeACcode);

    Operand* replace_operand = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    Operand* replace_end_label = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_DEFVAR, replace_operand, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQS, replace_end_label, NULL, NULL, &threeACcode);

    emit(OP_MOVE, replace_operand, op1, NULL, &threeACcode);
    emit(OP_MOVE, op1, op2, NULL, &threeACcode);
    emit(OP_MOVE, op2, replace_operand, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);


    emit(OP_LABEL, replace_end_label, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
  
    Operand* op2_is_int_label = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_JUMPIFNEQS, op2_is_int_label, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    emit(OP_LABEL, op2_is_int_label, NULL, NULL, &threeACcode);

    Operand* result_str = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, result_str, NULL, NULL, &threeACcode);
    emit(OP_MOVE, result_str, create_operand_from_constant_string(""), NULL, &threeACcode);

    Operand* loop_start = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* loop_end = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* condition = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, condition, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loop_start, NULL, NULL, &threeACcode);

    emit(OP_GT, condition, op2, create_operand_from_constant_int(0), &threeACcode);
    emit(OP_JUMPIFNEQ, loop_end, condition, create_operand_from_constant_bool(true), &threeACcode);

    emit(OP_CONCAT, result_str, result_str, op1, &threeACcode);

    emit(OP_SUB, op2, op2, create_operand_from_constant_int(1), &threeACcode);
    emit(OP_JUMP, loop_start, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loop_end, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, result_str, NULL, NULL, &threeACcode);

    emit(OP_JUMP, end_mult_label, NULL, NULL, &threeACcode);

    emit(OP_LABEL, num_mult_label_check, NULL, NULL, &threeACcode);

    Operand* operand1_check_end = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand1_check_end, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, type_error_label, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);

    emit(OP_LABEL, operand1_check_end, NULL, NULL, &threeACcode);

    Operand* operand2_check_end = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand2_check_end, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, type_error_label, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);

    emit(OP_LABEL, operand2_check_end, NULL, NULL, &threeACcode);

    emit(OP_LABEL, num_mult_label_type, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_MULS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, end_mult_label, NULL, NULL, &threeACcode);

    emit(OP_LABEL, type_error_label, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL, &threeACcode);
  
    emit(OP_LABEL, end_mult_label, NULL, NULL, &threeACcode);
}


void generate_numeric_op(ThreeACList *list, char* op) {
    Operand* op2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op2, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);

    Operand* op1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op1, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);

    Operand* type1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type1, NULL, NULL, list);
    emit(OP_TYPE, type1, op1, NULL, list);

    Operand* type2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type2, NULL, NULL, list);
    emit(OP_TYPE, type2, op2, NULL, list);

    Operand* int_type = create_operand_from_constant_string("int");

    Operand* is_int1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, is_int1, NULL, NULL, list);
    emit(OP_EQ, is_int1, type1, int_type, list);
    Operand* op1_ok_label = create_operand_from_label(threeAC_create_label(list));
    emit(OP_JUMPIFNEQ, op1_ok_label, is_int1, create_operand_from_constant_bool(true), list);
    emit(OP_INT2FLOAT, op1, op1, NULL, list);
    emit(OP_LABEL, op1_ok_label, NULL, NULL, list);

    Operand* is_int2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, is_int2, NULL, NULL, list);
    emit(OP_EQ, is_int2, type2, int_type, list);
    Operand* op2_ok_label = create_operand_from_label(threeAC_create_label(list));
    emit(OP_JUMPIFNEQ, op2_ok_label, is_int2, create_operand_from_constant_bool(true), list);
    emit(OP_INT2FLOAT, op2, op2, NULL, list);
    emit(OP_LABEL, op2_ok_label, NULL, NULL, list);

    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_PUSHS, op2, NULL, NULL, list);

    if (strcmp(op, "-") == 0) {
        emit(OP_SUBS, NULL, NULL, NULL, list);
    } else if (strcmp(op, "/") == 0) {
        emit(OP_DIVS, NULL, NULL, NULL, list);
    } else if (strcmp(op, "*") == 0) {
        emit(OP_MULS, NULL, NULL, NULL, list);
    } else if (strcmp(op, "+") == 0) {
        emit(OP_ADDS, NULL, NULL, NULL, list);
    }
}

void generate_relational_op(ThreeACList *list, char* op) {
    Operand* op2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op2, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);

    Operand* op1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op1, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);

    Operand* type1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type1, NULL, NULL, list);
    emit(OP_TYPE, type1, op1, NULL, list);

    Operand* type2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type2, NULL, NULL, list);
    emit(OP_TYPE, type2, op2, NULL, list);

    if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0) {
        Operand* type_error_label = create_operand_from_label(threeAC_create_label(list));
        Operand* after_num_type_check_label = create_operand_from_label(threeAC_create_label(list));

        emit(OP_PUSHS, type1, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_PUSHS, type1, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_ORS, NULL, NULL, NULL, list);

        emit(OP_PUSHS, type2, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_PUSHS, type2, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_ORS, NULL, NULL, NULL, list);

        emit(OP_ANDS, NULL, NULL, NULL, list);

        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, list);
        emit(OP_JUMPIFNEQS, type_error_label, NULL, NULL, list);

        emit(OP_JUMP, after_num_type_check_label, NULL, NULL, list);

        emit(OP_LABEL, type_error_label, NULL, NULL, list);
        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL, list);

        emit(OP_LABEL, after_num_type_check_label, NULL, NULL, list);
    }

    Operand* op1_to_float_if_int = create_operand_from_label(threeAC_create_label(list));
    emit(OP_PUSHS, type1, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, op1_to_float_if_int, NULL, NULL, list);

    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_INT2FLOATS, NULL, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);
    emit(OP_MOVE, type1, create_operand_from_constant_string("float"), NULL, list);

    emit(OP_LABEL, op1_to_float_if_int, NULL, NULL, list);

    Operand* op2_to_float_if_int = create_operand_from_label(threeAC_create_label(list));
    emit(OP_PUSHS, type2, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, op2_to_float_if_int, NULL, NULL, list);

    emit(OP_PUSHS, op2, NULL, NULL, list);
    emit(OP_INT2FLOATS, NULL, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);
    emit(OP_MOVE, type2, create_operand_from_constant_string("float"), NULL, list);

    emit(OP_LABEL, op2_to_float_if_int, NULL, NULL, list);
    Operand* perform_op_label_on_same_type = create_operand_from_label(threeAC_create_label(list));
    Operand* end_rel_op_label = create_operand_from_label(threeAC_create_label(list));

    emit(OP_JUMPIFEQ, perform_op_label_on_same_type, type1, type2, list);
    if (strcmp(op, "!=") == 0) 
    {
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, list);
    }
    else {
        emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, list);
    } 

    emit(OP_JUMP, end_rel_op_label, NULL, NULL, list);

    emit(OP_LABEL, perform_op_label_on_same_type, NULL, NULL, list);
    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_PUSHS, op2, NULL, NULL, list);

    OperationType opType;
    bool use_not = false;

    if (strcmp(op, "<") == 0) opType = OP_LTS;
    else if (strcmp(op, ">") == 0) opType = OP_GTS;
    else if (strcmp(op, "==") == 0) opType = OP_EQS;
    else if (strcmp(op, "!=") == 0) { opType = OP_EQS; use_not = true; }
    else if (strcmp(op, "<=") == 0) { opType = OP_GTS; use_not = true; }
    else if (strcmp(op, ">=") == 0) { opType = OP_LTS; use_not = true; }
    
    emit(opType, NULL, NULL, NULL, list);
    if (use_not) {
        emit(OP_NOTS, NULL, NULL, NULL, list);
    }
    emit(OP_LABEL, end_rel_op_label, NULL, NULL, list);
}

