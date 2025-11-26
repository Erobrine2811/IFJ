#include "parser.h"
#include "scanner.h"
#include "symtable.h"
#include <stdio.h>
#include "semantic.h"
#include "3AC.h"
#include "3AC_patterns.h"
static tToken peek_buffer = NULL;

static tBuiltinDef builtin_defs[] = {
    {"Ifj.write", TYPE_NULL, 1, {TYPE_UNDEF}},
    {"Ifj.read_num", TYPE_NUM, 0, {}},
    {"Ifj.read_str", TYPE_STRING, 0, {}},
    {"Ifj.floor", TYPE_NUM, 1, {TYPE_NUM}},
    {"Ifj.str", TYPE_STRING, 1, {TYPE_UNDEF}},
    {"Ifj.length", TYPE_NUM, 1, {TYPE_STRING}},
    {"Ifj.substring", TYPE_STRING, 3, {TYPE_STRING, TYPE_NUM, TYPE_NUM}},
    {"Ifj.strcmp", TYPE_NUM, 2, {TYPE_STRING, TYPE_STRING}},
    {"Ifj.ord", TYPE_NUM, 2, {TYPE_STRING, TYPE_NUM}},
    {"Ifj.chr", TYPE_STRING, 1, {TYPE_NUM}},
};

tSymTable *global_symtable = NULL;

tToken peek_token(FILE *file)
{
    if (peek_buffer == NULL)
    {
        if (getToken(file, &peek_buffer) != 0)
        {
            exit(LEXICAL_ERROR);
        }
    }

    return peek_buffer;
}

void get_next_token(FILE *file, tToken *currentToken)
{
    if (*currentToken != NULL)
    {
        freeToken(currentToken);
    }

    if (peek_buffer != NULL)
    {
        *currentToken = peek_buffer;
        peek_buffer = NULL;
        return;
    }

    if (getToken(file, currentToken) != 0)
    {
        exit(LEXICAL_ERROR);
    }
}

void expect_and_consume(tType type, tToken *currentToken, FILE *file, bool check_value, const char* value)
{
    if ((*currentToken)->type != type)
    {
        printf("Unexpected token: %d\n", (*currentToken)->type);
        printf("Unexpected token: %d:%d\n", (*currentToken)->linePos, (*currentToken)->colPos);
        printf("Expected token: %d\n", type);
        exit(SYNTAX_ERROR);
    }

    if (check_value)
    {
        if (strcmp((*currentToken)->data, value) != 0)
        {
            exit(SYNTAX_ERROR);
        }
    }

    get_next_token(file, currentToken);
}

void skip_optional_eol(tToken *currentToken, FILE *file)
{
    while ((*currentToken)->type == T_EOL)
        get_next_token(file, currentToken);
}

void consume_eol(FILE *file, tToken *currentToken)
{
    if ((*currentToken)->type != T_EOL)
    {
        fprintf(stderr, "[PARSER] Syntax Error: Expected EOL, but got token of type %d\n", (*currentToken)->type);
        exit(SYNTAX_ERROR);
    }

    do {
        get_next_token(file, currentToken);
    } while ((*currentToken)->type == T_EOL);
}

tSymbolData *find_data_in_stack(tSymTableStack *stack, const char *key)
{
    for (int i = stack->top; i >= 0; i--)
    {
        tSymTable *table = stack->tables[i];
        tSymbolData *data = symtable_find(table, key);
        if (data != NULL)
        {
            return data;
        }
    }
    return NULL;
}

void parser_dispose_stack(tSymTableStack *stack)
{
    while (!symtable_stack_is_empty(stack))
    {
        tSymTable *table = symtable_stack_top(stack);
        symtable_stack_pop(stack);
        symtable_free(table);
        free(table);
    }
}

int parse_program(FILE *file)
{
    tToken currentToken = NULL;
    tSymTableStack stack;
    symtable_stack_init(&stack);

    global_symtable = safeMalloc(sizeof(tSymTable));
    symtable_init(global_symtable);
    symtable_stack_push(&stack, global_symtable);

    insert_builtin_functions();

    get_next_token(file, &currentToken);

    parse_prolog(file, &currentToken);
    parse_class_def(file, &currentToken, &stack);
    skip_optional_eol(&currentToken, file);
    expect_and_consume(T_EOF, &currentToken, file, false, NULL);

    check_undefined_functions();

    parser_dispose_stack(&stack);
    freeToken(&currentToken);

    return 0;
}

void parse_prolog(FILE *file, tToken *currentToken)
{
    skip_optional_eol(currentToken, file);
    expect_and_consume(T_KW_IMPORT, currentToken, file, false, NULL);
    expect_and_consume(T_STRING, currentToken, file, true, "\"ifj25\"");
    expect_and_consume(T_KW_FOR, currentToken, file, false, NULL);
    expect_and_consume(T_KW_IFJ, currentToken, file, false, NULL);
    consume_eol(file, currentToken);
}

void parse_class_def(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    expect_and_consume(T_KW_CLASS, currentToken, file, false, NULL);
    expect_and_consume(T_ID, currentToken, file, true, "Program");
    expect_and_consume(T_LEFT_BRACE, currentToken, file, false, NULL);
    consume_eol(file, currentToken);

    generate_program_entrypoint(&threeACcode);

    parse_func_list(file, currentToken, stack);

    if (symtable_find(global_symtable, "main@0") == NULL)
    {
        fprintf(stderr, "[SEMANTIC] Error: undefined function 'main' with 0 parameters\n");
        exit(UNDEFINED_FUN_ERROR);
    }

    expect_and_consume(T_RIGHT_BRACE, currentToken, file, false, NULL);
}

