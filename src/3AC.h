#ifndef IFJ_GENERATOR_H
#define IFJ_GENERATOR_H

#include <stdbool.h>
#include <stdio.h>

typedef enum
{
    OP_LABEL,
    OP_JUMP,
    OP_JUMPIFEQ,
    OP_JUMPIFNEQ,
    OP_JUMPIFEQS,
    OP_JUMPIFNEQS,

    OP_DEFVAR,
    OP_MOVE,

    OP_CREATEFRAME,
    OP_PUSHFRAME,
    OP_POPFRAME,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_IDIV,

    OP_AND,
    OP_OR,
    OP_NOT,

    OP_LT,
    OP_GT,
    OP_EQ,

    OP_CONCAT,
    OP_STRLEN,
    OP_GETCHAR,
    OP_SETCHAR,

    OP_PUSHS,
    OP_POPS,

    OP_ADDS,
    OP_SUBS,
    OP_MULS,
    OP_DIVS,
    OP_IDIVS,

    OP_ANDS,
    OP_ORS,
    OP_NOTS,

    OP_LTS,
    OP_GTS,
    OP_EQS,

    OP_CALL,
    OP_RETURN,

    OP_INT2FLOAT,
    OP_FLOAT2INT,
    OP_FLOAT2STR,
    OP_INT2CHAR,
    OP_STRI2INT,
    OP_INT2STR,

    OP_INT2FLOATS,
    OP_FLOAT2INTS,
    OP_INT2CHARS,
    OP_STRI2INTS,
    OP_FLOAT2STRS,
    OP_INT2STRS,

    OP_TYPE,
    OP_ISINT,

    OP_TYPES,
    OP_ISINTS,

    OP_READ,
    OP_WRITE,

    OP_EXIT,
    OP_COMMENT,
    NO_OP
} tOperationType;

typedef enum
{
    OPP_NONE,
    OPP_TYPE,
    OPP_COMMENT_TEXT,
    OPP_VAR,
    OPP_TF_VAR,
    OPP_TEMP,
    OPP_GLOBAL,
    OPP_CONST_INT,
    OPP_CONST_FLOAT,
    OPP_CONST_STRING,
    OPP_CONST_BOOL,
    OPP_CONST_NIL,
    OPP_LABEL
} tOperandType;

typedef struct
{
    tOperandType type;
    union
    {
        int intval;
        double floatval;
        char *strval;
        bool boolval;
        char *varname;
        char *label;
        char *typeName;
    } value;
} tOperand;

typedef struct InstructionNode
{
    tOperationType opType;
    tOperand *result;
    tOperand *arg1;
    tOperand *arg2;

    struct InstructionNode *next;
    struct InstructionNode *prev;
} tInstructionNode;

typedef struct
{
    tInstructionNode *head;
    tInstructionNode *active;
    tInstructionNode *tail;
    size_t length;
    int tempCounter;
    int loopCounter;
    int varCounter;
    char *expressionResult;
    bool returnUsed;
    bool whileUsed;
    bool ifUsed;
    tInstructionNode *globalDefHead;
    tInstructionNode *globalDefTail;
} tThreeACList;

void list_init(tThreeACList *list);
void list_dispose(tThreeACList *list);
void list_first(tThreeACList *list);
void list_last(tThreeACList *list);
void list_next(tThreeACList *list);
void list_previous(tThreeACList *list);
bool list_isActive(tThreeACList *list);
void list_setValue(tThreeACList *list, tOperationType opType, tOperand *arg1, tOperand *arg2,
                   tOperand *result);
void list_InsertAfter(tThreeACList *list, tOperationType opType, tOperand *result, tOperand *arg1,
                      tOperand *arg2);
void list_InsertBefore(tThreeACList *list, tOperationType opType, tOperand *result, tOperand *arg1,
                       tOperand *arg2);
void list_GetValue(tThreeACList *list, tOperationType *opType, tOperand **arg1, tOperand **arg2,
                   tOperand **result);
void list_DeleteAfter(tThreeACList *list);
void list_DeleteBefore(tThreeACList *list);
void list_InsertFirst(tThreeACList *list, tOperationType opType, tOperand *result, tOperand *arg1,
                      tOperand *arg2);
void list_add_global_def(tThreeACList *list, tOperationType op, tOperand *result, tOperand *arg1,
                         tOperand *arg2);

void emit(tOperationType op, tOperand *result, tOperand *arg1, tOperand *arg2, tThreeACList *list);
void emit_comment(const char *text, tThreeACList *list);

char *threeAC_create_temp(tThreeACList *list);
char *threeAC_create_label(tThreeACList *list);
char *threeAC_get_current_label(tThreeACList *list);

tOperand *create_operand_from_constant_string(const char *value);
tOperand *create_operand_from_constant_int(int value);
tOperand *create_operand_from_constant_float(double value);
tOperand *create_operand_from_constant_bool(bool value);
tOperand *create_operand_from_label(const char *label);
tOperand *create_operand_from_variable(const char *varname, bool isGlobal);
tOperand *create_operand_from_tf_variable(const char *varname);
tOperand *create_operand_from_constant_nil();
tOperand *create_operand_from_type(const char *typeName);

extern tThreeACList threeACcode;

void list_print(tThreeACList *list);
const char *operation_to_string(tOperationType op);
const char *operand_to_string(const tOperand *operand);

#endif // IFJ_GENERATOR_H
