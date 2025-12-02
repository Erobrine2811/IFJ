#include <stdbool.h>
#include <stdlib.h>

#include "3AC.h"
#include "helper.h"

void list_init(tThreeACList *list)
{
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
    list->active = list->head;
    list->tempCounter = 0;
    list->varCounter = 0;
    list->expressionResult = NULL;
    list->returnUsed = false;
    list->whileUsed = false;
    list->ifUsed = false;
    list->globalDefHead = NULL;
    list->globalDefTail = NULL;
}

void list_dispose(tThreeACList *list)
{

    tInstructionNode *current = list->head;
    tInstructionNode *next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
    list->active = NULL;

    current = list->globalDefHead;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    list->globalDefHead = NULL;
    list->globalDefTail = NULL;
}

void list_first(tThreeACList *list)
{
    list->active = list->head;
}

void list_last(tThreeACList *list)
{
    list->active = list->tail;
}

void list_next(tThreeACList *list)
{
    if (list->active != NULL)
    {
        if (list->tail != list->active)
        {
            list->active = list->active->next;
        }
        else
        {
            list->active = NULL;
        }
    }
}
void list_previous(tThreeACList *list)
{
    if (list->active != NULL)
    {
        if (list->head != list->active)
        {
            list->active = list->active->prev;
        }
        else
        {
            list->active = NULL;
        }
    }
}

void list_setValue(tThreeACList *list, tOperationType opType, tOperand *arg1, tOperand *arg2,
                   tOperand *result)
{
    if (list->active != NULL)
    {
        list->active->opType = opType;
        list->active->arg1 = arg1;
        list->active->arg2 = arg2;
        list->active->result = result;
    }
}

void list_getValue(tThreeACList *list, tOperationType *opType, tOperand **arg1, tOperand **arg2,
                   tOperand **result)
{
    if (list->active != NULL)
    {
        *opType = list->active->opType;
        *arg1 = list->active->arg1;
        *arg2 = list->active->arg2;
        *result = list->active->result;
    }
}

bool list_isActive(tThreeACList *list)
{
    return (list->active != NULL);
}

void list_InsertFirst(tThreeACList *list, tOperationType opType, tOperand *result, tOperand *arg1,
                      tOperand *arg2)
{
    tInstructionNode *newNode = (tInstructionNode *)safeMalloc(sizeof(tInstructionNode));
    newNode->opType = opType;
    newNode->arg1 = arg1;
    newNode->arg2 = arg2;
    newNode->result = result;
    newNode->prev = NULL;
    list->head = newNode;
    list->tail = newNode;
    list->active = newNode;
    list->length++;
}

void list_InsertAfter(tThreeACList *list, tOperationType opType, tOperand *result, tOperand *arg1,
                      tOperand *arg2)
{
    if (list_isActive(list))
    {
        tInstructionNode *newNode = (tInstructionNode *)safeMalloc(sizeof(tInstructionNode));
        tInstructionNode *next = list->active->next;
        newNode->opType = opType;
        newNode->arg1 = arg1;
        newNode->arg2 = arg2;
        newNode->result = result;
        newNode->prev = list->active;

        if (list->active->next == NULL)
        {
            newNode->next = NULL;
            list->active->next = newNode;
            list->tail = newNode;
        }
        else
        {
            newNode->next = list->active->next;
            next->prev = newNode;
            list->active->next = newNode;
        }

        list->active = newNode;
        list->length++;
    }
}

void list_InsertBefore(tThreeACList *list, tOperationType opType, tOperand *result, tOperand *arg1,
                       tOperand *arg2)
{
    if (list_isActive(list))
    {
        tInstructionNode *newNode = (tInstructionNode *)safeMalloc(sizeof(tInstructionNode));
        tInstructionNode *prev = list->active->prev;
        newNode->opType = opType;
        newNode->arg1 = arg1;
        newNode->arg2 = arg2;
        newNode->result = result;
        newNode->next = list->active;
        if (list->head == NULL)
        {
            list->head = list->tail = list->active = newNode;
        }
        else if (list->active->prev == NULL)
        {
            newNode->prev = NULL;
            list->active->prev = newNode;
            list->head = newNode;
        }
        else
        {
            newNode->prev = list->active->prev;
            prev->next = newNode;
            list->active->prev = newNode;
        }

        list->active = newNode;
        list->length++;
    }
}