void insert_builtin_functions()
{
    for (int i = 0; i < 10; i++)
    {
        tBuiltinDef *def = &builtin_defs[i];

        tSymbolData builtinData = {0};
        builtinData.kind = SYM_FUNC;
        builtinData.dataType = TYPE_UNDEF;
        builtinData.index = -1;
        builtinData.defined = true;
        builtinData.returnType = def->returnType;
        builtinData.paramCount = def->paramCount;
        builtinData.paramNames = NULL;
        builtinData.unique_name = NULL;

        if (def->paramCount > 0)
        {
            builtinData.paramTypes = safeMalloc(sizeof(tDataType) * def->paramCount);
            for (int j = 0; j < def->paramCount; j++)
            {
                builtinData.paramTypes[j] = def->params[j];
            }
        }
        else
        {
            builtinData.paramTypes = NULL;
        }

        if (!symtable_insert(global_symtable, def->name, builtinData))
        {
            fprintf(stderr, "[INTERNAL] Could not insert builtin '%s'.\n", def->name);
        }
    }
}

void parse_func_list(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    if ((*currentToken)->type != T_KW_STATIC)
    {
        exit(SYNTAX_ERROR);
    }

    while ((*currentToken)->type == T_KW_STATIC)
    {
        parse_function_declaration(file, currentToken, stack);
        consume_eol(file, currentToken);
    }
}

void parse_function_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    expect_and_consume(T_KW_STATIC, currentToken, file, false, NULL);

    if ((*currentToken)->type != T_ID)
    {
        printf("Expected token type %d but got2 %d\n", T_ID, (*currentToken)->type);
        exit(SYNTAX_ERROR);
    }

    char *funcName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(funcName, (*currentToken)->data);

    get_next_token(file, currentToken);

    if ((*currentToken)->type != T_LEFT_PAREN)
    {
        if ((*currentToken)->type == T_LEFT_BRACE)
        {
            parse_getter(file, currentToken, stack, funcName);
            free(funcName);
            return;
        }
        else if ((*currentToken)->type == T_ASSIGN)
        {
            parse_setter(file, currentToken, stack, funcName);
            free(funcName);
            return;
        }

        free(funcName);
        exit(SYNTAX_ERROR);
    }


    get_next_token(file, currentToken);
    skip_optional_eol(currentToken, file);

    tSymTable *funcSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(funcSymtable);
    symtable_stack_push(stack, funcSymtable);

    char **paramNames = NULL;
    int paramCount = parse_parameter_list(file, currentToken, stack, &paramNames);

    int mangled_len = strlen(funcName) + 1 + 10 + strlen("%func") + 1;
    char *mangledName = safeMalloc(mangled_len);
    sprintf(mangledName, "%s$%d%%func", funcName, paramCount);

    Operand *labelOp = safeMalloc(sizeof(Operand));
    labelOp->type = OPP_LABEL;
    labelOp->value.label = mangledName;
    char* commentText = safeMalloc(strlen(funcName) + 25);
    sprintf(commentText, "####################");
    emit_comment(commentText, &threeACcode);
    sprintf(commentText, "Function declaration: %s", funcName);
    emit_comment(commentText, &threeACcode);
    sprintf(commentText, "####################");
    emit_comment(commentText, &threeACcode);
    free(commentText);
    emit(OP_LABEL, labelOp, NULL, NULL , &threeACcode);

    Operand* retval_def = create_operand_from_variable("%retval", false);
    emit(OP_DEFVAR, retval_def, NULL, NULL, &threeACcode);
    Operand* retval_init = create_operand_from_variable("%retval", false);
    Operand* nil_op = create_operand_from_constant_nil();
    emit(OP_MOVE, retval_init, nil_op, NULL, &threeACcode);

    tSymTable *func_scope_table = symtable_stack_top(stack);

    for (int i = 0; i < paramCount; i++) {
        tSymbolData *param_data = symtable_find(func_scope_table, paramNames[i]);
        if (!param_data) {
            exit(INTERNAL_ERROR);
        }
        Operand* paramOp = create_operand_from_variable(param_data->unique_name, false);
        emit(OP_DEFVAR, paramOp, NULL, NULL, &threeACcode);
    }

    for (int i = 0; i < paramCount; i++) {
        char temp_param_name[20];
        sprintf(temp_param_name, "%%param%d", i);

        tSymbolData *param_data = symtable_find(func_scope_table, paramNames[i]);
        if (!param_data) {
            exit(INTERNAL_ERROR);
        }

        Operand* dest = create_operand_from_variable(param_data->unique_name, false);
        Operand* src = create_operand_from_variable(temp_param_name, false);
        emit(OP_MOVE, dest, src, NULL, &threeACcode);
    }

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    threeACcode.temp_counter = 0;

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    int keyLength = strlen(funcName) + 1 + 10 + 1;
    char *key = safeMalloc(keyLength);
    sprintf(key, "%s@%d", funcName, paramCount);
    free(funcName);

    tSymbolData *existing = symtable_find(global_symtable, key);
    if (existing != NULL)
    {
        if (existing->defined)
        {
            fprintf(stderr, "[SEMANTIC] Error: function '%s' redefined\n", key);
            free(key);
            exit(REDEFINITION_FUN_ERROR);
        }
        existing->paramCount = paramCount;
        existing->paramNames = paramNames;
        if (existing->paramTypes) free(existing->paramTypes);
        if (paramCount > 0) {
            existing->paramTypes = safeMalloc(sizeof(tDataType) * paramCount);
            for (int i = 0; i < paramCount; i++) existing->paramTypes[i] = TYPE_UNDEF;
        } else {
            existing->paramTypes = NULL;
        }
    }
    else
    {
        tSymbolData funcData = {0};
        funcData.kind = SYM_FUNC;
        funcData.dataType = TYPE_UNDEF;
        funcData.index = -1;
        funcData.defined = false;
        funcData.paramCount = paramCount;
        funcData.paramNames = paramNames;
        funcData.unique_name = NULL;

        if (paramCount > 0)
        {
            funcData.paramTypes = safeMalloc(sizeof(tDataType) * paramCount);
            for (int i = 0; i < paramCount; i++)
                funcData.paramTypes[i] = TYPE_UNDEF;
        }
        else
        {
            funcData.paramTypes = NULL;
        }

        if (!symtable_insert(global_symtable, key, funcData))
        {
            free(key);
            exit(INTERNAL_ERROR);
        }
    }

    parse_block(file, currentToken, stack, true);

    tSymbolData *justDefined = symtable_find(global_symtable, key);
    if (justDefined != NULL)
    {
        justDefined->defined = true;
    }

    tSymTable *poppedSymtable = symtable_stack_top(stack);
    symtable_stack_pop(stack);
    symtable_free(poppedSymtable);
    free(poppedSymtable);
    free(key);
    // For space bettween instructions
    emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
}

