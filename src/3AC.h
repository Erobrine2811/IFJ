#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdbool.h>

typedef enum { 
  OP_LABEL,
  OP_JUMP,
  OP_JUMPIFEQ,
  OP_JUMPIFNEQ,

  OP_DEFVAR,
  OP_MOVE,

  OP_CREATEFRAME,
  OP_PUSHFRAME,
  OP_POPFRAME,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,

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

  OP_TYPE,
  OP_ISINT,

  OP_READ,
  OP_WRITE,

  OP_EXIT,
  OP_COMMENT,
  NO_OP
} OperationType;

typedef enum {
    OPP_NONE,
    OPP_COMMENT_TEXT,
    OPP_VAR,
    OPP_TEMP,
    OPP_GLOBAL,
    OPP_CONST_INT,
    OPP_CONST_FLOAT,
    OPP_CONST_STRING,
    OPP_CONST_BOOL,
    OPP_CONST_NIL,
    OPP_LABEL
} OperandType;


typedef struct {
    OperandType type;
    union {
        int intval;
        double floatval;
        char *strval;
        bool boolval;
        char *varname;
        char *label; 
    } value;
} Operand;

typedef struct InstructionNode{
    OperationType opType;
    Operand *result;
    Operand *arg1;
    Operand *arg2;
    
    struct InstructionNode *next;
    struct InstructionNode *prev;
} InstructionNode;



typedef struct  {
    InstructionNode *head;
    InstructionNode *active;
    InstructionNode *tail;
    size_t length;
    int temp_counter;
    int loop_counter;
    char *expression_result;
    bool return_used;
    bool while_used;
    bool if_used;
} ThreeACList;



void list_init(ThreeACList *list);
void list_dispose(ThreeACList *list);
void list_first(ThreeACList *list);
void list_last(ThreeACList *list);
void list_next(ThreeACList *list);
void list_previous(ThreeACList *list);
bool list_isActive(ThreeACList *list);
void list_setValue(ThreeACList *list, OperationType opType, Operand *arg1, Operand *arg2, Operand *result);
void list_InsertAfter( ThreeACList *list, OperationType opType, Operand *result, Operand *arg1, Operand *arg2 );
void list_InsertBefore( ThreeACList *list, OperationType opType, Operand *result, Operand *arg1, Operand *arg2 );
void list_GetValue( ThreeACList *list, OperationType *opType, Operand **arg1, Operand **arg2, Operand **result );
void list_DeleteAfter( ThreeACList *list );
void list_DeleteBefore( ThreeACList *list );
void list_InsertFirst( ThreeACList *list, OperationType opType, Operand *result, Operand *arg1, Operand *arg2 );

void emit(OperationType op, Operand *result, Operand *arg1,  Operand *arg2, ThreeACList *list);
void emit_comment(const char *text, ThreeACList *list);


char *threeAC_create_temp(ThreeACList *list);
char *threeAC_create_label(ThreeACList *list);
char *threeAC_get_current_label(ThreeACList *list);


extern ThreeACList threeACcode;

void list_print(ThreeACList *list);
const char *operation_to_string(OperationType op);
const char *operand_to_string(const Operand *operand);

#endif