void list_DeleteAfter(tThreeACList *list)
{
    if (list_isActive(list) && list->active != list->tail)
    {
        tInstructionNode *nextNode = list->active->next;

        if (nextNode == list->tail)
        {
            list->tail = list->active;
            list->active->next = NULL;
        }
        else
        {
            tInstructionNode *nextToNext = nextNode->next;
            list->active->next = nextToNext;
            nextToNext->prev = list->active;
        }
        list->length--;
        free(nextNode);
    }
}

void list_DeleteBefore(tThreeACList *list)
{
    if (list_isActive(list) && list->active != list->head)
    {
        tInstructionNode *prevNode = list->active->prev;

        if (prevNode == list->head)
        {
            list->head = list->active;
            list->active->prev = NULL;
        }
        else
        {
            tInstructionNode *prevToPrev = prevNode->prev;
            list->active->prev = prevToPrev;
            prevToPrev->next = list->active;
        }
        list->length--;
        free(prevNode);
    }
}

void list_add_global_def(tThreeACList *list, tOperationType op, tOperand *result, tOperand *arg1,
                         tOperand *arg2)
{
    tInstructionNode *newNode = (tInstructionNode *)safeMalloc(sizeof(tInstructionNode));
    newNode->opType = op;
    newNode->result = result;
    newNode->arg1 = arg1;
    newNode->arg2 = arg2;
    newNode->next = NULL;
    newNode->prev = list->globalDefTail;

    if (list->globalDefTail)
    {
        list->globalDefTail->next = newNode;
    }
    else
    {
        list->globalDefHead = newNode;
    }
    list->globalDefTail = newNode;
}

const char *operation_to_string(tOperationType op)
{
    switch (op)
    {
        case OP_LABEL:
            return "LABEL";
        case OP_JUMP:
            return "JUMP";
        case OP_JUMPIFEQ:
            return "JUMPIFEQ";
        case OP_JUMPIFNEQ:
            return "JUMPIFNEQ";
        case OP_JUMPIFEQS:
            return "JUMPIFEQS";
        case OP_JUMPIFNEQS:
            return "JUMPIFNEQS";

        case OP_DEFVAR:
            return "DEFVAR";
        case OP_MOVE:
            return "MOVE";

        case OP_CREATEFRAME:
            return "CREATEFRAME";
        case OP_PUSHFRAME:
            return "PUSHFRAME";
        case OP_POPFRAME:
            return "POPFRAME";

        case OP_ADD:
            return "ADD";
        case OP_SUB:
            return "SUB";
        case OP_MUL:
            return "MUL";
        case OP_DIV:
            return "DIV";
        case OP_IDIV:
            return "IDIV";

        case OP_AND:
            return "AND";
        case OP_OR:
            return "OR";
        case OP_NOT:
            return "NOT";

        case OP_LT:
            return "LT";
        case OP_GT:
            return "GT";
        case OP_EQ:
            return "EQ";

        case OP_CONCAT:
            return "CONCAT";
        case OP_STRLEN:
            return "STRLEN";
        case OP_GETCHAR:
            return "GETCHAR";
        case OP_SETCHAR:
            return "SETCHAR";

        case OP_PUSHS:
            return "PUSHS";
        case OP_POPS:
            return "POPS";

        case OP_ADDS:
            return "ADDS";
        case OP_SUBS:
            return "SUBS";
        case OP_MULS:
            return "MULS";
        case OP_DIVS:
            return "DIVS";
        case OP_IDIVS:
            return "IDIVS";

        case OP_ANDS:
            return "ANDS";
        case OP_ORS:
            return "ORS";
        case OP_NOTS:
            return "NOTS";

        case OP_LTS:
            return "LTS";
        case OP_GTS:
            return "GTS";
        case OP_EQS:
            return "EQS";

        case OP_CALL:
            return "CALL";
        case OP_RETURN:
            return "RETURN";

        case OP_INT2FLOAT:
            return "INT2FLOAT";
        case OP_FLOAT2INT:
            return "FLOAT2INT";
        case OP_FLOAT2STR:
            return "FLOAT2STR";
        case OP_INT2CHAR:
            return "INT2CHAR";
        case OP_STRI2INT:
            return "STRI2INT";
        case OP_INT2STR:
            return "INT2STR";

        case OP_INT2FLOATS:
            return "INT2FLOATS";
        case OP_FLOAT2INTS:
            return "FLOAT2INTS";
        case OP_INT2CHARS:
            return "INT2CHARS";
        case OP_STRI2INTS:
            return "STRI2INTS";
        case OP_FLOAT2STRS:
            return "FLOAT2STRS";
        case OP_INT2STRS:
            return "INT2STRS";

        case OP_TYPE:
            return "TYPE";
        case OP_ISINT:
            return "ISINT";
        case OP_TYPES:
            return "TYPES";
        case OP_ISINTS:
            return "ISINTS";

        case OP_READ:
            return "READ";
        case OP_WRITE:
            return "WRITE";

        case OP_EXIT:
            return "EXIT";
        case OP_COMMENT:
            return "#";

        case NO_OP:
            return "NO_OP";

        default:
            return "UNKNOWN_OP";
    }
}