void parse_getter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName)
{
    int keyLength = strlen("getter:") + strlen(funcName) + 3;
    char *key = safeMalloc(keyLength);
    sprintf(key, "getter:%s@0", funcName);

    tSymbolData *alreadyDefined = symtable_find(global_symtable, key);

    if (alreadyDefined != NULL && alreadyDefined->defined)
    {
        free(key);
        printf("Getter redefinition error for %s\n", funcName);
        exit(REDEFINITION_FUN_ERROR);
    }

    if (alreadyDefined == NULL)
    {
        tSymbolData getterData = {0};
        getterData.kind = SYM_FUNC;
        getterData.dataType = TYPE_UNDEF;
        getterData.index = -1;
        getterData.defined = false;
        getterData.paramCount = 0;
        getterData.paramNames = NULL;
        getterData.unique_name = NULL;

        if (!symtable_insert(global_symtable, key, getterData))
        {
            free(key);
            printf("Getter redefinition error for %s\n", funcName);
            exit(REDEFINITION_FUN_ERROR);
        }
    }

    tSymTable *getterSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(getterSymtable);
    symtable_stack_push(stack, getterSymtable);

    int mangled_len = strlen(funcName) + strlen("$0%getter") + 1;
    char *mangledName = safeMalloc(mangled_len);
    sprintf(mangledName, "%s$0%%getter", funcName);

    Operand *labelOp = create_operand_from_label(mangledName);

    emit_comment("####################", &threeACcode);
    char* commentText = safeMalloc(strlen(funcName) + 25);
    sprintf(commentText, "Function declaration: %s (getter)", funcName);
    emit_comment(commentText, &threeACcode);
    emit_comment("####################", &threeACcode);
    emit(OP_LABEL, labelOp, NULL, NULL, &threeACcode);

    Operand* retval_def = create_operand_from_variable("%retval", false);
    emit(OP_DEFVAR, retval_def, NULL, NULL, &threeACcode);
    Operand* retval_init = create_operand_from_variable("%retval", false);
    Operand* nil_op = create_operand_from_constant_nil();
    emit(OP_MOVE, retval_init, nil_op, NULL, &threeACcode);

    parse_block(file, currentToken, stack, true);
    tSymbolData *definedGetter = symtable_find(global_symtable, key);

    if (definedGetter)
    {
        definedGetter->defined = true;
    }

    symtable_stack_pop(stack);
    symtable_free(getterSymtable);
    free(getterSymtable);
    free(key);
}

