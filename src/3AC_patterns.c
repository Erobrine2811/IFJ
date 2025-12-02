#include <stdbool.h>
#include <stdio.h>

#include "3AC_patterns.h"
#include "error.h"
#include "expr_parser.h"
#include "parser.h"
#include "scanner.h"
#include "symtable.h"

void generate_program_entrypoint(ThreeACList *list)
{
    emit(NO_OP, NULL, NULL, NULL, list);
    Operand *startJumpLabel = create_operand_from_label("%start");
    emit(OP_JUMP, startJumpLabel, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);

    emit_comment("####################", list);
    emit_comment("Program entry point", list);
    emit_comment("####################", list);

    emit(OP_LABEL, startJumpLabel, NULL, NULL, list);
    Operand *mainLabel = create_operand_from_label("main$0%func");
    emit(OP_CREATEFRAME, NULL, NULL, NULL, list);
    emit(OP_PUSHFRAME, NULL, NULL, NULL, list);
    emit(OP_CALL, mainLabel, NULL, NULL, list);
    emit(OP_POPFRAME, NULL, NULL, NULL, list);

    Operand *exitValue = create_operand_from_constant_int(0);
    emit(OP_EXIT, exitValue, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);
}

void generate_eof(ThreeACList *list)
{
    // This can be used for any cleanup or finalization code.
    // For now, it will also handle built-in function implementations.
}

void generate_while(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);
    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("While loop start", &threeACcode);

    char *loopStartLabelStr = threeAC_create_label(&threeACcode);
    char *loopEndLabelStr = threeAC_create_label(&threeACcode);
    Operand *loopStartLabel = create_operand_from_label(loopStartLabelStr);
    Operand *loopEndLabel = create_operand_from_label(loopEndLabelStr);

    emit(OP_LABEL, loopStartLabel, NULL, NULL, &threeACcode);
    emit_comment("While condition", &threeACcode);

    parse_expression(file, currentToken, stack);

    Operand *exprValWhile = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, exprValWhile, NULL, NULL, &threeACcode);
    emit(OP_POPS, exprValWhile, NULL, NULL, &threeACcode);
    generate_truthiness_check(&threeACcode, exprValWhile);

    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, loopEndLabel, NULL, NULL, &threeACcode);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    emit_comment("While body", &threeACcode);
    parse_block(file, currentToken, stack, false);

    emit(OP_JUMP, loopStartLabel, NULL, NULL, &threeACcode);
    emit(OP_LABEL, loopEndLabel, NULL, NULL, &threeACcode);

    free(loopStartLabelStr);
    free(loopEndLabelStr);
    emit_comment("While loop end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
}

void generate_function_call(tSymTable *g_symtable, tSymTableStack *stack, tToken *func_token,
                            FILE *file, tToken *currentToken)
{
    // This is a simplified version. The full logic from parse_function_call needs to be adapted.
    // For now, this is a placeholder.
}

void generate_return(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isOneLine)
{
    if (!isOneLine)
    {
        get_next_token(file, currentToken);
    }

    parse_expression(file, currentToken, stack);
    Operand *retvalVar = create_operand_from_variable("%retval", false);
    emit(OP_POPS, retvalVar, NULL, NULL, &threeACcode);
    emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
}

tDataType generate_ifj_write(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.write call", &threeACcode);

    Operand *writeArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, writeArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, writeArg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, writeArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    Operand *afterChecking = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, afterChecking, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, writeArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, afterChecking, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, writeArg, writeArg, NULL, &threeACcode);
    emit(OP_LABEL, afterChecking, NULL, NULL, &threeACcode);

    emit(OP_WRITE, writeArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_nil(), NULL, NULL, &threeACcode);

    return TYPE_NULL;
}

tDataType generate_ifj_read_str(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.read_str call", &threeACcode);

    Operand *resultVar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, resultVar, NULL, NULL, &threeACcode);
    emit(OP_READ, resultVar, create_operand_from_type("string"), NULL, &threeACcode);
    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