void list_print(tThreeACList *list)
{
    list_first(list);
    printf(".IFJcode25\n");

    tInstructionNode *current_global = list->globalDefHead;
    while (current_global != NULL)
    {
        printf("DEFVAR %s\n", operand_to_string(current_global->result));
        printf("MOVE %s %s\n", operand_to_string(current_global->result), "nil@nil");
        current_global = current_global->next;
    }

    bool indent = false;
    while (list_isActive(list))
    {
        tOperationType opType;
        tOperand *arg1;
        tOperand *arg2;
        tOperand *result;

        list_getValue(list, &opType, &arg1, &arg2, &result);

        if (opType == NO_OP)
        {
            printf("\n");
            list_next(list);
            continue;
        }

        // if (indent) {
        //     printf("    ");
        // }

        printf("%s %s %s %s\n", operation_to_string(opType), operand_to_string(result),
               operand_to_string(arg1), operand_to_string(arg2));

        if (opType == OP_LABEL)
        {
            indent = true;
        }
        else if (opType == OP_POPFRAME)
        {
            indent = false;
        }

        list_next(list);
    }
}

const char *operand_to_string(const tOperand *tOperand)
{
    if (tOperand == NULL)
        return "";

    switch (tOperand->type)
    {
        case OPP_TYPE:
            return tOperand->value.typeName;
        case OPP_COMMENT_TEXT:
            return tOperand->value.strval;
        case OPP_GLOBAL:
        {
            char *buf = safeMalloc(strlen(tOperand->value.varname) + 4);
            sprintf(buf, "GF@%s", tOperand->value.varname);
            return buf;
        }
        case OPP_TF_VAR:
        {
            char *buf = safeMalloc(strlen(tOperand->value.varname) + 4);
            sprintf(buf, "TF@%s", tOperand->value.varname);
            return buf;
        }
        case OPP_VAR:
        {
            char *buf = safeMalloc(strlen(tOperand->value.varname) + 4);
            sprintf(buf, "LF@%s", tOperand->value.varname);
            return buf;
        }
        case OPP_TEMP:
        {
            char *buf = safeMalloc(strlen(tOperand->value.varname) + 4);
            sprintf(buf, "LF@%s", tOperand->value.varname);
            return buf;
        }
        case OPP_CONST_INT:
        {
            int len = snprintf(NULL, 0, "%d", tOperand->value.intval);
            char *str = safeMalloc(len + 5);
            sprintf(str, "int@%d", tOperand->value.intval);
            return str;
        }
        case OPP_CONST_FLOAT:
        {
            int len = snprintf(NULL, 0, "%a", tOperand->value.floatval);
            char *str = safeMalloc(len + 7);
            sprintf(str, "float@%a", tOperand->value.floatval);
            return str;
        }
        case OPP_CONST_STRING:
        {
            const char *s = tOperand->value.strval;
            size_t sLen = strlen(s);
            char *escapedStr = safeMalloc(sLen * 4 + 1);
            char *p = escapedStr;
            for (size_t i = 0; i < sLen; i++)
            {
                unsigned char c = s[i];
                if (c <= 32 || c == '#' || c == '\\')
                {
                    sprintf(p, "\\%03d", c);
                    p += 4;
                }
                else
                {
                    *p++ = c;
                }
            }
            *p = '\0';

            int finalLen = snprintf(NULL, 0, "string@%s", escapedStr);
            char *str = safeMalloc(finalLen + 1);
            sprintf(str, "string@%s", escapedStr);

            free(escapedStr);
            return str;
        }
        case OPP_CONST_BOOL:
        {
            int boolLen = tOperand->value.boolval ? 5 : 6;
            char *boolStr = safeMalloc(boolLen);
            sprintf(boolStr, "bool@%s", tOperand->value.boolval ? "true" : "false");
            return boolStr;
        }
        case OPP_CONST_NIL:
            return "nil@nil";
        case OPP_LABEL:
            return tOperand->value.label;
        default:
            return "UNKNOWN_OPERAND";
    }
}