void parse_setter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName)
{
    get_next_token(file, currentToken);
    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);

    if ((*currentToken)->type != T_ID)
    {
        exit(SYNTAX_ERROR);
    }

    char *paramName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(paramName, (*currentToken)->data);

    get_next_token(file, currentToken);
    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    int keyLength = strlen("setter:") + strlen(funcName) + 3;
    char *key = safeMalloc(keyLength);
    sprintf(key, "setter:%s@1", funcName);

    tSymbolData *alreadyDefined = symtable_find(global_symtable, key);

    if (alreadyDefined != NULL && alreadyDefined->defined)
    {
        free(key);
        free(paramName);
        printf("Setter redefinition error for %s\n", funcName);
        exit(REDEFINITION_FUN_ERROR);
    }

    if (alreadyDefined == NULL)
    {
        tSymbolData setterData = {0};
        setterData.kind = SYM_FUNC;
        setterData.dataType = TYPE_UNDEF;
        setterData.index = -1;
        setterData.defined = false;
        setterData.paramCount = 1;
        setterData.paramTypes = safeMalloc(sizeof(tDataType));
        setterData.paramTypes[0] = TYPE_UNDEF;
        setterData.paramNames = NULL;
        setterData.unique_name = NULL;

        if (!symtable_insert(global_symtable, key, setterData))
        {
            free(key);
            free(setterData.paramTypes);
            exit(REDEFINITION_FUN_ERROR);
        }
    }

    tSymTable *setterSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(setterSymtable);
    symtable_stack_push(stack, setterSymtable);

    tSymbolData paramData = {0};
    paramData.kind = SYM_VAR;
    paramData.dataType = TYPE_UNDEF;
    paramData.index = -1;

    int len = snprintf(NULL, 0, "%s%%%d", paramName, threeACcode.var_counter);
    paramData.unique_name = safeMalloc(len + 1);
    sprintf(paramData.unique_name, "%s%%%d", paramName, threeACcode.var_counter++);

    symtable_insert(setterSymtable, paramName, paramData);
    free(paramName);

    int mangled_len = strlen(funcName) + strlen("$1%setter") + 1;
    char *mangledName = safeMalloc(mangled_len);
    sprintf(mangledName, "%s$1%%setter", funcName);

    Operand *labelOp = create_operand_from_label(mangledName);
    emit(OP_LABEL, labelOp, NULL, NULL, &threeACcode);

    Operand* retval_def = create_operand_from_variable("%retval", false);
    emit(OP_DEFVAR, retval_def, NULL, NULL, &threeACcode);
    Operand* retval_init = create_operand_from_variable("%retval", false);
    Operand* nil_op = create_operand_from_constant_nil();
    emit(OP_MOVE, retval_init, nil_op, NULL, &threeACcode);

    Operand* setter_param_dest = create_operand_from_variable(paramData.unique_name, false);
    Operand* setter_param_src = create_operand_from_variable("%param0", false);
    emit(OP_DEFVAR, setter_param_dest, NULL, NULL, &threeACcode);
    emit(OP_MOVE, setter_param_dest, setter_param_src, NULL, &threeACcode);

    parse_block(file, currentToken, stack, true);

    tSymbolData *definedSetter = symtable_find(global_symtable, key);
    if (definedSetter)
    {
        definedSetter->defined = true;
    }

    symtable_stack_pop(stack);
    symtable_free(setterSymtable);
    free(setterSymtable);
    free(key);
    emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
}

int parse_parameter_list(FILE *file, tToken *currentToken, tSymTableStack *stack, char ***paramNames)
{
    int paramCount = 0;
    tSymTable *currentSymtable = symtable_stack_top(stack);

    if ((*currentToken)->type == T_RIGHT_PAREN)
    {
        *paramNames = NULL;
        return 0;
    }

    *paramNames = NULL;

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Parameter declaration", &threeACcode);
    while (true)
    {
        if ((*currentToken)->type != T_ID)
        {
            exit(SYNTAX_ERROR);
        }

        char *paramName = safeMalloc(strlen((*currentToken)->data) + 1);
        strcpy(paramName, (*currentToken)->data);

        paramCount++;
        *paramNames = safeRealloc(*paramNames, paramCount * sizeof(char*));
        (*paramNames)[paramCount - 1] = safeMalloc(strlen(paramName) + 1);
        strcpy((*paramNames)[paramCount - 1], paramName);

        tSymbolData paramData = {0};
        paramData.kind = SYM_VAR;
        paramData.dataType = TYPE_UNDEF;
        paramData.index = -1;

        int len = snprintf(NULL, 0, "%s%%%d", paramName, threeACcode.var_counter);
        paramData.unique_name = safeMalloc(len + 1);
        sprintf(paramData.unique_name, "%s%%%d", paramName, threeACcode.var_counter++);

        if (!symtable_insert(currentSymtable, paramName, paramData))
        {
            free(paramData.unique_name);
            free(paramName);
            exit(REDEFINITION_FUN_ERROR);
        }
        free(paramName);

        get_next_token(file, currentToken);

        if ((*currentToken)->type == T_RIGHT_PAREN)
        {
            break;
        }

        expect_and_consume(T_COMMA, currentToken, file, false, NULL);
        skip_optional_eol(currentToken, file);
    }

    return paramCount;
}

void check_node_defined(tSymNode *node)
{
    if (node == NULL)
    {
        return;
    }

    check_node_defined(node->left);

    if (node->data.kind == SYM_FUNC && !node->data.defined)
    {

            fprintf(stderr, "[Parser] Error: undefined function: %s\n", node->key);
            exit(UNDEFINED_FUN_ERROR);
    }

    check_node_defined(node->right);
}

void check_undefined_functions()
{
    if (global_symtable != NULL)
    {
       check_node_defined(global_symtable->root);
    }
}

void parse_block(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isFunctionBody)
{
    tSymTable *blockSymtable;

    if (!isFunctionBody)
    {
        blockSymtable = safeMalloc(sizeof(tSymTable));
        symtable_init(blockSymtable);
        symtable_stack_push(stack, blockSymtable);
    }
    else {
        blockSymtable = symtable_stack_top(stack);
    }

    expect_and_consume(T_LEFT_BRACE, currentToken, file, false, NULL);
    consume_eol(file, currentToken);

    while ((*currentToken)->type != T_RIGHT_BRACE && (*currentToken)->type != T_EOF)
    {
        parse_statement(file, currentToken, stack);
        consume_eol(file, currentToken);
    }

    expect_and_consume(T_RIGHT_BRACE, currentToken, file, false, NULL);

    if (!isFunctionBody)
    {
        symtable_stack_pop(stack);
        symtable_free(blockSymtable);
        free(blockSymtable);
    }
}