tDataType generate_ifj_read_num(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.read_num call", &threeACcode);

    Operand *resultVar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultVar, NULL, NULL, &threeACcode);
    emit(OP_READ, resultVar, create_operand_from_type("float"), NULL, &threeACcode);

    Operand *resultIsNotIntOrNull = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, resultIsNotIntOrNull, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, resultIsNotIntOrNull, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, resultVar, resultVar, NULL, &threeACcode);

    emit(OP_LABEL, resultIsNotIntOrNull, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_strcmp(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.strcmp call", &threeACcode);

    Operand *s2Arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, s2Arg, NULL, NULL, &threeACcode);

    Operand *s1Arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, s1Arg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    Operand *labelTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelContinueStrcmp = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_JUMP, labelContinueStrcmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueStrcmp, NULL, NULL, &threeACcode);

    Operand *resultCmp = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultCmp, NULL, NULL, &threeACcode);

    Operand *labelEqual = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelLess = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelGreater = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelEndCmp = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelEqual, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelLess, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelGreater, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultCmp, create_operand_from_constant_int(1), NULL, &threeACcode);
    emit(OP_JUMP, labelEndCmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEqual, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultCmp, create_operand_from_constant_int(0), NULL, &threeACcode);
    emit(OP_JUMP, labelEndCmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelLess, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultCmp, create_operand_from_constant_int(-1), NULL, &threeACcode);
    emit(OP_JUMP, labelEndCmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEndCmp, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultCmp, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_ord(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.ord call", &threeACcode);

    Operand *iArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, iArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, iArg, NULL, NULL, &threeACcode);

    Operand *sArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, sArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, sArg, NULL, NULL, &threeACcode);

    Operand *typeI = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeI, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeI, iArg, NULL, &threeACcode);

    Operand *labelTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelContinueOrd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, sArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelContinueOrd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, iArg, iArg, NULL, &threeACcode);
    emit(OP_JUMP, labelContinueOrd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_JUMP, labelContinueOrd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelContinueOrd, NULL, NULL, &threeACcode);

    Operand *lenS = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, lenS, NULL, NULL, &threeACcode);
    emit(OP_STRLEN, lenS, sArg, NULL, &threeACcode);

    Operand *resultOrd = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultOrd, NULL, NULL, &threeACcode);

    Operand *labelReturnZero = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelEndOrd = create_operand_from_label(threeAC_create_label(&threeACcode));

    // (lenS == 0)
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnZero, NULL, NULL, &threeACcode);

    // Check if i < 0
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnZero, NULL, NULL, &threeACcode);

    // Check if i >= lenS
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnZero, NULL, NULL, &threeACcode);

    emit(OP_STRI2INT, resultOrd, sArg, iArg, &threeACcode);
    emit(OP_JUMP, labelEndOrd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelReturnZero, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultOrd, create_operand_from_constant_int(0), NULL, &threeACcode);

    emit(OP_LABEL, labelEndOrd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultOrd, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_floor(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.floor call", &threeACcode);

    Operand *labelIsInt = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelIsNotNum = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelEndFloor = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand *argVal = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, argVal, NULL, NULL, &threeACcode);
    emit(OP_POPS, argVal, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelIsInt, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelIsNotNum, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INTS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, labelEndFloor, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelIsInt, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);

    emit(OP_JUMP, labelEndFloor, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelIsNotNum, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelEndFloor, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_str(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.str call", &threeACcode);

    Operand *argVal = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, argVal, NULL, NULL, &threeACcode);
    emit(OP_POPS, argVal, NULL, NULL, &threeACcode);

    Operand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);

    Operand *typeCheckVar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeCheckVar, NULL, NULL, &threeACcode);

    Operand *labelEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand *labelIsString = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelIsInt = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelIsFloat = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelIsNil = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_TYPE, typeCheckVar, argVal, NULL, &threeACcode);

    emit(OP_JUMPIFEQ, labelIsNil, argVal, create_operand_from_constant_nil(), &threeACcode);
    emit(OP_JUMPIFEQ, labelIsString, typeCheckVar, create_operand_from_constant_string("string"),
         &threeACcode);
    emit(OP_JUMPIFEQ, labelIsInt, typeCheckVar, create_operand_from_constant_string("int"),
         &threeACcode);
    emit(OP_JUMPIFEQ, labelIsFloat, typeCheckVar, create_operand_from_constant_string("float"),
         &threeACcode);

    emit(OP_MOVE, resultStr, create_operand_from_constant_string(""), NULL,
         &threeACcode); // Empty string or error
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle string
    emit(OP_LABEL, labelIsString, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, argVal, NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle int
    emit(OP_LABEL, labelIsInt, NULL, NULL, &threeACcode);
    emit(OP_INT2STR, resultStr, argVal, NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle float
    emit(OP_LABEL, labelIsFloat, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2STR, resultStr, argVal, NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle nil
    emit(OP_LABEL, labelIsNil, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, create_operand_from_constant_string("null"), NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

tDataType generate_ifj_length(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.length call", &threeACcode);

    Operand *strArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, strArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, strArg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, strArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    Operand *labelContinueLength = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelContinueLength, NULL, NULL, &threeACcode);

    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueLength, NULL, NULL, &threeACcode);

    Operand *resultLen = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultLen, NULL, NULL, &threeACcode);

    emit(OP_STRLEN, resultLen, strArg, NULL, &threeACcode);
    emit(OP_PUSHS, resultLen, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_substring(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.substring call", &threeACcode);

    Operand *jArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, jArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, jArg, NULL, NULL, &threeACcode);

    Operand *iArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, iArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, iArg, NULL, NULL, &threeACcode);

    Operand *sArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, sArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, sArg, NULL, NULL, &threeACcode);

    Operand *typeI = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    Operand *typeJ = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeI, NULL, NULL, &threeACcode);
    emit(OP_DEFVAR, typeJ, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeI, iArg, NULL, &threeACcode);
    emit(OP_TYPE, typeJ, jArg, NULL, &threeACcode);

    emit(OP_PUSHS, sArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    Operand *labelParamTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelContinueSubstring = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelCheckJEnd = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelCheckIEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelParamTypeError, NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQ, labelCheckJEnd, typeJ, create_operand_from_constant_string("int"),
         &threeACcode);
    emit(OP_JUMPIFNEQ, labelParamTypeError, typeJ, create_operand_from_constant_string("float"),
         &threeACcode);
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelParamTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, jArg, jArg, NULL, &threeACcode);
    emit(OP_LABEL, labelCheckJEnd, NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQ, labelContinueSubstring, typeI, create_operand_from_constant_string("int"),
         &threeACcode);
    emit(OP_JUMPIFNEQ, labelParamTypeError, typeI, create_operand_from_constant_string("float"),
         &threeACcode);
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelParamTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, iArg, iArg, NULL, &threeACcode);
    emit(OP_LABEL, labelCheckIEnd, NULL, NULL, &threeACcode);

    emit(OP_JUMP, labelContinueSubstring, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelParamTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueSubstring, NULL, NULL, &threeACcode);

    // Get length of s
    Operand *lenS = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, lenS, NULL, NULL, &threeACcode);
    emit(OP_STRLEN, lenS, sArg, NULL, &threeACcode);

    // Labels for null return
    Operand *labelReturnNull = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelEndSubstring = create_operand_from_label(threeAC_create_label(&threeACcode));

    // Boundary checks
    // i < 0
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // j < 0
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // i > j
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // i >= Ifj.length(s)  => NOT (i < Ifj.length(s))
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // j > Ifj.length(s)
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    Operand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, create_operand_from_constant_string(""), NULL, &threeACcode);

    Operand *loopCounter = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, loopCounter, NULL, NULL, &threeACcode);
    emit(OP_MOVE, loopCounter, iArg, NULL, &threeACcode);

    Operand *currentChar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, currentChar, NULL, NULL, &threeACcode);

    Operand *loopStartLabelSub = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *loopEndLabelSub = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_LABEL, loopStartLabelSub, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, loopCounter, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode); // Result (bool) is on stack
    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, loopEndLabelSub, NULL, NULL, &threeACcode);

    emit(OP_GETCHAR, currentChar, sArg, loopCounter, &threeACcode);
    emit(OP_CONCAT, resultStr, resultStr, currentChar, &threeACcode);

    emit(OP_ADD, loopCounter, loopCounter, create_operand_from_constant_int(1), &threeACcode);
    emit(OP_JUMP, loopStartLabelSub, NULL, NULL, &threeACcode);
    emit(OP_LABEL, loopEndLabelSub, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);
    emit(OP_JUMP, labelEndSubstring, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelReturnNull, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_nil(), NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEndSubstring, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

tDataType generate_ifj_chr(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.chr call", &threeACcode);

    Operand *iArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, iArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, iArg, NULL, NULL, &threeACcode);

    Operand *typeI = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeI, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeI, iArg, NULL, &threeACcode);

    Operand *labelTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *labelContinueChr = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelContinueChr, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, iArg, iArg, NULL, &threeACcode);
    emit(OP_JUMP, labelContinueChr, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueChr, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_INT2CHARS, NULL, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

void generate_truthiness_check(ThreeACList *list, Operand *expr_val)
{
    Operand *finalBoolResult = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, finalBoolResult, NULL, NULL, list);

    Operand *labelIsNull = create_operand_from_label(threeAC_create_label(list));
    Operand *labelIsBool = create_operand_from_label(threeAC_create_label(list));
    Operand *labelIsOther = create_operand_from_label(threeAC_create_label(list));
    Operand *labelEndTruthiness = create_operand_from_label(threeAC_create_label(list));

    // Check if null
    emit(OP_JUMPIFEQ, labelIsNull, expr_val, create_operand_from_constant_nil(), list);

    // Check if boolean (using TYPE instruction)
    Operand *typeCheckVar = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, typeCheckVar, NULL, NULL, list);
    emit(OP_TYPE, typeCheckVar, expr_val, NULL, list);

    emit(OP_JUMPIFEQ, labelIsBool, typeCheckVar, create_operand_from_constant_string("bool"), list);

    // If not null and not bool, it's true
    emit(OP_JUMP, labelIsOther, NULL, NULL, list);

    // Case: is null
    emit(OP_LABEL, labelIsNull, NULL, NULL, list);
    emit(OP_MOVE, finalBoolResult, create_operand_from_constant_bool(false), NULL, list);
    emit(OP_JUMP, labelEndTruthiness, NULL, NULL, list);

    // Case: is boolean
    emit(OP_LABEL, labelIsBool, NULL, NULL, list);
    emit(OP_MOVE, finalBoolResult, expr_val, NULL, list);
    emit(OP_JUMP, labelEndTruthiness, NULL, NULL, list);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, labelIsOther, NULL, NULL, list);
    emit(OP_MOVE, finalBoolResult, create_operand_from_constant_bool(true), NULL, list);
    emit(OP_JUMP, labelEndTruthiness, NULL, NULL, list);

    emit(OP_LABEL, labelEndTruthiness, NULL, NULL, list);
    emit(OP_PUSHS, finalBoolResult, NULL, NULL, list); // Push the final boolean result
}

