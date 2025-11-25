#include "expr_parser.h"
#include "parser.h"
#include <stdio.h>
#include "3AC.h"
#include "3AC_patterns.h"
#include "semantic.h"

static char* process_string_literal(const char* raw_data) {
    int len = strlen(raw_data);

    if (strncmp(raw_data, "\"\"\"", 3) == 0) {
        // Multiline string
        if (len < 6) {
            char *s = safeMalloc(1);
            s[0] = '\0';
            return s;
        }

        const char* content_start = raw_data + 3;
        const char* content_end = raw_data + len - 3;

        // Trim leading whitespace and a single newline
        const char* p = content_start;
        bool only_whitespace_before_newline = true;
        while (p < content_end && *p != '\n') {
            if (!isspace((unsigned char)*p)) {
                only_whitespace_before_newline = false;
                break;
            }
            p++;
        }
        if (only_whitespace_before_newline && p < content_end && *p == '\n') {
            content_start = p + 1;
        }

        // Trim trailing newline and whitespace
        p = content_end - 1;
        bool only_whitespace_after_newline = true;
        while (p >= content_start && *p != '\n') {
            if (!isspace((unsigned char)*p)) {
                only_whitespace_after_newline = false;
                break;
            }
            p--;
        }
        if (only_whitespace_after_newline && p >= content_start && *p == '\n') {
            content_end = p;
        }
        
        if (content_start >= content_end) {
             char *empty_str = safeMalloc(1);
             empty_str[0] = '\0';
             return empty_str;
        }

        size_t new_len = content_end - content_start;
        char* trimmed_str = safeMalloc(new_len + 1);
        strncpy(trimmed_str, content_start, new_len);
        trimmed_str[new_len] = '\0';
        
        char* final_str = safeMalloc(new_len + 1);
        int j = 0;
        for (int i = 0; i < new_len; i++) {
            if (trimmed_str[i] == '\\') {
                i++;
                if (i >= new_len) { break; }
                switch (trimmed_str[i]) {
                    case 'n': final_str[j++] = '\n'; break;
                    case 'r': final_str[j++] = '\r'; break;
                    case 't': final_str[j++] = '\t'; break;
                    case '"': final_str[j++] = '"'; break;
                    case '\\': final_str[j++] = '\\'; break;
                    case 'x':
                        if (i + 2 < new_len && isxdigit(trimmed_str[i+1]) && isxdigit(trimmed_str[i+2])) {
                            char hex[3] = {trimmed_str[i+1], trimmed_str[i+2], '\0'};
                            final_str[j++] = (char)strtol(hex, NULL, 16);
                            i += 2;
                        } else {
                            final_str[j++] = 'x';
                        }
                        break;
                    default:
                        final_str[j++] = '\\';
                        final_str[j++] = trimmed_str[i];
                        break;
                }
            } else {
                final_str[j++] = trimmed_str[i];
            }
        }
        final_str[j] = '\0';
        free(trimmed_str);
        return safeRealloc(final_str, j + 1);

    } else { // Regular string
        if (len < 2) {
            char *s = safeMalloc(1);
            s[0] = '\0';
            return s;
        }
        
        char* unescaped_str = safeMalloc(len);
        int j = 0;
        for (int i = 1; i < len - 1; i++) {
            if (raw_data[i] == '\\') {
                i++;
                if (i >= len - 1) { break; }
                switch (raw_data[i]) {
                    case 'n': unescaped_str[j++] = '\n'; break;
                    case 'r': unescaped_str[j++] = '\r'; break;
                    case 't': unescaped_str[j++] = '\t'; break;
                    case '"': unescaped_str[j++] = '"'; break;
                    case '\\': unescaped_str[j++] = '\\'; break;
                    case 'x':
                        if (i + 2 < len - 1 && isxdigit(raw_data[i+1]) && isxdigit(raw_data[i+2])) {
                            char hex[3] = {raw_data[i+1], raw_data[i+2], '\0'};
                            unescaped_str[j++] = (char)strtol(hex, NULL, 16);
                            i += 2;
                        } else {
                            unescaped_str[j++] = 'x';
                        }
                        break;
                    default:
                        unescaped_str[j++] = '\\';
                        unescaped_str[j++] = raw_data[i];
                        break;
                }
            } else {
                unescaped_str[j++] = raw_data[i];
            }
        }
        unescaped_str[j] = '\0';
        return safeRealloc(unescaped_str, j + 1);
    }
}