void parse_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    switch ((*currentToken)->type)
    {
        case T_LEFT_BRACE:
            parse_block(file, currentToken, stack, false);
            break;
        case T_KW_IF:
            parse_if_statement(file, currentToken, stack);
            break;
        case T_KW_WHILE:
            parse_while_statement(file, currentToken, stack);
            break;
        case T_KW_RETURN:
            generate_return(file, currentToken, stack);
            break;
        case T_KW_VAR:
            parse_variable_declaration(file, currentToken, stack);
            break;
        case T_ID:
        case T_GLOBAL_ID:
            parse_assignment_statement(file, currentToken, stack);
            break;
        case T_KW_IFJ:
            parse_ifj_call(file, currentToken, stack, true);
            break;
        default:
            fprintf(stderr, "[PARSER] Syntax error: unexpected token '%s'\n", (*currentToken)->data);
            exit(SYNTAX_ERROR);
            break;
    }
}


void parse_if_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken); // consume 'if'

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    bool while_used_backup = threeACcode.while_used;
    threeACcode.while_used = false;
    threeACcode.if_used = true;

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("If statement condition", &threeACcode);
    parse_expression(file, currentToken, stack);

    // Handle truthiness rules
    Operand* expr_val_if = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, expr_val_if, NULL, NULL, &threeACcode);
    emit(OP_POPS, expr_val_if, NULL, NULL, &threeACcode); // Pop expression result

    Operand* final_bool_result_if = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, final_bool_result_if, NULL, NULL, &threeACcode);

    Operand* label_is_null_if = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_is_bool_if = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_is_other_if = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_end_truthiness_if = create_operand_from_label(threeAC_create_label(&threeACcode));

    // Check if null
    emit(OP_JUMPIFEQ, label_is_null_if, expr_val_if, create_operand_from_constant_nil(), &threeACcode);

    // Check if boolean (using TYPE instruction)
    Operand* type_check_var_if = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type_check_var_if, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type_check_var_if, expr_val_if, NULL, &threeACcode); // Get type of expr_val

    emit(OP_JUMPIFEQ, label_is_bool_if, type_check_var_if, create_operand_from_constant_string("bool"), &threeACcode);

    // If not null and not bool, it's true
    emit(OP_JUMP, label_is_other_if, NULL, NULL, &threeACcode);

    // Case: is null
    emit(OP_LABEL, label_is_null_if, NULL, NULL, &threeACcode);
    emit(OP_MOVE, final_bool_result_if, create_operand_from_constant_bool(false), NULL, &threeACcode);
    emit(OP_JUMP, label_end_truthiness_if, NULL, NULL, &threeACcode);

    // Case: is boolean
    emit(OP_LABEL, label_is_bool_if, NULL, NULL, &threeACcode);
    emit(OP_MOVE, final_bool_result_if, expr_val_if, NULL, &threeACcode);
    emit(OP_JUMP, label_end_truthiness_if, NULL, NULL, &threeACcode);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, label_is_other_if, NULL, NULL, &threeACcode);
    emit(OP_MOVE, final_bool_result_if, create_operand_from_constant_bool(true), NULL, &threeACcode);
    emit(OP_JUMP, label_end_truthiness_if, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_end_truthiness_if, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, final_bool_result_if, NULL, NULL, &threeACcode); // Push the final boolean result

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    char *label1_str = threeAC_create_label(&threeACcode);
    Operand *label1 = create_operand_from_label(label1_str);

    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, label1, NULL, NULL, &threeACcode);

    emit_comment("If-block", &threeACcode);
    parse_block(file, currentToken, stack, false);

    expect_and_consume(T_KW_ELSE, currentToken, file, false, NULL);

    char *label2_str = threeAC_create_label(&threeACcode);
    Operand *label2 = create_operand_from_label(label2_str);

    emit(OP_JUMP, label2, NULL, NULL, &threeACcode);
    emit(OP_LABEL, label1, NULL, NULL, &threeACcode);

    emit_comment("Else-block", &threeACcode);
    parse_block(file, currentToken, stack, false);

    emit(OP_LABEL, label2, NULL, NULL, &threeACcode);

    free(label2_str);

    free(label1_str);

    emit_comment("If statement end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);

    threeACcode.if_used = false;
    threeACcode.while_used = while_used_backup;
}


void parse_for_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);
}