void generate_add_op(ThreeACList *list)
{
    Operand *op2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    Operand *op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

    Operand *type1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type1, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);

    Operand *type2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type2, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    Operand *endAddLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand *numAddLabelCheck = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *numAddLabelType = create_operand_from_label(threeAC_create_label(&threeACcode));

    Operand *typeErrorLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, numAddLabelCheck, NULL, NULL, &threeACcode);

    Operand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);
    emit(OP_CONCAT, resultStr, op1, op2, &threeACcode);
    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);
    emit(OP_JUMP, endAddLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numAddLabelCheck, NULL, NULL, &threeACcode);

    Operand *operand1CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand1CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);

    emit(OP_LABEL, operand1CheckEnd, NULL, NULL, &threeACcode);

    Operand *operand2CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand2CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);

    emit(OP_LABEL, operand2CheckEnd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numAddLabelType, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_ADDS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, endAddLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, endAddLabel, NULL, NULL, &threeACcode);
}

void generate_mult_op(ThreeACList *list)
{
    Operand *op2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    Operand *op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

    Operand *type1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type1, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);

    Operand *type2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type2, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    Operand *endMultLabel = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *numMultLabelCheck = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *numMultLabelType = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *typeErrorLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

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

    emit(OP_JUMPIFNEQS, numMultLabelCheck, NULL, NULL, &threeACcode);

    Operand *replaceOperand =
        create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    Operand *replaceEndLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_DEFVAR, replaceOperand, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQS, replaceEndLabel, NULL, NULL, &threeACcode);

    emit(OP_MOVE, replaceOperand, op1, NULL, &threeACcode);
    emit(OP_MOVE, op1, op2, NULL, &threeACcode);
    emit(OP_MOVE, op2, replaceOperand, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    emit(OP_LABEL, replaceEndLabel, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);

    Operand *op2IsIntLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_JUMPIFNEQS, op2IsIntLabel, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    emit(OP_LABEL, op2IsIntLabel, NULL, NULL, &threeACcode);

    Operand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, create_operand_from_constant_string(""), NULL, &threeACcode);

    Operand *loopStart = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *loopEnd = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand *condition = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, condition, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loopStart, NULL, NULL, &threeACcode);

    emit(OP_GT, condition, op2, create_operand_from_constant_int(0), &threeACcode);
    emit(OP_JUMPIFNEQ, loopEnd, condition, create_operand_from_constant_bool(true), &threeACcode);

    emit(OP_CONCAT, resultStr, resultStr, op1, &threeACcode);

    emit(OP_SUB, op2, op2, create_operand_from_constant_int(1), &threeACcode);
    emit(OP_JUMP, loopStart, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loopEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);

    emit(OP_JUMP, endMultLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numMultLabelCheck, NULL, NULL, &threeACcode);

    Operand *operand1CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand1CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);

    emit(OP_LABEL, operand1CheckEnd, NULL, NULL, &threeACcode);

    Operand *operand2CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand2CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);

    emit(OP_LABEL, operand2CheckEnd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numMultLabelType, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_MULS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, endMultLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, endMultLabel, NULL, NULL, &threeACcode);
}

