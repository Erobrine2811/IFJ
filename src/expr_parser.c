#include "expr_parser.h"
#include "parser.h"
#include <stdio.h>
#include "3AC.h"
#include "semantic.h"

// Helper to create an Operand from a token
static Operand* create_operand_from_token(tToken token, tSymTableStack *sym_stack) {
    if (!token) return NULL;

    Operand *op = safeMalloc(sizeof(Operand));
    char *data_copy = NULL;
    if (token->data) {
        data_copy = safeMalloc(strlen(token->data) + 1);
        strcpy(data_copy, token->data);
    } else if (token->type == T_KW_NULL_VALUE) {
        op->type = OPP_CONST_NIL;
        return op;
    } else {
        free(op);
        return NULL;
    }

    switch(token->type) {
        case T_INTEGER:
            op->type = OPP_CONST_FLOAT;
            op->value.floatval = (double)atoi(data_copy);
            free(data_copy);
            break;
        case T_FLOAT:
            op->type = OPP_CONST_FLOAT;
            op->value.floatval = atof(data_copy);
            free(data_copy);
            break;
        case T_STRING:
        {
            op->type = OPP_CONST_STRING;
            int len = strlen(data_copy);
            if (len < 2 || data_copy[0] != '"' || data_copy[len-1] != '"') {
                op->value.strval = data_copy;
                break;
            }

            char* unescaped_str = safeMalloc(len);
            int j = 0;
            for (int i = 1; i < len - 1; i++) {
                if (data_copy[i] == '\\') {
                    i++;
                    if (i >= len - 1) { break; }
                    switch (data_copy[i]) {
                        case 'n': unescaped_str[j++] = '\n'; break;
                        case 'r': unescaped_str[j++] = '\r'; break;
                        case 't': unescaped_str[j++] = '\t'; break;
                        case '"': unescaped_str[j++] = '"'; break;
                        case '\\': unescaped_str[j++] = '\\'; break;
                        case 'x':
                            if (i + 2 < len - 1 && isxdigit(data_copy[i+1]) && isxdigit(data_copy[i+2])) {
                                char hex[3] = {data_copy[i+1], data_copy[i+2], '\0'};
                                unescaped_str[j++] = (char)strtol(hex, NULL, 16);
                                i += 2;
                            } else {
                                unescaped_str[j++] = 'x';
                            }
                            break;
                        default:
                            unescaped_str[j++] = '\\';
                            unescaped_str[j++] = data_copy[i];
                            break;
                    }
                } else {
                    unescaped_str[j++] = data_copy[i];
                }
            }
            unescaped_str[j] = '\0';
            op->value.strval = safeRealloc(unescaped_str, j + 1);
            free(data_copy);
            break;
        }
        case T_KW_NULL_VALUE:
            op->type = OPP_CONST_NIL;
            free(data_copy);
            break;
        case T_ID:
        {
            char getterKey[256];
            sprintf(getterKey, "getter:%s@0", data_copy);
            tSymbolData *getter_data = symtable_find(global_symtable, getterKey);

            if (getter_data) {
                emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);
                emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);
                
                char mangledName[256];
                sprintf(mangledName, "%s$0%%getter", data_copy);
                Operand* call_label = create_operand_from_label(mangledName);
                emit(OP_CALL, call_label, NULL, NULL, &threeACcode);
                
                emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);
                
                op = create_operand_from_tf_variable("%retval");
                free(data_copy);
                return op;
            }

            tSymbolData *data = find_data_in_stack(sym_stack, data_copy);
            if (!data) {
                fprintf(stderr, "[SEMANTIC] Error: variable '%s' not found\n", data_copy);
                exit(OTHER_SEMANTIC_ERROR);
            }
            op->type = OPP_VAR;
            op->value.varname = safeMalloc(strlen(data->unique_name) + 1);
            strcpy(op->value.varname, data->unique_name);
            free(data_copy);
            break;
        }
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(sym_stack, data_copy);
            if (!data) {
                semantic_define_variable(sym_stack, data_copy, true);
                data = find_data_in_stack(sym_stack, data_copy);
            }
            op->type = OPP_GLOBAL;
            op->value.varname = safeMalloc(strlen(data->unique_name) + 1);
            strcpy(op->value.varname, data->unique_name);
            free(data_copy);
            break;
        }
        default:
            free(op);
            free(data_copy);
            return NULL;
    }
    return op;
}

