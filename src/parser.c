#include "parser.h"
#include "scanner.h"
#include "symtable.h"
#include <stdio.h>
#include "semantic.h"

static tToken peek_buffer = NULL;

static tBuiltinDef builtin_defs[] = {
    {"Ifj.write", TYPE_NULL, 1, {TYPE_UNDEF}},
    {"Ifj.read_num", TYPE_NUM, 0, {}},
    {"Ifj.read_str", TYPE_STRING, 0, {}},
    {"Ifj.floor", TYPE_NUM, 1, {TYPE_NUM}},
    {"Ifj.str", TYPE_STRING, 1, {TYPE_NUM}},
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
        exit(SYNTAX_ERROR);
    }

    do {
        get_next_token(file, currentToken);
    } while ((*currentToken)->type == T_EOL);
}

tSymbolData *find_data_in_stack(tSymTableStack *stack, char *key)
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

    int paramCount = parse_parameter_list(file, currentToken, stack);

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
    }
    else
    {
        tSymbolData funcData = {0};
        funcData.kind = SYM_FUNC;
        funcData.dataType = TYPE_UNDEF;
        funcData.index = -1;
        funcData.defined = false;
        funcData.paramCount = paramCount;

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
}

void parse_getter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName)
{
    int keyLength = strlen("getter:") + strlen(funcName) + 3;
    char *key = safeMalloc(keyLength);
    sprintf(key, "getter:%s@0", funcName);

    tSymbolData getterData = {0};
    getterData.kind = SYM_FUNC;
    getterData.dataType = TYPE_UNDEF;
    getterData.index = -1;
    getterData.defined = false;
    getterData.paramCount = 0;

    if (!symtable_insert(global_symtable, key, getterData))
    {
        free(key);
        exit(REDEFINITION_FUN_ERROR);
    }

    tSymTable *getterSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(getterSymtable);
    symtable_stack_push(stack, getterSymtable);

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

    tSymbolData setterData = {0};
    setterData.kind = SYM_FUNC;
    setterData.dataType = TYPE_UNDEF;
    setterData.index = -1;
    setterData.defined = false;
    setterData.paramCount = 1;
    setterData.paramTypes = safeMalloc(sizeof(tDataType));
    setterData.paramTypes[0] = TYPE_UNDEF;

    if (!symtable_insert(global_symtable, key, setterData))
    {
        free(key);
        free(setterData.paramTypes);
        exit(REDEFINITION_FUN_ERROR);
    }

    tSymTable *setterSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(setterSymtable);
    symtable_stack_push(stack, setterSymtable);

    tSymbolData paramData = {0};
    paramData.kind = SYM_VAR;
    paramData.dataType = TYPE_UNDEF;
    paramData.index = -1;
    symtable_insert(setterSymtable, paramName, paramData);
    free(paramName);

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
}

int parse_parameter_list(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    int paramCount = 0;
    tSymTable *currentSymtable = symtable_stack_top(stack);

    if ((*currentToken)->type == T_RIGHT_PAREN)
    {
        return 0;
    }

    while (true)
    {
        if ((*currentToken)->type != T_ID)
        {
            exit(SYNTAX_ERROR);
        }

        char *paramName = safeMalloc(strlen((*currentToken)->data) + 1);
        strcpy(paramName, (*currentToken)->data);

        tSymbolData paramData = {0};
        paramData.kind = SYM_VAR;
        paramData.dataType = TYPE_UNDEF;
        paramData.index = -1;

        if (!symtable_insert(currentSymtable, paramName, paramData))
        {
            free(paramName);
            exit(REDEFINITION_FUN_ERROR);
        }
        free(paramName);

        paramCount++;
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
            parse_return_statement(file, currentToken, stack);
            break;
        case T_KW_VAR:
            parse_variable_declaration(file, currentToken, stack);
            break;
        case T_ID:
        case T_GLOBAL_ID:
            parse_assignment_statement(file, currentToken, stack);
            break;
        case T_KW_IFJ:
            parse_ifj_call(file, currentToken, stack);
            break;
        default:
            fprintf(stderr, "[PARSER] Syntax error: unexpected token '%s'\n", (*currentToken)->data);
            exit(SYNTAX_ERROR);
            break;
    }
}

void parse_if_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    parse_expression(file, currentToken, stack);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    parse_block(file, currentToken, stack, false);

    if ((*currentToken)->type == T_KW_ELSE)
    {
        get_next_token(file, currentToken);
        parse_block(file, currentToken, stack, false);
    }
}

void parse_while_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    parse_expression(file, currentToken, stack);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    parse_block(file, currentToken, stack, false);
}

void parse_for_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);
}

void parse_return_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    if ((*currentToken)->type != T_EOL)
    {
        parse_expression(file, currentToken, stack);
    }
}

