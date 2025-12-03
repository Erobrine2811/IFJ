/**
 * @file parser.c
 *
 * IFJ25 project
 *
 * Syntactic and semantic analyzer
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#include "3AC_patterns.h"
#include "parser.h"
#include "scanner.h"

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
            fprintf(stderr, "[PARSER] LexicalError:%d:%d: Failed to get next token.\n",
                    peek_buffer->linePos, peek_buffer->colPos);
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
        fprintf(stderr, "[PARSER] LexicalError:%d:%d: Failed to get next token.\n",
                (*currentToken)->linePos, (*currentToken)->colPos);
        exit(LEXICAL_ERROR);
    }
}

void expect_and_consume(tType type, tToken *currentToken, FILE *file, bool checkValue,
                               const char *value)
{
    if ((*currentToken)->type != type)
    {
        fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'.\n",
                (*currentToken)->linePos, (*currentToken)->colPos,
                typeToString((*currentToken)->type));
        fprintf(stderr, "\t Expected: '%s'\n", typeToString(type));
        exit(SYNTAX_ERROR);
    }

    if (checkValue)
    {
        if (strcmp((*currentToken)->data, value) != 0)
        {
            fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token value: %s.\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, (*currentToken)->data);
            fprintf(stderr, "\t Expected: %s\n", value);
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
        fprintf(stderr, "[PARSER] SyntaxError: Expected EOL, but got token of type %s at %d:%d\n",
                typeToString((*currentToken)->type), (*currentToken)->linePos,
                (*currentToken)->colPos);
        exit(SYNTAX_ERROR);
    }

    do
    {
        get_next_token(file, currentToken);
    } while ((*currentToken)->type == T_EOL);
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
    skip_optional_eol(currentToken, file);
    expect_and_consume(T_STRING, currentToken, file, true, "\"ifj25\"");
    expect_and_consume(T_KW_FOR, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);
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
        fprintf(stderr, "[PARSER] SemanticError: undefined function 'main' with 0 parameters\n");
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
            fprintf(stderr, "[INTERNAL] Could not insert builtin '%s' into symbol table\n",
                    def->name);
        }
    }
}

void parse_func_list(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    if ((*currentToken)->type != T_KW_STATIC)
    {
        fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Missing expected 'static' keyword\n",
                (*currentToken)->linePos, (*currentToken)->colPos);
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
        fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'.\n",
                (*currentToken)->linePos, (*currentToken)->colPos,
                typeToString((*currentToken)->type));
        fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_ID));
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

        fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'.\n",
                (*currentToken)->linePos, (*currentToken)->colPos,
                typeToString((*currentToken)->type));
        fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_LEFT_PAREN));
        exit(SYNTAX_ERROR);
    }

    get_next_token(file, currentToken);
    skip_optional_eol(currentToken, file);

    tSymTable *funcSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(funcSymtable);
    symtable_stack_push(stack, funcSymtable);

    char **paramNames = NULL;
    int paramCount = parse_parameter_list(file, currentToken, stack, &paramNames);

    int mangledLen = strlen(funcName) + 1 + 10 + strlen("%func") + 1;
    char *mangledName = safeMalloc(mangledLen);
    sprintf(mangledName, "%s$%d%%func", funcName, paramCount);

    tOperand *labelOp = safeMalloc(sizeof(tOperand));
    labelOp->type = OPP_LABEL;
    labelOp->value.label = mangledName;
    char *commentText = safeMalloc(strlen(funcName) + 25);
    sprintf(commentText, "####################");
    emit_comment(commentText, &threeACcode);
    sprintf(commentText, "Function declaration: %s", funcName);
    emit_comment(commentText, &threeACcode);
    sprintf(commentText, "####################");
    emit_comment(commentText, &threeACcode);
    free(commentText);
    emit(OP_LABEL, labelOp, NULL, NULL, &threeACcode);

    tOperand *retvalDef = create_operand_from_variable("%retval", false);
    emit(OP_DEFVAR, retvalDef, NULL, NULL, &threeACcode);
    tOperand *retvalInit = create_operand_from_variable("%retval", false);
    tOperand *nilOp = create_operand_from_constant_nil();
    emit(OP_MOVE, retvalInit, nilOp, NULL, &threeACcode);

    tSymTable *funcScopeTable = symtable_stack_top(stack);

    for (int i = 0; i < paramCount; i++)
    {
        tSymbolData *paramData = symtable_find(funcScopeTable, paramNames[i]);
        tOperand *paramOp = create_operand_from_variable(paramData->unique_name, false);
        emit(OP_DEFVAR, paramOp, NULL, NULL, &threeACcode);
    }

    for (int i = 0; i < paramCount; i++)
    {
        char tempParamName[20];
        sprintf(tempParamName, "%%param%d", i);

        tSymbolData *paramData = symtable_find(funcScopeTable, paramNames[i]);

        tOperand *dest = create_operand_from_variable(paramData->unique_name, false);
        tOperand *src = create_operand_from_variable(tempParamName, false);
        emit(OP_MOVE, dest, src, NULL, &threeACcode);
    }

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    threeACcode.tempCounter = 0;

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    int keyLength = strlen(funcName) + 1 + 10 + 1;
    char *key = safeMalloc(keyLength);
    sprintf(key, "%s@%d", funcName, paramCount);

    tSymbolData *existing = symtable_find(global_symtable, key);
    if (existing != NULL)
    {
        if (existing->defined)
        {
            fprintf(stderr, "[PARSER] SemanticError:%d:%d: Function '%s' redefined\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, funcName);
            free(funcName);
            free(key);
            exit(REDEFINITION_FUN_ERROR);
        }
        existing->paramCount = paramCount;
        existing->paramNames = paramNames;
        if (existing->paramTypes)
            free(existing->paramTypes);
        if (paramCount > 0)
        {
            existing->paramTypes = safeMalloc(sizeof(tDataType) * paramCount);
            for (int i = 0; i < paramCount; i++)
                existing->paramTypes[i] = TYPE_UNDEF;
        }
        else
        {
            existing->paramTypes = NULL;
        }
    }
    else
    {
        tSymbolData funcData = {0};
        funcData.kind = SYM_FUNC;
        funcData.dataType = TYPE_UNDEF;
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
            fprintf(stderr,
                    "[INTERNAL] Error:%d:%d: Failed to insert function '%s' into symbol table\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, funcName);
            free(funcName);
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
    free(funcName);
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
        fprintf(stderr, "[PARSER] SemanticError:%d:%d: Getter '%s' redefined\n",
                (*currentToken)->linePos, (*currentToken)->colPos, funcName);
        free(key);
        exit(REDEFINITION_FUN_ERROR);
    }

    if (alreadyDefined == NULL)
    {
        tSymbolData getterData = {0};
        getterData.kind = SYM_FUNC;
        getterData.dataType = TYPE_UNDEF;
        getterData.defined = false;
        getterData.paramCount = 0;
        getterData.paramNames = NULL;
        getterData.unique_name = NULL;

        if (!symtable_insert(global_symtable, key, getterData))
        {
            fprintf(stderr,
                    "[INTERNAL] Error:%d:%d: Failed to insert getter '%s' into symbol table\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, funcName);
            free(key);
            exit(INTERNAL_ERROR);
        }
    }

    tSymTable *getterSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(getterSymtable);
    symtable_stack_push(stack, getterSymtable);

    int mangledLen = strlen(funcName) + strlen("$0%getter") + 1;
    char *mangledName = safeMalloc(mangledLen);
    sprintf(mangledName, "%s$0%%getter", funcName);

    tOperand *labelOp = create_operand_from_label(mangledName);

    emit_comment("####################", &threeACcode);
    char *commentText = safeMalloc(strlen(funcName) + 25);
    sprintf(commentText, "Function declaration: %s (getter)", funcName);
    emit_comment(commentText, &threeACcode);
    emit_comment("####################", &threeACcode);
    emit(OP_LABEL, labelOp, NULL, NULL, &threeACcode);

    tOperand *retvalDef = create_operand_from_variable("%retval", false);
    emit(OP_DEFVAR, retvalDef, NULL, NULL, &threeACcode);
    tOperand *retvalInit = create_operand_from_variable("%retval", false);
    tOperand *nilOp = create_operand_from_constant_nil();
    emit(OP_MOVE, retvalInit, nilOp, NULL, &threeACcode);

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
        fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token for setter parameter '%s'\n",
                (*currentToken)->linePos, (*currentToken)->colPos,
                typeToString((*currentToken)->type));
        fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_ID));
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
        fprintf(stderr, "[PARSER] SemanticError:%d:%d: Setter '%s' already defined\n",
                (*currentToken)->linePos, (*currentToken)->colPos, funcName);
        free(key);
        free(paramName);
        exit(REDEFINITION_FUN_ERROR);
    }

    if (alreadyDefined == NULL)
    {
        tSymbolData setterData = {0};
        setterData.kind = SYM_FUNC;
        setterData.dataType = TYPE_UNDEF;
        setterData.defined = false;
        setterData.paramCount = 1;
        setterData.paramTypes = safeMalloc(sizeof(tDataType));
        setterData.paramTypes[0] = TYPE_UNDEF;
        setterData.paramNames = NULL;
        setterData.unique_name = NULL;

        if (!symtable_insert(global_symtable, key, setterData))
        {
            fprintf(stderr,
                    "[INTERNAL] Error:%d:%d: Failed to insert setter '%s' into symbol table\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, funcName);
            free(key);
            free(setterData.paramTypes);
            exit(INTERNAL_ERROR);
        }
    }

    tSymTable *setterSymtable = safeMalloc(sizeof(tSymTable));
    symtable_init(setterSymtable);
    symtable_stack_push(stack, setterSymtable);

    tSymbolData paramData = {0};
    paramData.kind = SYM_VAR;
    paramData.dataType = TYPE_UNDEF;

    int len = snprintf(NULL, 0, "%s%%%d", paramName, threeACcode.varCounter);
    paramData.unique_name = safeMalloc(len + 1);
    sprintf(paramData.unique_name, "%s%%%d", paramName, threeACcode.varCounter++);

    symtable_insert(setterSymtable, paramName, paramData);
    free(paramName);

    int mangledLen = strlen(funcName) + strlen("$1%setter") + 1;
    char *mangledName = safeMalloc(mangledLen);
    sprintf(mangledName, "%s$1%%setter", funcName);

    tOperand *labelOp = create_operand_from_label(mangledName);
    emit(OP_LABEL, labelOp, NULL, NULL, &threeACcode);

    tOperand *retvalDef = create_operand_from_variable("%retval", false);
    emit(OP_DEFVAR, retvalDef, NULL, NULL, &threeACcode);
    tOperand *retvalInit = create_operand_from_variable("%retval", false);
    tOperand *nilOp = create_operand_from_constant_nil();
    emit(OP_MOVE, retvalInit, nilOp, NULL, &threeACcode);

    tOperand *setterParamDest = create_operand_from_variable(paramData.unique_name, false);
    tOperand *setterParamSrc = create_operand_from_variable("%param0", false);
    emit(OP_DEFVAR, setterParamDest, NULL, NULL, &threeACcode);
    emit(OP_MOVE, setterParamDest, setterParamSrc, NULL, &threeACcode);

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

int parse_parameter_list(FILE *file, tToken *currentToken, tSymTableStack *stack,
                                char ***paramNames)
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
            fprintf(stderr,
                    "[PARSER] SyntaxError:%d:%d: Unexpected token for fuction parameter '%s'\n",
                    (*currentToken)->linePos, (*currentToken)->colPos,
                    typeToString((*currentToken)->type));
            fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_ID));
            exit(SYNTAX_ERROR);
        }

        char *paramName = safeMalloc(strlen((*currentToken)->data) + 1);
        strcpy(paramName, (*currentToken)->data);

        paramCount++;
        *paramNames = safeRealloc(*paramNames, paramCount * sizeof(char *));
        (*paramNames)[paramCount - 1] = safeMalloc(strlen(paramName) + 1);
        strcpy((*paramNames)[paramCount - 1], paramName);

        tSymbolData paramData = {0};
        paramData.kind = SYM_VAR;
        paramData.dataType = TYPE_UNDEF;

        int len = snprintf(NULL, 0, "%s%%%d", paramName, threeACcode.varCounter);
        paramData.unique_name = safeMalloc(len + 1);
        sprintf(paramData.unique_name, "%s%%%d", paramName, threeACcode.varCounter++);

        if (!symtable_insert(currentSymtable, paramName, paramData))
        {
            fprintf(stderr,
                    "[PARSER] SemanticError:%d:%d: Redefinition of function parameter '%s'\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, paramName);
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
        fprintf(stderr, "[PARSER] SemanticError: Undefined function '%s'\n", node->key);
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

void parse_block(FILE *file, tToken *currentToken, tSymTableStack *stack,
                        bool isFunctionBody)
{
    tSymTable *blockSymtable;

    if (!isFunctionBody)
    {
        blockSymtable = safeMalloc(sizeof(tSymTable));
        symtable_init(blockSymtable);
        symtable_stack_push(stack, blockSymtable);
    }
    else
    {
        blockSymtable = symtable_stack_top(stack);
    }

    expect_and_consume(T_LEFT_BRACE, currentToken, file, false, NULL);

    if ((*currentToken)->type != T_EOL) // ONELINEBLOCK extension
    {
        if (!isFunctionBody)
        {
            if ((*currentToken)->type != T_RIGHT_BRACE)
            {
                parse_expression(file, currentToken, stack);
            }

            expect_and_consume(T_RIGHT_BRACE, currentToken, file, false, NULL);

            symtable_stack_pop(stack);
            symtable_free(blockSymtable);
            free(blockSymtable);
        }
        else
        {
            if ((*currentToken)->type != T_RIGHT_BRACE)
            {
                generate_return(file, currentToken, stack, true);
            }

            expect_and_consume(T_RIGHT_BRACE, currentToken, file, false, NULL);
        }

        return;
    }

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
            generate_return(file, currentToken, stack, false);
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
            fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'.\n",
                    (*currentToken)->linePos, (*currentToken)->colPos,
                    typeToString((*currentToken)->type));
            exit(SYNTAX_ERROR);
            break;
    }
}

void parse_if_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken); // consume 'if'

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    bool whileUsedBackup = threeACcode.whileUsed;
    threeACcode.whileUsed = false;
    threeACcode.ifUsed = true;

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("If statement condition", &threeACcode);
    parse_expression(file, currentToken, stack);

    // Handle truthiness rules
    tOperand *exprValIf = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, exprValIf, NULL, NULL, &threeACcode);
    emit(OP_POPS, exprValIf, NULL, NULL, &threeACcode); // Pop expression result

    tOperand *finalBoolResultIf =
        create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, finalBoolResultIf, NULL, NULL, &threeACcode);

    tOperand *labelIsNullIf = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsBoolIf = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsOtherIf = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelEndTruthinessIf = create_operand_from_label(threeAC_create_label(&threeACcode));

    // Check if null
    emit(OP_JUMPIFEQ, labelIsNullIf, exprValIf, create_operand_from_constant_nil(), &threeACcode);

    // Check if boolean (using TYPE instruction)
    tOperand *typeCheckVarIf =
        create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeCheckVarIf, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeCheckVarIf, exprValIf, NULL, &threeACcode); // Get type of expr_val

    emit(OP_JUMPIFEQ, labelIsBoolIf, typeCheckVarIf, create_operand_from_constant_string("bool"),
         &threeACcode);

    // If not null and not bool, it's true
    emit(OP_JUMP, labelIsOtherIf, NULL, NULL, &threeACcode);

    // Case: is null
    emit(OP_LABEL, labelIsNullIf, NULL, NULL, &threeACcode);
    emit(OP_MOVE, finalBoolResultIf, create_operand_from_constant_bool(false), NULL, &threeACcode);
    emit(OP_JUMP, labelEndTruthinessIf, NULL, NULL, &threeACcode);

    // Case: is boolean
    emit(OP_LABEL, labelIsBoolIf, NULL, NULL, &threeACcode);
    emit(OP_MOVE, finalBoolResultIf, exprValIf, NULL, &threeACcode);
    emit(OP_JUMP, labelEndTruthinessIf, NULL, NULL, &threeACcode);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, labelIsOtherIf, NULL, NULL, &threeACcode);
    emit(OP_MOVE, finalBoolResultIf, create_operand_from_constant_bool(true), NULL, &threeACcode);
    emit(OP_JUMP, labelEndTruthinessIf, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEndTruthinessIf, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, finalBoolResultIf, NULL, NULL, &threeACcode); // Push the final boolean result

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    char *label1Str = threeAC_create_label(&threeACcode);
    tOperand *label1 = create_operand_from_label(label1Str);

    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, label1, NULL, NULL, &threeACcode);

    emit_comment("If-block", &threeACcode);
    parse_block(file, currentToken, stack, false);

    expect_and_consume(T_KW_ELSE, currentToken, file, false, NULL);

    char *label2Str = threeAC_create_label(&threeACcode);
    tOperand *label2 = create_operand_from_label(label2Str);

    emit(OP_JUMP, label2, NULL, NULL, &threeACcode);
    emit(OP_LABEL, label1, NULL, NULL, &threeACcode);

    emit_comment("Else-block", &threeACcode);
    parse_block(file, currentToken, stack, false);

    emit(OP_LABEL, label2, NULL, NULL, &threeACcode);

    free(label2Str);

    free(label1Str);

    emit_comment("If statement end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);

    threeACcode.ifUsed = false;
    threeACcode.whileUsed = whileUsedBackup;
}

void parse_assignment_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    tToken nextToken = peek_token(file);

    if (nextToken && nextToken->type == T_LEFT_PAREN && (*currentToken)->type == T_ID)
    {
        parse_function_call(file, currentToken, stack, true);
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
    tSymbolData *varData = NULL;
    bool isNewDeclaration = false;

    if (isGlobal)
    {
        varData = symtable_find(global_symtable, varName);
        if (varData == NULL)
        {
            isNewDeclaration = true;
            semantic_define_variable(stack, varName, true);
            varData = symtable_find(global_symtable, varName);
        }
    }
    else
    {
        varData = symtable_stack_find(stack, varName);

        if (varData == NULL || setterSymbol != NULL)
        {

            if (varData == NULL && setterSymbol == NULL)
            {
                varData = safeMalloc(sizeof(tSymbolData));
                varData->kind = SYM_FUNC;
                ;
                varData->dataType = TYPE_UNDEF;
                varData->unique_name = NULL;
                varData->defined = false;
                varData->paramCount = 1;
                varData->paramNames = NULL;
                varData->paramTypes = safeMalloc(sizeof(tDataType));
                varData->paramTypes[0] = TYPE_UNDEF;

                if (!symtable_insert(global_symtable, setterKey, *varData))
                {
                    fprintf(stderr, "[PARSER] SemanticError: Variable redefinition for '%s'\n",
                            varName);
                    free(varName);
                    free(varData);
                    exit(REDEFINITION_FUN_ERROR);
                }
            }

            expect_and_consume(T_ASSIGN, currentToken, file, false, NULL);
            skip_optional_eol(currentToken, file);

            parse_expression(file, currentToken, stack);

            emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);

            tOperand *tfParam = create_operand_from_tf_variable("%param0");
            emit(OP_DEFVAR, tfParam, NULL, NULL, &threeACcode);

            tOperand *tfParamPop = create_operand_from_tf_variable("%param0");
            emit(OP_POPS, tfParamPop, NULL, NULL, &threeACcode);

            emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);

            char mangledName[256];
            sprintf(mangledName, "%s$1%%setter", varName);
            tOperand *callLabel = create_operand_from_label(mangledName);
            emit(OP_CALL, callLabel, NULL, NULL, &threeACcode);

            emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);

            free(varName);
            free(setterKey);
            return;
        }
    }

    if (!isNewDeclaration)
    {
        char *commentText = safeMalloc(strlen(varName) + 30);
        sprintf(commentText, "Assignment to variable '%s'", varName);
        emit_comment(commentText, &threeACcode);
        free(commentText);
    }

    free(setterKey);

    expect_and_consume(T_ASSIGN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    nextToken = peek_token(file);
    tDataType exprType;

    parse_expression(file, currentToken, stack);

    if (varData)
    {
        varData->dataType = TYPE_UNDEF;
    }

    tOperand *popsVarOp = create_operand_from_variable(varData->unique_name, isGlobal);
    emit(OP_POPS, popsVarOp, NULL, NULL, &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    free(varName);
}

void parse_variable_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    if ((*currentToken)->type != T_ID && (*currentToken)->type != T_GLOBAL_ID)
    {
        fprintf(
            stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token for variable declaration '%s'\n",
            (*currentToken)->linePos, (*currentToken)->colPos, typeToString((*currentToken)->type));
        fprintf(stderr, "\t Expected: '%s' or '%s'\n", typeToString(T_ID),
                typeToString(T_GLOBAL_ID));
        exit(SYNTAX_ERROR);
    }

    bool isGlobal = ((*currentToken)->type == T_GLOBAL_ID);

    char *variableName = safeMalloc(strlen((*currentToken)->data) + 1);
    strcpy(variableName, (*currentToken)->data);

    semantic_define_variable(stack, variableName, isGlobal);

    tSymTable *targetTable = isGlobal ? global_symtable : symtable_stack_top(stack);
    tSymbolData *varSymData = symtable_find(targetTable, variableName);

    tOperand *varOp = safeMalloc(sizeof(tOperand));
    varOp->type = isGlobal ? OPP_GLOBAL : OPP_VAR;
    varOp->value.varname = safeMalloc(strlen(varSymData->unique_name) + 1);
    strcpy(varOp->value.varname, varSymData->unique_name);

    char *commentText = safeMalloc(strlen(variableName) + 30);
    sprintf(commentText, "Declaration of variable '%s'", variableName);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment(commentText, &threeACcode);
    free(commentText);

    emit(OP_DEFVAR, varOp, NULL, NULL, &threeACcode);
    get_next_token(file, currentToken);

    if ((*currentToken)->type == T_ASSIGN)
    {
        get_next_token(file, currentToken);
        tDataType exprType;

        exprType = parse_expression(file, currentToken, stack);

        tSymbolData *varData = isGlobal ? symtable_find(global_symtable, variableName)
                                        : symtable_stack_find(stack, variableName);
        if (varData)
        {
            varData->dataType = exprType;
        }
        emit(OP_POPS, varOp, NULL, NULL, &threeACcode);
    }
}

void parse_while_statement(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    bool ifUsedBackup = threeACcode.ifUsed;
    threeACcode.ifUsed = false;
    threeACcode.whileUsed = true;

    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("While loop start", &threeACcode);
    tInstructionNode *hoistPoint = threeACcode.active;

    char *loopStartLabelStr = threeAC_create_label(&threeACcode);
    char *loopEndLabelStr = threeAC_create_label(&threeACcode);

    tOperand *loopStartLabel = safeMalloc(sizeof(tOperand));
    loopStartLabel->type = OPP_LABEL;
    loopStartLabel->value.label = loopStartLabelStr;

    tOperand *loopEndLabel = safeMalloc(sizeof(tOperand));
    loopEndLabel->type = OPP_LABEL;
    loopEndLabel->value.label = loopEndLabelStr;

    emit(OP_LABEL, loopStartLabel, NULL, NULL, &threeACcode);
    emit_comment("While condition", &threeACcode);

    parse_expression(file, currentToken, stack);

    // Handle truthiness rules
    tOperand *exprValWhile = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, exprValWhile, NULL, NULL, &threeACcode);
    emit(OP_POPS, exprValWhile, NULL, NULL, &threeACcode); // Pop expression result

    tOperand *finalBoolResultWhile =
        create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, finalBoolResultWhile, NULL, NULL, &threeACcode);

    tOperand *labelIsNullWhile = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsBoolWhile = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsOtherWhile = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelEndTruthinessWhile =
        create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_JUMPIFEQ, labelIsNullWhile, exprValWhile, create_operand_from_constant_nil(),
         &threeACcode);

    tOperand *typeCheckVarWhile =
        create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeCheckVarWhile, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeCheckVarWhile, exprValWhile, NULL, &threeACcode); // Get type of expr_val

    emit(OP_JUMPIFEQ, labelIsBoolWhile, typeCheckVarWhile,
         create_operand_from_constant_string("bool"), &threeACcode);

    emit(OP_JUMP, labelIsOtherWhile, NULL, NULL, &threeACcode);

    // Case: is null
    emit(OP_LABEL, labelIsNullWhile, NULL, NULL, &threeACcode);
    emit(OP_MOVE, finalBoolResultWhile, create_operand_from_constant_bool(false), NULL,
         &threeACcode);
    emit(OP_JUMP, labelEndTruthinessWhile, NULL, NULL, &threeACcode);

    // Case: is boolean
    emit(OP_LABEL, labelIsBoolWhile, NULL, NULL, &threeACcode);
    emit(OP_MOVE, finalBoolResultWhile, exprValWhile, NULL, &threeACcode);
    emit(OP_JUMP, labelEndTruthinessWhile, NULL, NULL, &threeACcode);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, labelIsOtherWhile, NULL, NULL, &threeACcode);
    emit(OP_MOVE, finalBoolResultWhile, create_operand_from_constant_bool(true), NULL,
         &threeACcode);
    emit(OP_JUMP, labelEndTruthinessWhile, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEndTruthinessWhile, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, finalBoolResultWhile, NULL, NULL, &threeACcode); // Push the final boolean result

    tOperand *conditionResult = safeMalloc(sizeof(tOperand));
    conditionResult->type = OPP_TEMP;
    conditionResult->value.varname = threeAC_create_temp(&threeACcode);
    emit(OP_DEFVAR, conditionResult, NULL, NULL, &threeACcode);
    emit(OP_POPS, conditionResult, NULL, NULL, &threeACcode);

    tOperand *constFalse = safeMalloc(sizeof(tOperand));
    constFalse->type = OPP_CONST_BOOL;
    constFalse->value.boolval = false;

    emit(OP_JUMPIFEQ, loopEndLabel, conditionResult, constFalse, &threeACcode);

    expect_and_consume(T_RIGHT_PAREN, currentToken, file, false, NULL);

    emit_comment("While body", &threeACcode);
    parse_block(file, currentToken, stack, false);

    emit(OP_JUMP, loopStartLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loopEndLabel, NULL, NULL, &threeACcode);
    emit_comment("While loop end", &threeACcode);
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    tInstructionNode *loopEndNode = threeACcode.active;

    // Hoist DEFVARs
    tInstructionNode *scanPtr = hoistPoint ? hoistPoint->next : threeACcode.head;
    while (scanPtr != NULL && scanPtr != loopEndNode)
    {
        tInstructionNode *nextScan = scanPtr->next;
        if (scanPtr->opType == OP_DEFVAR &&
            (scanPtr->result->type == OPP_VAR || scanPtr->result->type == OPP_TEMP))
        {
            // Unlink from current position
            scanPtr->prev->next = scanPtr->next;
            if (scanPtr->next)
            {
                scanPtr->next->prev = scanPtr->prev;
            }
            else
            {
                threeACcode.tail = scanPtr->prev;
            }

            // Insert after hoistPoint
            if (hoistPoint)
            {
                scanPtr->next = hoistPoint->next;
                if (hoistPoint->next)
                {
                    hoistPoint->next->prev = scanPtr;
                }
                hoistPoint->next = scanPtr;
                scanPtr->prev = hoistPoint;
            }
            else
            { // Hoisting to the very beginning of the list
                scanPtr->next = threeACcode.head;
                if (threeACcode.head)
                {
                    threeACcode.head->prev = scanPtr;
                }
                threeACcode.head = scanPtr;
                scanPtr->prev = NULL;
            }

            if (threeACcode.tail == hoistPoint)
            {
                threeACcode.tail = scanPtr;
            }

            hoistPoint = scanPtr; // The next DEFVAR will be inserted after this one
        }
        scanPtr = nextScan;
    }

    threeACcode.active = loopEndNode;

    threeACcode.whileUsed = false;
    threeACcode.ifUsed = ifUsedBackup;
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
        while ((*currentToken)->type == T_COMMA)
        {
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
            fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'.\n",
                    (*currentToken)->linePos, (*currentToken)->colPos,
                    typeToString((*currentToken)->type));
            fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_RIGHT_PAREN));
            exit(SYNTAX_ERROR);
        }
    }

    emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);

    for (int i = 0; i < argCount; i++)
    {
        char paramName[20];
        sprintf(paramName, "%%param%d", i);
        tOperand *tfParam = create_operand_from_tf_variable(paramName);
        emit(OP_DEFVAR, tfParam, NULL, NULL, &threeACcode);
    }

    for (int i = argCount - 1; i >= 0; i--)
    {
        char paramName[20];
        sprintf(paramName, "%%param%d", i);
        tOperand *tfParam = create_operand_from_tf_variable(paramName);
        emit(OP_POPS, tfParam, NULL, NULL, &threeACcode);
    }

    // 3. Call function
    emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);

    int mangledLen = strlen(funcName) + 1 + 10 + strlen("%func") + 1;
    char *mangledName = safeMalloc(mangledLen);
    sprintf(mangledName, "%s$%d%%func", funcName, argCount);
    tOperand *callLabel = create_operand_from_label(mangledName);
    free(mangledName);

    emit(OP_CALL, callLabel, NULL, NULL, &threeACcode);
    emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);

    // 4. Push return value for expression evaluation
    tOperand *retval = create_operand_from_tf_variable("%retval");
    emit(OP_PUSHS, retval, NULL, NULL, &threeACcode);

    // 5. Semantic checks and forward declaration
    int keyLength = strlen(funcName) + 1 + 10 + 1;
    char *key = safeMalloc(keyLength);
    sprintf(key, "%s@%d", funcName, argCount);

    tSymbolData *funcData = symtable_find(global_symtable, key);

    if (!funcData)
    {
        if (symtable_find_function(global_symtable, key))
        {
            fprintf(stderr,
                    "[PARSER] SemanticError:%d:%d: Wrong argument count for function '%s'\n",
                    (*currentToken)->linePos, (*currentToken)->colPos, funcName);
            free(funcName);
            free(key);
            exit(WRONG_ARGUMENT_COUNT_ERROR);
        }

        tSymbolData forwardDecl = {0};
        forwardDecl.kind = SYM_FUNC;
        forwardDecl.dataType = TYPE_UNDEF;
        forwardDecl.returnType = TYPE_UNDEF;
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
            fprintf(stderr,
                    "[INTERNAL] Error: Unable to insert forward declaration for '%s' into symbol "
                    "table\n",
                    funcName);
            free(funcName);
            free(key);
            exit(INTERNAL_ERROR);
        }
    }
    else if (funcData->kind != SYM_FUNC)
    {
        fprintf(stderr, "[PARSER] SemanticError: '%s' is not a function\n", funcName);
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
    skip_optional_eol(currentToken, file);

    if ((*currentToken)->type != T_ID)
    {
        fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'\n",
                (*currentToken)->linePos, (*currentToken)->colPos,
                typeToString((*currentToken)->type));
        fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_ID));
        exit(SYNTAX_ERROR);
    }

    size_t fullNameLen = strlen("Ifj.") + strlen((*currentToken)->data) + 1;
    char *fullName = safeMalloc(fullNameLen);
    sprintf(fullName, "Ifj.%s", (*currentToken)->data);
    get_next_token(file, currentToken);

    expect_and_consume(T_LEFT_PAREN, currentToken, file, false, NULL);
    skip_optional_eol(currentToken, file);

    int argCount = 0;
    tDataType argTypes[3];
    if ((*currentToken)->type != T_RIGHT_PAREN)
    {
        argTypes[argCount] = parse_expression(file, currentToken, stack);
        argCount++;
        while ((*currentToken)->type == T_COMMA)
        {
            get_next_token(file, currentToken);
            skip_optional_eol(currentToken, file);
            argTypes[argCount] = parse_expression(file, currentToken, stack);
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
            fprintf(stderr, "[PARSER] SyntaxError:%d:%d: Unexpected token '%s'\n",
                    (*currentToken)->linePos, (*currentToken)->colPos,
                    typeToString((*currentToken)->type));
            fprintf(stderr, "\t Expected: '%s'\n", typeToString(T_RIGHT_PAREN));
            exit(SYNTAX_ERROR);
        }
    }

    tSymbolData *funcData = symtable_find(global_symtable, fullName);
    if (funcData == NULL || funcData->kind != SYM_FUNC)
    {
        fprintf(stderr, "[PARSER] SemanticError:%d:%d: Undefined built-in function '%s'\n",
                (*currentToken)->linePos, (*currentToken)->colPos, fullName);
        free(fullName);
        exit(UNDEFINED_FUN_ERROR);
    }

    semantic_check_argument_count(funcData, argCount, fullName);
    semantic_check_argument_types(funcData, argTypes, argCount, fullName);

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