static tDataType get_data_type_from_token(tToken token, tSymTableStack *sym_stack) {
    if (!token) return TYPE_UNDEF;

    switch(token->type) {
        case T_INTEGER:
            return TYPE_INT;
        case T_FLOAT:
            return TYPE_FLOAT;
        case T_STRING:
            return TYPE_STRING;
        case T_KW_NULL_VALUE:
            return TYPE_NULL;
        case T_ID:
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(sym_stack, token->data);
            if (data) {
                return data->dataType;
            }
            return TYPE_UNDEF;
        }
        default:
            return TYPE_UNDEF;
    }
}

static tPrec precedence_table[10][10] = {
/*                MUL_DIV  PLUS_MINUS  REL     EQ_NEQ   IS     TYPE    LPAREN  RPAREN  ID     DOLLAR */
/* MUL_DIV */    { '>',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* PLUS_MINUS */ { '<',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* REL */        { '<',     '<',        '>',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* EQ_NEQ */     { '<',     '<',        '<',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* IS */         { '<',     '<',        '<',    '<',     'E',   '<',     '<',    '>',    '<',   '>'  },
/* TYPE */       { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',   '>'  },
/* LPAREN */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    '=',    '<',   'E'  },
/* RPAREN */     { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',   '>'  },
/* ID */         { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',   '>'  },
/* DOLLAR */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    'E',    '<',   'E'  },
};

static tSymbol get_precedence_type(tType type)
{
    switch (type)
    {
        case T_MUL:
        case T_DIV:
            return E_MUL_DIV;

        case T_ADD:
        case T_SUB:
            return E_PLUS_MINUS;

        case T_LT:
        case T_GT:
        case T_LTE:
        case T_GTE:
            return E_REL;

        case T_EQL:
        case T_NEQ:
            return E_EQ_NEQ;

        case T_KW_IS:
            return E_IS;

        case T_KW_NUM:
        case T_KW_STRING:
        case T_KW_NULL_TYPE:
            return E_TYPE;

        case T_LEFT_PAREN:
            return E_LPAREN;

        case T_RIGHT_PAREN:
            return E_RPAREN;

        case T_ID:
        case T_GLOBAL_ID:
        case T_INTEGER:
        case T_FLOAT:
        case T_STRING:
        case T_KW_NULL_VALUE:
            return E_ID;

        case T_EOF:
            return E_DOLLAR;

        default:
            return E_DOLLAR; // fallback
    }
}

static void expr_push(tExprStack *stack, tSymbol sym, bool is_terminal)
{
    tExprStackNode *node = (tExprStackNode *)safeMalloc(sizeof(tExprStackNode));
    node->symbol = sym;
    node->is_terminal = is_terminal;
    node->next = stack->top;
    node->dataType = TYPE_UNDEF;
    node->value = NULL;
    stack->top = node;
}

static void expr_pop(tExprStack *stack)
{
    if (stack->top == NULL) return;
    tExprStackNode *tmp = stack->top;
    stack->top = tmp->next;
    free(tmp);
}

static tExprStackNode *expr_top_terminal(tExprStack *stack)
{
    tExprStackNode *curr = stack->top;
    while (curr != NULL)
    {
        if (curr->is_terminal) return curr;
        curr = curr->next;
    }
    return NULL;
}

static void expr_pop_until_marker(tExprStack *stack)
{
    while (stack->top != NULL && !(stack->top->is_terminal && stack->top->symbol == E_DOLLAR))
    {
        expr_pop(stack);
    }
}

static int is_token_expr_end(tToken *token)
{
    // Expression end tokens: ), ,, EOL, }, EOF
    if (token == NULL) return 1;
    switch ((*token)->type)
    {
        case T_RIGHT_PAREN:
        case T_COMMA:
        case T_EOL:
        case T_RIGHT_BRACE:
        case T_EOF:
            return 1;
        default:
            return 0;
    }
}

static int reduce_expr(tExprStack *stack)
{
    tExprStackNode *n1 = stack->top;
    if (n1 == NULL) return 0;

    // E -> i
    if (n1->is_terminal && n1->symbol == E_ID)
    {
        tDataType type = n1->dataType;
        n1->is_terminal = false;
        if (n1->value) {
            free(n1->value);
            n1->value = NULL;
        }
        n1->dataType = type;
        return 1;
    }

    tExprStackNode *n2 = n1->next;
    tExprStackNode *n3 = n2 ? n2->next : NULL;

    // E -> (E)
    if (n1 && n2 && n3)
    {
        if (n1->is_terminal && n1->symbol == E_RPAREN &&
           !n2->is_terminal &&
           n3->is_terminal && n3->symbol == E_LPAREN)
        {
            tDataType type = n2->dataType;
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);
            expr_push(stack, E_ID, false);
            stack->top->dataType = type;
            return 1;
        }
    }

    // E -> E is TYPE
    if (n1 && n2 && n3) {
        if (n1->is_terminal && n1->symbol == E_TYPE &&
            n2->is_terminal && n2->symbol == E_IS &&
            !n3->is_terminal)
        {
            // Pop E, is, TYPE
        char* type_str = n1->value;
        expr_pop(stack);
        expr_pop(stack);
        expr_pop(stack);

        Operand* expr_val = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, expr_val, NULL, NULL, &threeACcode);
        emit(OP_POPS, expr_val, NULL, NULL, &threeACcode);

        Operand* type_val = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, type_val, NULL, NULL, &threeACcode);
        emit(OP_TYPE, type_val, expr_val, NULL, &threeACcode);

        Operand* result = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
        emit(OP_DEFVAR, result, NULL, NULL, &threeACcode);

        if (strcmp(type_str, "Num") == 0) {
            Operand* is_int_label = create_operand_from_label(threeAC_create_label(&threeACcode));
            Operand* end_label = create_operand_from_label(threeAC_create_label(&threeACcode));
            
            emit(OP_JUMPIFEQ, is_int_label, type_val, create_operand_from_constant_string("int"), &threeACcode);
            
            emit(OP_EQ, result, type_val, create_operand_from_constant_string("float"), &threeACcode);
            emit(OP_JUMP, end_label, NULL, NULL, &threeACcode);

            emit(OP_LABEL, is_int_label, NULL, NULL, &threeACcode);
            emit(OP_MOVE, result, create_operand_from_constant_bool(true), NULL, &threeACcode);

            emit(OP_LABEL, end_label, NULL, NULL, &threeACcode);
        } else if (strcmp(type_str, "Null") == 0) {
            emit(OP_EQ, result, type_val, create_operand_from_constant_string("nil"), &threeACcode);
        } else if (strcmp(type_str, "String") == 0) {
            emit(OP_EQ, result, type_val, create_operand_from_constant_string("string"), &threeACcode);
        } else {
            emit(OP_MOVE, result, create_operand_from_constant_bool(false), NULL, &threeACcode);
        }
        
        emit(OP_PUSHS, result, NULL, NULL, &threeACcode);

        if (type_str) free(type_str);

        expr_push(stack, E_ID, false);
        stack->top->dataType = TYPE_UNDEF;
        return 1;
    }
}

    // E -> E op E
    if (n1 && n2 && n3)
    {
        if (!n1->is_terminal &&
           n2->is_terminal &&
           !n3->is_terminal)
        {
            tSymbol op = n2->symbol;
            if (op == E_MUL_DIV || op == E_PLUS_MINUS || op == E_REL || op == E_EQ_NEQ)
            {
                tDataType result_type = TYPE_UNDEF;

                    if (strcmp(n2->value, "+") == 0) {
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

                        Operand* string_type = create_operand_from_constant_string("string");
                        Operand* is_string1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                        emit(OP_DEFVAR, is_string1, NULL, NULL, &threeACcode);
                        emit(OP_EQ, is_string1, type1, string_type, &threeACcode);

                        Operand* is_string2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                        emit(OP_DEFVAR, is_string2, NULL, NULL, &threeACcode);
                        emit(OP_EQ, is_string2, type2, string_type, &threeACcode);

                        Operand* both_strings = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                        emit(OP_DEFVAR, both_strings, NULL, NULL, &threeACcode);
                        emit(OP_AND, both_strings, is_string1, is_string2, &threeACcode);

                        Operand* numeric_add_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                        emit(OP_JUMPIFNEQ, numeric_add_label, both_strings, create_operand_from_constant_bool(true), &threeACcode);

                        // String concatenation
                        Operand* concat_res = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                        emit(OP_DEFVAR, concat_res, NULL, NULL, &threeACcode);
                        emit(OP_CONCAT, concat_res, op1, op2, &threeACcode);
                        emit(OP_PUSHS, concat_res, NULL, NULL, &threeACcode);
                        result_type = TYPE_STRING;
                        Operand* end_add_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                        emit(OP_JUMP, end_add_label, NULL, NULL, &threeACcode);

                        // Numeric addition
                        emit(OP_LABEL, numeric_add_label, NULL, NULL, &threeACcode);
                        // Convert op1 if int
                        Operand* int_type = create_operand_from_constant_string("int");
                        Operand* is_int1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                        emit(OP_DEFVAR, is_int1, NULL, NULL, &threeACcode);
                        emit(OP_EQ, is_int1, type1, int_type, &threeACcode);
                        Operand* op1_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                        emit(OP_JUMPIFNEQ, op1_ok_label, is_int1, create_operand_from_constant_bool(true), &threeACcode);
                        emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);
                        emit(OP_LABEL, op1_ok_label, NULL, NULL, &threeACcode);

                        // Convert op2 if int
                        Operand* is_int2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                        emit(OP_DEFVAR, is_int2, NULL, NULL, &threeACcode);
                        emit(OP_EQ, is_int2, type2, int_type, &threeACcode);
                        Operand* op2_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                        emit(OP_JUMPIFNEQ, op2_ok_label, is_int2, create_operand_from_constant_bool(true), &threeACcode);
                        emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);
                        emit(OP_LABEL, op2_ok_label, NULL, NULL, &threeACcode);
                        
                        emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
                        emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
                        emit(OP_ADDS, NULL, NULL, NULL, &threeACcode);
                        result_type = TYPE_FLOAT;

                        emit(OP_LABEL, end_add_label, NULL, NULL, &threeACcode);
                  }
                  else if (strcmp(n2->value, "*") == 0) {
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

                    Operand* string_type = create_operand_from_constant_string("string");
                    Operand* int_type = create_operand_from_constant_string("int");
                    Operand* float_type = create_operand_from_constant_string("float");

                    Operand* is_string1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_string1, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_string1, type1, string_type, &threeACcode);

                    Operand* is_int2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_int2, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_int2, type2, int_type, &threeACcode);

                    Operand* is_float2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_float2, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_float2, type2, float_type, &threeACcode);

                    Operand* is_numeric2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_numeric2, NULL, NULL, &threeACcode);
                    emit(OP_OR, is_numeric2, is_int2, is_float2, &threeACcode);

                    Operand* is_repetition = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_repetition, NULL, NULL, &threeACcode);
                    emit(OP_AND, is_repetition, is_string1, is_numeric2, &threeACcode);

                    Operand* numeric_mul_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, numeric_mul_label, is_repetition, create_operand_from_constant_bool(true), &threeACcode);

                    // String repetition path
                    Operand* str_op = op1;
                    Operand* num_op = op2;

                    // Convert numeric operand to int if it's a float
                    Operand* num_op_type = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, num_op_type, NULL, NULL, &threeACcode);
                    emit(OP_TYPE, num_op_type, num_op, NULL, &threeACcode);
                    Operand* is_float_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFEQ, is_float_label, num_op_type, float_type, &threeACcode);
                    Operand* conversion_done_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMP, conversion_done_label, NULL, NULL, &threeACcode);
                    emit(OP_LABEL, is_float_label, NULL, NULL, &threeACcode);
                    emit(OP_FLOAT2INT, num_op, num_op, NULL, &threeACcode);
                    emit(OP_LABEL, conversion_done_label, NULL, NULL, &threeACcode);

                    Operand* result_str = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, result_str, NULL, NULL, &threeACcode);
                    emit(OP_MOVE, result_str, create_operand_from_constant_string(""), NULL, &threeACcode);
                    
                    Operand* loop_start = create_operand_from_label(threeAC_create_label(&threeACcode));
                    Operand* loop_end = create_operand_from_label(threeAC_create_label(&threeACcode));
                    Operand* condition = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, condition, NULL, NULL, &threeACcode);

                    emit(OP_LABEL, loop_start, NULL, NULL, &threeACcode);
                    
                    emit(OP_GT, condition, num_op, create_operand_from_constant_int(0), &threeACcode);
                    emit(OP_JUMPIFNEQ, loop_end, condition, create_operand_from_constant_bool(true), &threeACcode);
                    
                    emit(OP_CONCAT, result_str, result_str, str_op, &threeACcode);
                    
                    emit(OP_SUB, num_op, num_op, create_operand_from_constant_int(1), &threeACcode);
                    emit(OP_JUMP, loop_start, NULL, NULL, &threeACcode);
                    
                    emit(OP_LABEL, loop_end, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, result_str, NULL, NULL, &threeACcode);
                    result_type = TYPE_STRING;
                    Operand* end_mul_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMP, end_mul_label, NULL, NULL, &threeACcode);

                    // Numeric multiplication path
                    emit(OP_LABEL, numeric_mul_label, NULL, NULL, &threeACcode);
                    
                    Operand* is_int1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_int1, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_int1, type1, int_type, &threeACcode);

                    // Convert op1 if int
                    Operand* op1_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, op1_ok_label, is_int1, create_operand_from_constant_bool(true), &threeACcode);
                    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);
                    emit(OP_LABEL, op1_ok_label, NULL, NULL, &threeACcode);

                    // Convert op2 if int
                    Operand* op2_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, op2_ok_label, is_int2, create_operand_from_constant_bool(true), &threeACcode);
                    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);
                    emit(OP_LABEL, op2_ok_label, NULL, NULL, &threeACcode);

                    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
                    emit(OP_MULS, NULL, NULL, NULL, &threeACcode);
                    result_type = TYPE_FLOAT;

                    emit(OP_LABEL, end_mul_label, NULL, NULL, &threeACcode);
                } else if (strcmp(n2->value, "-") == 0 || strcmp(n2->value, "/") == 0) {
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

                    Operand* int_type = create_operand_from_constant_string("int");

                    // Convert op1 if int
                    Operand* is_int1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_int1, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_int1, type1, int_type, &threeACcode);
                    Operand* op1_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, op1_ok_label, is_int1, create_operand_from_constant_bool(true), &threeACcode);
                    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);
                    emit(OP_LABEL, op1_ok_label, NULL, NULL, &threeACcode);

                    // Convert op2 if int
                    Operand* is_int2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_int2, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_int2, type2, int_type, &threeACcode);
                    Operand* op2_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, op2_ok_label, is_int2, create_operand_from_constant_bool(true), &threeACcode);
                    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);
                    emit(OP_LABEL, op2_ok_label, NULL, NULL, &threeACcode);

                    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);

                    if (strcmp(n2->value, "-") == 0) {
                        emit(OP_SUBS, NULL, NULL, NULL, &threeACcode);
                    } else { // Division
                        emit(OP_DIVS, NULL, NULL, NULL, &threeACcode);
                    }
                    result_type = TYPE_FLOAT;
                }
                else { // Relational operators
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

                    Operand* int_type = create_operand_from_constant_string("int");
                    Operand* float_type = create_operand_from_constant_string("float");

                    Operand* is_int1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_int1, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_int1, type1, int_type, &threeACcode);

                    Operand* is_float1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_float1, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_float1, type1, float_type, &threeACcode);

                    Operand* is_numeric1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_numeric1, NULL, NULL, &threeACcode);
                    emit(OP_OR, is_numeric1, is_int1, is_float1, &threeACcode);

                    Operand* is_int2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_int2, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_int2, type2, int_type, &threeACcode);

                    Operand* is_float2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_float2, NULL, NULL, &threeACcode);
                    emit(OP_EQ, is_float2, type2, float_type, &threeACcode);

                    Operand* is_numeric2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, is_numeric2, NULL, NULL, &threeACcode);
                    emit(OP_OR, is_numeric2, is_int2, is_float2, &threeACcode);

                    Operand* both_numeric = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                    emit(OP_DEFVAR, both_numeric, NULL, NULL, &threeACcode);
                    emit(OP_AND, both_numeric, is_numeric1, is_numeric2, &threeACcode);

                    Operand* non_numeric_comp_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, non_numeric_comp_label, both_numeric, create_operand_from_constant_bool(true), &threeACcode);

                    // Numeric comparison path
                    Operand* op1_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, op1_ok_label, is_int1, create_operand_from_constant_bool(true), &threeACcode);
                    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);
                    emit(OP_LABEL, op1_ok_label, NULL, NULL, &threeACcode);

                    Operand* op2_ok_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMPIFNEQ, op2_ok_label, is_int2, create_operand_from_constant_bool(true), &threeACcode);
                    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);
                    emit(OP_LABEL, op2_ok_label, NULL, NULL, &threeACcode);
                    
                    Operand* end_comp_label = create_operand_from_label(threeAC_create_label(&threeACcode));
                    emit(OP_JUMP, end_comp_label, NULL, NULL, &threeACcode);

                    // Non-numeric comparison path
                    emit(OP_LABEL, non_numeric_comp_label, NULL, NULL, &threeACcode);

                    emit(OP_LABEL, end_comp_label, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);

                    OperationType opType;
                    bool use_not = false;

                    if (strcmp(n2->value, "<") == 0) opType = OP_LTS;
                    else if (strcmp(n2->value, ">") == 0) opType = OP_GTS;
                    else if (strcmp(n2->value, "==") == 0) opType = OP_EQS;
                    else if (strcmp(n2->value, "!=") == 0) { opType = OP_EQS; use_not = true; }
                    else if (strcmp(n2->value, "<=") == 0) { opType = OP_GTS; use_not = true; }
                    else if (strcmp(n2->value, ">=") == 0) { opType = OP_LTS; use_not = true; }
                    else {
                        return 0; // Should not happen
                    }
                
                    emit(opType, NULL, NULL, NULL, &threeACcode);
                    if (use_not) {
                        emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
                    }
                    result_type = TYPE_UNDEF;
                }
             
                expr_pop(stack); 
                if (n2->value) free(n2->value);
                expr_pop(stack);  
                expr_pop(stack);  
                expr_push(stack, E_ID, false);
                stack->top->dataType = result_type;
                return 1;
            }
        }
    }

    return 0;
}