// Helper to create an Operand from a token
static Operand* create_operand_from_token(tToken token, tSymTableStack *sym_stack) {
    if (!token) return NULL;

    Operand *op;
    char *data_copy = NULL;

    if (token->data) {
        data_copy = safeMalloc(strlen(token->data) + 1);
        strcpy(data_copy, token->data);
    } else if (token->type == T_KW_NULL_VALUE) {
        op = create_operand_from_constant_nil();
        return op;
    } else {
        return NULL;
    }

    switch(token->type) {
        case T_INTEGER:
            op = create_operand_from_constant_int(atoi(data_copy));
            break;
        case T_FLOAT:
            op = create_operand_from_constant_float(atof(data_copy));
            break;
        case T_STRING:
        {
            op = create_operand_from_constant_string(process_string_literal(data_copy));
            break;
        }
        case T_KW_NULL_VALUE:
            op = create_operand_from_constant_nil();
            break;
        case T_ID:
        {
            int getterKeyLen = strlen("getter:") + strlen(data_copy) + 3;
            char* getterKey = safeMalloc(getterKeyLen);
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

                return op;
            }

            tSymbolData *data = find_data_in_stack(sym_stack, data_copy);

            // Treat as getter not yet defined
            if (!data) {
                data = safeMalloc(sizeof(tSymbolData));
                data->kind = SYM_FUNC;
                data->dataType = TYPE_UNDEF;
                data->returnType = TYPE_UNDEF;
                data->index = -1;
                data->defined = false;
                data->paramCount = 0;
                data->paramNames = NULL;
                data->unique_name = NULL;

                if (!symtable_insert(global_symtable, getterKey, *data))
                {
                    fprintf(stderr, "[PARSER] Error inserting forward decl for '%s'\n", getterKey);
                    exit(INTERNAL_ERROR);
                }

                emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);
                emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);
                
                char mangledName[256];
                sprintf(mangledName, "%s$0%%getter", data_copy);
                Operand* call_label = create_operand_from_label(mangledName);
                emit(OP_CALL, call_label, NULL, NULL, &threeACcode);
                
                emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);
                op = create_operand_from_tf_variable("%retval");
                free(getterKey);
                free(data);
                break;
            }

            op = create_operand_from_variable(data->unique_name, false);

            break;
        }
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(sym_stack, data_copy);
            if (!data) {
                semantic_define_variable(sym_stack, data_copy, true);
                data = find_data_in_stack(sym_stack, data_copy);
            }

            op = create_operand_from_variable(data->unique_name, true);
            break;
        }
        default:
            return NULL;
    }
    free(data_copy);
    return op;
}

static tDataType get_data_type_from_token(tToken token, tSymTableStack *sym_stack) {
    if (!token) return TYPE_UNDEF;

    switch(token->type) {
        case T_INTEGER:
        case T_FLOAT:
            return TYPE_NUM;
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

static tPrec precedence_table[11][11] = {
/*                MUL_DIV  PLUS_MINUS  REL     EQ_NEQ   IS     TYPE    LPAREN  RPAREN     ID   LITERAL  DOLLAR */
/* MUL_DIV */    { '>',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',    '<',    '>'  },
/* PLUS_MINUS */ { '<',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',    '<',    '>'  },
/* REL */        { '<',     '<',        '>',    '>',     '>',   'E',     '<',    '>',    '<',    '<',    '>'  },
/* EQ_NEQ */     { '<',     '<',        '<',    '>',     '>',   'E',     '<',    '>',    '<',    '<',    '>'  },
/* IS */         { '<',     '<',        '<',    '<',     'E',   '<',     '<',    '>',    '<',    '<',    '>'  },
/* TYPE */       { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',    'E',    '>'  },
/* LPAREN */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    '=',    '<',    '<',    'E'  },
/* RPAREN */     { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',    'E',    '>'  },
/* ID */         { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',    'E',    '>'  },
/* LITERAL */    { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',    'E',    '>'  },
/* DOLLAR */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    'E',    '<',    '<',    'E'  },
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

        case T_INTEGER:
        case T_FLOAT:
        case T_STRING:
        case T_KW_NULL_VALUE:
            return E_LITERAL;
        case T_ID:
        case T_GLOBAL_ID:
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
    if (n1->is_terminal && (n1->symbol == E_ID || n1->symbol == E_LITERAL))
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
            expr_push(stack, n2->symbol, false);
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
            tDataType result_type = TYPE_UNDEF;

            //Static semantic analysis of literal types
            if (n1->symbol == E_LITERAL && n3->symbol == E_LITERAL)
            {
                result_type = semantic_check_literal_operation(n2->value, n1->dataType, n3->dataType);
            }

            tSymbol op = n2->symbol;
            if (op == E_MUL_DIV || op == E_PLUS_MINUS || op == E_REL || op == E_EQ_NEQ)
            {

                if (strcmp(n2->value, "+") == 0) {
                    generate_add_op(&threeACcode);
                }
                else if (strcmp(n2->value, "*") == 0) 
                {
                    generate_mult_op(&threeACcode);
                } 
                else if (strcmp(n2->value, "-") == 0 || strcmp(n2->value, "/") == 0) 
                {
                    generate_numeric_op(&threeACcode, n2->value);
                    result_type = TYPE_NUM;
                }
                else {
                    generate_relational_op(&threeACcode, n2->value);
                }
             
                expr_pop(stack); 
                if (n2->value) free(n2->value);
                expr_pop(stack);  
                expr_pop(stack);  

                if (n1->symbol == E_ID || n3->symbol == E_ID) 
                {
                  expr_push(stack, E_ID, false);
                }
                else 
                {
                  expr_push(stack, E_LITERAL, false);
                }

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

            if (look_sym == E_ID || look_sym == E_LITERAL)
            {
                Operand *op = create_operand_from_token(lookahead, stack);
                emit(OP_PUSHS, op, NULL, NULL, &threeACcode);
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