void generate_numeric_op(ThreeACList *list, char *op)
{
    Operand *op2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op2, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);

    Operand *op1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op1, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);

    Operand *type1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type1, NULL, NULL, list);
    emit(OP_TYPE, type1, op1, NULL, list);

    Operand *type2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type2, NULL, NULL, list);
    emit(OP_TYPE, type2, op2, NULL, list);

    Operand *typeErrorLabel = create_operand_from_label(threeAC_create_label(list));
    Operand *afterNumTypeCheckLabel = create_operand_from_label(threeAC_create_label(list));

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
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, list);

    emit(OP_JUMP, afterNumTypeCheckLabel, NULL, NULL, list);

    emit(OP_LABEL, typeErrorLabel, NULL, NULL, list);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL,
         list);

    emit(OP_LABEL, afterNumTypeCheckLabel, NULL, NULL, list);

    Operand *intType = create_operand_from_constant_string("int");

    Operand *isInt1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, isInt1, NULL, NULL, list);
    emit(OP_EQ, isInt1, type1, intType, list);
    Operand *op1OkLabel = create_operand_from_label(threeAC_create_label(list));
    emit(OP_JUMPIFNEQ, op1OkLabel, isInt1, create_operand_from_constant_bool(true), list);
    emit(OP_INT2FLOAT, op1, op1, NULL, list);
    emit(OP_LABEL, op1OkLabel, NULL, NULL, list);

    Operand *isInt2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, isInt2, NULL, NULL, list);
    emit(OP_EQ, isInt2, type2, intType, list);
    Operand *op2OkLabel = create_operand_from_label(threeAC_create_label(list));
    emit(OP_JUMPIFNEQ, op2OkLabel, isInt2, create_operand_from_constant_bool(true), list);
    emit(OP_INT2FLOAT, op2, op2, NULL, list);
    emit(OP_LABEL, op2OkLabel, NULL, NULL, list);

    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_PUSHS, op2, NULL, NULL, list);

    if (strcmp(op, "-") == 0)
    {
        emit(OP_SUBS, NULL, NULL, NULL, list);
    }
    else if (strcmp(op, "/") == 0)
    {
        emit(OP_DIVS, NULL, NULL, NULL, list);
    }
    else if (strcmp(op, "*") == 0)
    {
        emit(OP_MULS, NULL, NULL, NULL, list);
    }
    else if (strcmp(op, "+") == 0)
    {
        emit(OP_ADDS, NULL, NULL, NULL, list);
    }
}