void parse_assignment_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    tToken nextToken = peek_token(file);

    if (nextToken && nextToken->type == T_LEFT_PAREN && (*currentToken)->type == T_ID)
    {
        parse_function_call(file, currentToken, stack, true);
        if ((*currentToken)->type == T_EOL) {
            consume_eol(file, currentToken);
        }
        return;
    }

    bool isGlobal = ((*currentToken)->type == T_GLOBAL_ID);

    char *varName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(varName, (*currentToken)->data);
    get_next_token(file, currentToken);

    int keyLength = strlen("setter:") + strlen(varName) + 3;
    char *setterKey = safeMalloc(keyLength);
    sprintf(setterKey, "setter:%s@1", varName);

    tSymbolData *setterSymbol = symtable_find(global_symtable, setterKey);
    tSymbolData *var_data = NULL;
    bool is_new_declaration = false;

    if (isGlobal)
    {
        var_data = symtable_find(global_symtable, varName);
        if (var_data == NULL)
        {
            is_new_declaration = true;
            semantic_define_variable(stack, varName, true);
            var_data = symtable_find(global_symtable, varName);
        }
    }
    else
    {
        var_data = find_data_in_stack(stack, varName);

        if (var_data == NULL || setterSymbol != NULL)
        {

            if (var_data == NULL && setterSymbol == NULL)
            {
                var_data = safeMalloc(sizeof(tSymbolData));
                var_data->kind = SYM_FUNC;;
                var_data->dataType = TYPE_UNDEF;
                var_data->index = -1;
                var_data->unique_name = NULL;
                var_data->defined = false;
                var_data->paramCount = 1;
                var_data->paramNames = NULL;
                var_data->paramTypes = safeMalloc(sizeof(tDataType));
                var_data->paramTypes[0] = TYPE_UNDEF;

                if (!symtable_insert(global_symtable, setterKey, *var_data))
                {
                    free(var_data);
                    printf("Variable redefinition error for %s\n", varName);
                    exit(REDEFINITION_FUN_ERROR);
                }
            }

            expect_and_consume(T_ASSIGN, currentToken, file, false, NULL);
            skip_optional_eol(currentToken, file);

            parse_expression(file, currentToken, stack);

            emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);

            Operand* tf_param = create_operand_from_tf_variable("%param0");
            emit(OP_DEFVAR, tf_param, NULL, NULL, &threeACcode);

            Operand* tf_param_pop = create_operand_from_tf_variable("%param0");
            emit(OP_POPS, tf_param_pop, NULL, NULL, &threeACcode);

            emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);

            char mangledName[256];
            sprintf(mangledName, "%s$1%%setter", varName);
            Operand* call_label = create_operand_from_label(mangledName);
            emit(OP_CALL, call_label, NULL, NULL, &threeACcode);

            emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);

            free(varName);
            free(setterKey);
            return;
        }
    }

    if (!is_new_declaration) {
        char *commentText = safeMalloc(strlen(varName) + 30);
        sprintf(commentText, "Assignment to variable '%s'", varName);
        emit_comment(commentText, &threeACcode);
        free(commentText);
    }

    free(setterKey);


    expect_and_consume(T_ASSIGN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    nextToken = peek_token(file);
    tDataType expr_type;

    expr_type = parse_expression(file, currentToken, stack);

    if (var_data) {
        var_data->dataType = expr_type;
    }

    Operand *popsVarOp = create_operand_from_variable(var_data->unique_name, isGlobal);
    emit(OP_POPS, popsVarOp, NULL, NULL, &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    free(varName);
}

void parse_variable_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    if ((*currentToken)->type != T_ID && (*currentToken)->type != T_GLOBAL_ID)
    {
        exit(SYNTAX_ERROR);
    }

    bool isGlobal = ((*currentToken)->type == T_GLOBAL_ID);

    char *variable_name = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(variable_name, (*currentToken)->data);

    semantic_define_variable(stack, variable_name, isGlobal);

    tSymTable *target_table = isGlobal ? global_symtable : symtable_stack_top(stack);
    tSymbolData *var_sym_data = symtable_find(target_table, variable_name);

    Operand *varOp = safeMalloc(sizeof(Operand));
    varOp->type = isGlobal ? OPP_GLOBAL : OPP_VAR;
    varOp->value.varname = safeMalloc(strlen(var_sym_data->unique_name) + 1);
    strcpy(varOp->value.varname, var_sym_data->unique_name);

    char *commentText = safeMalloc(strlen(variable_name) + 30);
    sprintf(commentText, "Declaration of variable '%s'", variable_name);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment(commentText, &threeACcode);
    free(commentText);

    emit(OP_DEFVAR, varOp, NULL, NULL, &threeACcode);
    get_next_token(file, currentToken);

    if ((*currentToken)->type == T_ASSIGN)
    {
        get_next_token(file, currentToken);
        tDataType expr_type;

        expr_type = parse_expression(file, currentToken, stack);

        tSymbolData* var_data = isGlobal ? symtable_find(global_symtable, variable_name) : find_data_in_stack(stack, variable_name);
        if (var_data) {
            var_data->dataType = expr_type;
        }
        emit(OP_POPS, varOp, NULL, NULL, &threeACcode);
    }
}