char *threeAC_create_temp(tThreeACList *list)
{
    int len = snprintf(NULL, 0, "t%d", list->tempCounter);
    char *name = safeMalloc(len + 1);
    sprintf(name, "t%d", list->tempCounter++);
    return name;
}

char *threeAC_create_label(tThreeACList *list)
{
    int len = snprintf(NULL, 0, "%%L%d", list->loopCounter);
    char *name = safeMalloc(len + 1);
    sprintf(name, "%%L%d", list->loopCounter++);
    return name;
}

char *threeAC_get_current_label(tThreeACList *list)
{
    if (list->loopCounter == 0)
        return NULL;

    int curr = list->loopCounter - 1;
    int len = snprintf(NULL, 0, "%%L%d", curr);
    char *label = safeMalloc(len + 1);
    sprintf(label, "%%L%d", curr);
    return label;
}

tOperand *create_operand_from_constant_int(int value)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_CONST_INT;
    op->value.intval = value;
    return op;
}

tOperand *create_operand_from_constant_float(double value)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_CONST_FLOAT;
    op->value.floatval = value;
    return op;
}

tOperand *create_operand_from_constant_string(const char *value)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_CONST_STRING;
    op->value.strval = safeMalloc(strlen(value) + 1);
    strcpy(op->value.strval, value);
    return op;
}

tOperand *create_operand_from_constant_bool(bool value)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_CONST_BOOL;
    op->value.boolval = value;
    return op;
}

tOperand *create_operand_from_label(const char *label)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_LABEL;
    op->value.label = safeMalloc(strlen(label) + 1);
    strcpy(op->value.label, label);
    return op;
}

tOperand *create_operand_from_variable(const char *varname, bool isGlobal)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = isGlobal ? OPP_GLOBAL : OPP_VAR;
    op->value.varname = safeMalloc(strlen(varname) + 1);
    strcpy(op->value.varname, varname);
    return op;
}

tOperand *create_operand_from_tf_variable(const char *varname)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_TF_VAR;
    op->value.varname = safeMalloc(strlen(varname) + 1);
    strcpy(op->value.varname, varname);
    return op;
}

tOperand *create_operand_from_type(const char *typeName)
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_TYPE;
    op->value.typeName = safeMalloc(strlen(typeName) + 1);
    strcpy(op->value.typeName, typeName);
    return op;
}

tOperand *create_operand_from_constant_nil()
{
    tOperand *op = safeMalloc(sizeof(tOperand));
    op->type = OPP_CONST_NIL;
    return op;
}

void emit_comment(const char *text, tThreeACList *list)
{
    tOperand *commentOp = safeMalloc(sizeof(tOperand));
    commentOp->type = OPP_COMMENT_TEXT;
    commentOp->value.strval = safeMalloc(strlen(text) + 1);
    strcpy(commentOp->value.strval, text);
    emit(OP_COMMENT, commentOp, NULL, NULL, list);
}

void emit(tOperationType op, tOperand *result, tOperand *arg1, tOperand *arg2, tThreeACList *list)
{
    if (list_isActive(list))
    {
        list_InsertAfter(list, op, result, arg1, arg2);
    }
    else if (list->length == 0)
    {
        list_InsertFirst(list, op, result, arg1, arg2);
    }
    else
    {
        list_last(list);
        list_InsertAfter(list, op, result, arg1, arg2);
    }
}