void generate_relational_op(ThreeACList *list, char *op)
{
    Operand *op2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op2, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);

    Operand *op1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op1, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);

    Operand *type1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type1, NULL, NULL, list);
    emit(OP_TYPE, type1, op1, NULL, list);

    Operand *type2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type2, NULL, NULL, list);
    emit(OP_TYPE, type2, op2, NULL, list);

    if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0)
    {
        Operand *typeErrorLabel = create_operand_from_label(threeAC_create_label(list));
        Operand *afterNumTypeCheckLabel = create_operand_from_label(threeAC_create_label(list));

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
        emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, list);

        emit(OP_JUMP, afterNumTypeCheckLabel, NULL, NULL, list);

        emit(OP_LABEL, typeErrorLabel, NULL, NULL, list);
        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL,
             NULL, list);

        emit(OP_LABEL, afterNumTypeCheckLabel, NULL, NULL, list);
    }

    Operand *op1ToFloatIfInt = create_operand_from_label(threeAC_create_label(list));
    emit(OP_PUSHS, type1, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, op1ToFloatIfInt, NULL, NULL, list);

    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_INT2FLOATS, NULL, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);
    emit(OP_MOVE, type1, create_operand_from_constant_string("float"), NULL, list);

    emit(OP_LABEL, op1ToFloatIfInt, NULL, NULL, list);

    Operand *op2ToFloatIfInt = create_operand_from_label(threeAC_create_label(list));
    emit(OP_PUSHS, type2, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, op2ToFloatIfInt, NULL, NULL, list);

    emit(OP_PUSHS, op2, NULL, NULL, list);
    emit(OP_INT2FLOATS, NULL, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);
    emit(OP_MOVE, type2, create_operand_from_constant_string("float"), NULL, list);

    emit(OP_LABEL, op2ToFloatIfInt, NULL, NULL, list);
    Operand *performOpLabelOnSameType = create_operand_from_label(threeAC_create_label(list));
    Operand *endRelOpLabel = create_operand_from_label(threeAC_create_label(list));

    emit(OP_JUMPIFEQ, performOpLabelOnSameType, type1, type2, list);
    if (strcmp(op, "!=") == 0)
    {
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, list);
    }
    else
    {
        emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, list);
    }

    emit(OP_JUMP, endRelOpLabel, NULL, NULL, list);

    emit(OP_LABEL, performOpLabelOnSameType, NULL, NULL, list);
    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_PUSHS, op2, NULL, NULL, list);

    OperationType opType;
    bool useNot = false;

    if (strcmp(op, "<") == 0)
        opType = OP_LTS;
    else if (strcmp(op, ">") == 0)
        opType = OP_GTS;
    else if (strcmp(op, "==") == 0)
        opType = OP_EQS;
    else if (strcmp(op, "!=") == 0)
    {
        opType = OP_EQS;
        useNot = true;
    }
    else if (strcmp(op, "<=") == 0)
    {
        opType = OP_GTS;
        useNot = true;
    }
    else if (strcmp(op, ">=") == 0)
    {
        opType = OP_LTS;
        useNot = true;
    }

    emit(opType, NULL, NULL, NULL, list);
    if (useNot)
    {
        emit(OP_NOTS, NULL, NULL, NULL, list);
    }
    emit(OP_LABEL, endRelOpLabel, NULL, NULL, list);
}