void parse_while_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    bool if_used_backup = threeACcode.if_used;
    threeACcode.if_used = false;
    threeACcode.while_used = true;

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("While loop start", &threeACcode);
    InstructionNode *hoist_point = threeACcode.active;

    char *loop_start_label_str = threeAC_create_label(&threeACcode);
    char *loop_end_label_str = threeAC_create_label(&threeACcode);

    Operand *loop_start_label = safeMalloc(sizeof(Operand));
    loop_start_label->type = OPP_LABEL;
    loop_start_label->value.label = loop_start_label_str;

    Operand *loop_end_label = safeMalloc(sizeof(Operand));
    loop_end_label->type = OPP_LABEL;
    loop_end_label->value.label = loop_end_label_str;

    emit(OP_LABEL, loop_start_label, NULL, NULL, &threeACcode);
    emit_comment("While condition", &threeACcode);

    parse_expression(file, currentToken, stack);

    // Handle truthiness rules
    Operand* expr_val_while = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, expr_val_while, NULL, NULL, &threeACcode);
    emit(OP_POPS, expr_val_while, NULL, NULL, &threeACcode); // Pop expression result

    Operand* final_bool_result_while = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, final_bool_result_while, NULL, NULL, &threeACcode);

    Operand* label_is_null_while = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_is_bool_while = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_is_other_while = create_operand_from_label(threeAC_create_label(&threeACcode));
    Operand* label_end_truthiness_while = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_JUMPIFEQ, label_is_null_while, expr_val_while, create_operand_from_constant_nil(), &threeACcode);

    Operand* type_check_var_while = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type_check_var_while, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type_check_var_while, expr_val_while, NULL, &threeACcode); // Get type of expr_val

    emit(OP_JUMPIFEQ, label_is_bool_while, type_check_var_while, create_operand_from_constant_string("bool"), &threeACcode);

    emit(OP_JUMP, label_is_other_while, NULL, NULL, &threeACcode);

    // Case: is null
    emit(OP_LABEL, label_is_null_while, NULL, NULL, &threeACcode);
    emit(OP_MOVE, final_bool_result_while, create_operand_from_constant_bool(false), NULL, &threeACcode);
    emit(OP_JUMP, label_end_truthiness_while, NULL, NULL, &threeACcode);

    // Case: is boolean
    emit(OP_LABEL, label_is_bool_while, NULL, NULL, &threeACcode);
    emit(OP_MOVE, final_bool_result_while, expr_val_while, NULL, &threeACcode);
    emit(OP_JUMP, label_end_truthiness_while, NULL, NULL, &threeACcode);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, label_is_other_while, NULL, NULL, &threeACcode);
    emit(OP_MOVE, final_bool_result_while, create_operand_from_constant_bool(true), NULL, &threeACcode);
    emit(OP_JUMP, label_end_truthiness_while, NULL, NULL, &threeACcode);

    emit(OP_LABEL, label_end_truthiness_while, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, final_bool_result_while, NULL, NULL, &threeACcode); // Push the final boolean result

    Operand* condition_result = safeMalloc(sizeof(Operand));
    condition_result->type = OPP_TEMP;
    condition_result->value.varname = threeAC_create_temp(&threeACcode);
    emit(OP_DEFVAR, condition_result, NULL, NULL, &threeACcode);
    emit(OP_POPS, condition_result, NULL, NULL, &threeACcode);

    Operand* const_false = safeMalloc(sizeof(Operand));
    const_false->type = OPP_CONST_BOOL;
    const_false->value.boolval = false;

    emit(OP_JUMPIFEQ, loop_end_label, condition_result, const_false, &threeACcode);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    emit_comment("While body", &threeACcode);
    parse_block(file, currentToken, stack, false);

    emit(OP_JUMP, loop_start_label, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loop_end_label, NULL, NULL, &threeACcode);
    emit_comment("While loop end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    InstructionNode *loop_end_node = threeACcode.active;

    // Hoist DEFVARs
    InstructionNode *scan_ptr = hoist_point ? hoist_point->next : threeACcode.head;
    while (scan_ptr != NULL && scan_ptr != loop_end_node) {
        InstructionNode *next_scan = scan_ptr->next;
        if (scan_ptr->opType == OP_DEFVAR && (scan_ptr->result->type == OPP_VAR || scan_ptr->result->type == OPP_TEMP)) {
            // Unlink from current position
            scan_ptr->prev->next = scan_ptr->next;
            if (scan_ptr->next) {
                scan_ptr->next->prev = scan_ptr->prev;
            } else {
                threeACcode.tail = scan_ptr->prev;
            }

            // Insert after hoist_point
            if (hoist_point) {
                scan_ptr->next = hoist_point->next;
                if (hoist_point->next) {
                    hoist_point->next->prev = scan_ptr;
                }
                hoist_point->next = scan_ptr;
                scan_ptr->prev = hoist_point;
            } else { // Hoisting to the very beginning of the list
                scan_ptr->next = threeACcode.head;
                if (threeACcode.head) {
                    threeACcode.head->prev = scan_ptr;
                }
                threeACcode.head = scan_ptr;
                scan_ptr->prev = NULL;
            }

            if (threeACcode.tail == hoist_point) {
                threeACcode.tail = scan_ptr;
            }

            hoist_point = scan_ptr; // The next DEFVAR will be inserted after this one
        }
        scan_ptr = next_scan;
    }

    threeACcode.active = loop_end_node;

    threeACcode.while_used = false;
    threeACcode.if_used = if_used_backup;
}