void parse_assignment_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    bool isGlobal = ((*currentToken)->type == T_GLOBAL_ID);

    char *varName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(varName, (*currentToken)->data);
    get_next_token(file, currentToken);

    int keyLength = strlen("setter:") + strlen(varName) + 3;
    char *setterKey = safeMalloc(keyLength);
    sprintf(setterKey, "setter:%s@1", varName);

    tSymbolData *setterSymbol = symtable_find(global_symtable, setterKey);
    free(setterKey);

    if (setterSymbol != NULL)
    {
        expect_and_consume(T_ASSIGN, currentToken, file, false, NULL);
        skip_optional_eol(currentToken, file);
        parse_expression(file, currentToken, stack);

        free(varName);
        return;
    }

    if (isGlobal)
    {
        if (symtable_find(global_symtable, varName) == NULL)
        {
            tSymbolData globalVar = {0};
            globalVar.kind = SYM_VAR;
            globalVar.dataType = TYPE_UNDEF;
            globalVar.index = -1;
            symtable_insert(global_symtable, varName, globalVar);
        }
    }
    else
    {
        tSymbolData *local = find_data_in_stack(stack, varName);
        if (local == NULL)
        {
            fprintf(stderr, "[PARSER] Error: assignment to undefined variable '%s'\n", varName);
            free(varName);
            exit(UNDEFINED_FUN_ERROR);
        }
    }

    free(varName);

    expect_and_consume(T_ASSIGN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    tToken nextToken = peek_token(file);
    if ((*currentToken)->type == T_ID && nextToken->type == T_LEFT_PAREN)
    {
        parse_function_call(file, currentToken, stack);
    }
    else if ((*currentToken)->type == T_KW_IFJ)
    {
        parse_ifj_call(file, currentToken, stack);
    }
    else
    {
        parse_expression(file, currentToken, stack);
    }
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
    
    get_next_token(file, currentToken);

    if ((*currentToken)->type == T_ASSIGN)
    {
        get_next_token(file, currentToken);
        parse_expression(file, currentToken, stack);
    }
}

void parse_function_call(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    char *funcName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(funcName, (*currentToken)->data);
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    int argCount = 0;
    if ((*currentToken)->type != T_RIGHT_PAREN)
    {
        parse_term(file, currentToken, stack);
        argCount++;
        while ((*currentToken)->type == T_COMMA)
        {
            get_next_token(file, currentToken);
            skip_optional_eol(currentToken, file);
            parse_term(file, currentToken, stack);
            argCount++;
        }
    }

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    int keyLength = strlen(funcName) + 1 + 10 + 1;
    char *key = safeMalloc(keyLength);
    sprintf(key, "%s@%d", funcName, argCount);

    tSymbolData *func = symtable_find(global_symtable, key);

    if (!func)
    {
        // Dopredná deklarácia funkcie
        tSymbolData forwardDecl = {0};
        forwardDecl.kind = SYM_FUNC;
        forwardDecl.dataType = TYPE_UNDEF;
        forwardDecl.returnType = TYPE_UNDEF;
        forwardDecl.index = -1;
        forwardDecl.defined = false;
        forwardDecl.paramCount = argCount;

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

void parse_term(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    switch ((*currentToken)->type)
    {
        case T_INTEGER:
        case T_FLOAT:
        case T_STRING:
        case T_KW_NULL_VALUE:
        case T_GLOBAL_ID:
            get_next_token(file, currentToken);
            break;

        case T_ID:
        {
            int keyLength = strlen("getter:") + strlen((*currentToken)->data) + 3;
            char *getterKey = safeMalloc(keyLength);
            sprintf(getterKey, "getter:%s@0", (*currentToken)->data);
            if (symtable_find(global_symtable, getterKey))
            {
                free(getterKey);
                get_next_token(file, currentToken);
                break;
            }
            free(getterKey);

            tToken nextToken = peek_token(file);
            if (nextToken->type == T_LEFT_PAREN)
            {
                parse_function_call(file, currentToken, stack);
            }
            else
            {
                get_next_token(file, currentToken);
            }
            break;
        }

        case T_KW_IFJ:
            parse_ifj_call(file, currentToken, stack);
            break;

        default:
            fprintf(stderr, "[PARSER] Invalid term '%s'\n", (*currentToken)->data);
            exit(SYNTAX_ERROR);
    }
}

void parse_ifj_call(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    expect_and_consume(T_KW_IFJ, currentToken, file, false, NULL);

    expect_and_consume(T_DOT, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

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

    tDataType argTypes[3];
    int argCount = 0;
    if ((*currentToken)->type != T_RIGHT_PAREN)
    {
        argTypes[0] = get_type_from_token(*currentToken);
        parse_term(file, currentToken, stack);
        argCount++;
        while ((*currentToken)->type == T_COMMA)
        {
            get_next_token(file, currentToken);
            skip_optional_eol(currentToken, file);
            parse_term(file, currentToken, stack);
            argTypes[argCount++] = get_type_from_token(*currentToken);
        }
    }

    
    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    tSymbolData *funcData = symtable_find(global_symtable, fullName);

    semantic_check_ifj_call(funcData, argTypes, argCount, fullName);

    free(fullName);
}