tDataType parse_expression(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    tExprStack expr_stack = { NULL };
    expr_push(&expr_stack, E_DOLLAR, true);

    tToken lookahead = *currentToken;
    int done = 0;

    while (!done)
    {
        tExprStackNode *top_terminal = expr_top_terminal(&expr_stack);
        if (top_terminal == NULL)
        {
            exit(SYNTAX_ERROR);
        }

        tSymbol stack_sym = top_terminal->symbol;
        tSymbol look_sym = get_precedence_type(lookahead->type);

        tPrec prec = precedence_table[stack_sym][look_sym];


        if (prec == PREC_LESS || prec == PREC_EQUAL) {
            expr_push(&expr_stack, look_sym, true);

            if (look_sym == E_ID) 
            {
                Operand *op = create_operand_from_token(lookahead, stack);
                if (op) {
                    emit(OP_PUSHS, op, NULL, NULL, &threeACcode);
                }
                expr_stack.top->dataType = get_data_type_from_token(lookahead, stack);
            }
            else if (look_sym == E_TYPE)
            {
                char *type_keyword = NULL;
                if (lookahead->type == T_KW_NUM) {
                    type_keyword = "Num";
                } else if (lookahead->type == T_KW_STRING) {
                    type_keyword = "String";
                } else if (lookahead->type == T_KW_NULL_TYPE) {
                    type_keyword = "Null";
                }
                
                if (type_keyword) {
                    expr_stack.top->value = safeMalloc(strlen(type_keyword) + 1);
                    strcpy(expr_stack.top->value, type_keyword);
                } else {
                    expr_stack.top->value = NULL;
                }
            }
            else if (look_sym == E_PLUS_MINUS || look_sym == E_MUL_DIV || look_sym == E_REL || look_sym == E_EQ_NEQ)
            {
                if (lookahead->type == T_ADD) { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "+");
                }
                else if (lookahead->type == T_SUB) 
                { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "-");
                }
                else if (lookahead->type == T_MUL) 
                { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "*");
                }
                else if (lookahead->type == T_DIV) 
                {
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "/");
                } else if (lookahead->type == T_EQL) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, "==");
                } else if (lookahead->type == T_NEQ) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, "!=");
                } else if (lookahead->type == T_LT) { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "<");
                } else if (lookahead->type == T_LTE) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, "<=");
                } else if (lookahead->type == T_GT) { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, ">");
                } else if (lookahead->type == T_GTE) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, ">=");
                }
            }
            
            if (lookahead->type == T_EOF)
            {
                break;
            }

            get_next_token(file, &lookahead);
        }
        else if (prec == PREC_GREATER)
        {
            if (!reduce_expr(&expr_stack))
            {
                printf("Reduction failed\n");
                exit(SYNTAX_ERROR);
            }
        }
        else
        {
            exit(SYNTAX_ERROR);
        }

        tExprStackNode *n1 = expr_stack.top;
        tExprStackNode *n2 = n1 ? n1->next : NULL;
        if (n1 && n2)
        {
            if (!n1->is_terminal && n2->is_terminal && n2->symbol == E_DOLLAR)
            {
                if (is_token_expr_end(&lookahead))
                {
                    done = 1;
                }
            }
        }
    }

    tDataType result_type = TYPE_UNDEF;
    if (expr_stack.top && !expr_stack.top->is_terminal) {
       result_type = expr_stack.top->dataType;
       if (threeACcode.return_used == true) {
          Operand* retval_var = create_operand_from_variable("%retval", false);
          emit(OP_POPS, retval_var, NULL, NULL, &threeACcode);
          emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
          threeACcode.return_used = false;
       }
       threeACcode.expression_result = NULL;
    }

    while (expr_stack.top != NULL)
    {
        if (expr_stack.top->value) {
            free(expr_stack.top->value);
        }
        expr_pop(&expr_stack);
    }

    *currentToken = lookahead;
    return result_type;
}