void parse_function_call(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isStatement)
{
    char *funcName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(funcName, (*currentToken)->data);
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    // 1. Evaluate argument expressions
    int argCount = 0;
    if ((*currentToken)->type != T_RIGHT_PAREN)
    {
        parse_expression(file, currentToken, stack);
        argCount++;
        while((*currentToken)->type == T_COMMA) {
            get_next_token(file, currentToken);
            skip_optional_eol(currentToken, file);
            parse_expression(file, currentToken, stack);
            argCount++;
        }
    }

    if (isStatement)
    {
        expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);
    }
    else
    {
        if ((*currentToken)->type != T_RIGHT_PAREN)
        {
            fprintf(stderr, "[SYNTAX] Error: expected ')'\n");
            exit(SYNTAX_ERROR);
        }
    }

    emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);

    for (int i = 0; i < argCount; i++) {
        char param_name[20];
        sprintf(param_name, "%%param%d", i);
        Operand* tf_param = create_operand_from_tf_variable(param_name);
        emit(OP_DEFVAR, tf_param, NULL, NULL, &threeACcode);
    }

    for (int i = argCount - 1; i >= 0; i--) {
        char param_name[20];
        sprintf(param_name, "%%param%d", i);
        Operand* tf_param = create_operand_from_tf_variable(param_name);
        emit(OP_POPS, tf_param, NULL, NULL, &threeACcode);
    }

    // 3. Call function
    emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);

    int mangled_len = strlen(funcName) + 1 + 10 + strlen("%func") + 1;
    char *mangledName = safeMalloc(mangled_len);
    sprintf(mangledName, "%s$%d%%func", funcName, argCount);
    Operand* call_label = create_operand_from_label(mangledName);
    free(mangledName);

    emit(OP_CALL, call_label, NULL, NULL, &threeACcode);
    emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);

    // 4. Push return value for expression evaluation
    Operand* retval = create_operand_from_tf_variable("%retval");
    emit(OP_PUSHS, retval, NULL, NULL, &threeACcode);

    // 5. Semantic checks and forward declaration
    int keyLength = strlen(funcName) + 1 + 10 + 1;
    char *key = safeMalloc(keyLength);
    sprintf(key, "%s@%d", funcName, argCount);

    tSymbolData *func = symtable_find(global_symtable, key);

    if (!func)
    {
        tSymbolData forwardDecl = {0};
        forwardDecl.kind = SYM_FUNC;
        forwardDecl.dataType = TYPE_UNDEF;
        forwardDecl.returnType = TYPE_UNDEF;
        forwardDecl.index = -1;
        forwardDecl.defined = false;
        forwardDecl.paramCount = argCount;
        forwardDecl.paramNames = NULL;
        forwardDecl.unique_name = NULL;

        if (argCount > 0)
        {
            forwardDecl.paramTypes = safeMalloc(sizeof(tDataType) * argCount);
            for (int i = 0; i < argCount; i++)
                forwardDecl.paramTypes[i] = TYPE_UNDEF;
        }
        else
        {
            forwardDecl.paramTypes = NULL;
        }

        if (!symtable_insert(global_symtable, key, forwardDecl))
        {
            fprintf(stderr, "[PARSER] Error inserting forward decl for '%s'\n", key);
            free(funcName);
            free(key);
            exit(INTERNAL_ERROR);
        }
    }
    else if (func->kind != SYM_FUNC)
    {
        fprintf(stderr, "[PARSER] Error: '%s' is not a function\n", funcName);
        free(funcName);
        free(key);
        exit(UNDEFINED_FUN_ERROR);
    }

    free(funcName);
    free(key);
}


tDataType get_type_from_token(tToken token)
{
    switch (token->type)
    {
        case T_INTEGER:
        case T_FLOAT:
            return TYPE_NUM;
        case T_STRING:
            return TYPE_STRING;
        case T_KW_NULL_VALUE:
            return TYPE_NULL;
        default:
            return TYPE_UNDEF;
    }
}

tDataType parse_ifj_call(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isStatement)
{
    expect_and_consume(T_KW_IFJ, currentToken, file, false, NULL);
    expect_and_consume(T_DOT, currentToken, file, false, NULL);

    if ((*currentToken)->type != T_ID)
    {
        exit(SYNTAX_ERROR);
    }

    size_t fullNameLen = strlen("Ifj.") + strlen((*currentToken)->data) + 1;
    char *fullName = safeMalloc(fullNameLen);
    sprintf(fullName, "Ifj.%s", (*currentToken)->data);
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);


    int argCount = 0;
    if ((*currentToken)->type != T_RIGHT_PAREN)
    {
        parse_expression(file, currentToken, stack);
        argCount++;
        while((*currentToken)->type == T_COMMA) {
            get_next_token(file, currentToken);
            skip_optional_eol(currentToken, file);
            parse_expression(file, currentToken, stack);
            argCount++;
        }
    }

    if (isStatement)
    {
        expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);
    }
    else
    {
        if ((*currentToken)->type != T_RIGHT_PAREN)
        {
            fprintf(stderr, "[SYNTAX] Error: expected ')'\n");
            exit(SYNTAX_ERROR);
        }
    }


    tSymbolData *funcData = symtable_find(global_symtable, fullName);
    if (funcData == NULL || funcData->kind != SYM_FUNC)
    {
        fprintf(stderr, "[SEMANTIC] Error: undefined IFJ function '%s'\n", fullName);
        free(fullName);
        exit(UNDEFINED_FUN_ERROR);
    }

    semantic_check_argument_count(funcData, argCount, fullName);

    tDataType returnType;

    if (strcmp(fullName, "Ifj.write") == 0)
    {
      returnType = generate_ifj_write(stack);
    }
    else if (strcmp(fullName, "Ifj.read_str") == 0)
    {
      returnType = generate_ifj_read_str(stack);
    }
    else if (strcmp(fullName, "Ifj.strcmp") == 0)
    {
      returnType = generate_ifj_strcmp(stack);
    }
    else if (strcmp(fullName, "Ifj.ord") == 0)
    {
      returnType = generate_ifj_ord(stack);
    }
    else if (strcmp(fullName, "Ifj.read_num") == 0)
    {
      returnType = generate_ifj_read_num(stack);
    }
    else if (strcmp(fullName, "Ifj.floor") == 0)
    {
      returnType = generate_ifj_floor(stack);
    }
    else if (strcmp(fullName, "Ifj.str") == 0)
    {
      returnType = generate_ifj_str(stack);
    }
    else if (strcmp(fullName, "Ifj.length") == 0)
    {
      returnType = generate_ifj_length(stack);
    }
    else if (strcmp(fullName, "Ifj.substring") == 0)
    {
      returnType = generate_ifj_substring(stack);
    }
    else if (strcmp(fullName, "Ifj.chr") == 0)
    {
      returnType = generate_ifj_chr(stack);
    }

    free(fullName);
    return returnType;
}
