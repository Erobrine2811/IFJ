#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdbool.h>

typedef enum { 
    OPP_ADD,
    OPP_SUB,
    OPP_MUL,
    OPP_DIV,
    OPP_ASSIGN,
    OP_LABEL,
    OP_DEFVAR,

    OP_JUMP,
    OP_JUMP_IF_FALSE,

    OP_CALL,
    OP_RETURN,
    OP_PARAM,
    NO_OP,

    OPP_LT,
    OPP_LTE,
    OPP_GT,
    OPP_GTE,
    OPP_EQ,
    OPP_NEQ,

    while_start,
    while_end,

    if_start,
    if_end,
    if_else
} OperationType;

typedef struct InstructionNode{
    OperationType opType;
    char *arg1;
    char *arg2;
    char *result;
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
void list_setValue(ThreeACList *list, OperationType opType, char *arg1, char *arg2, char *result);
void list_InsertAfter(ThreeACList *list, OperationType opType, char *arg1, char *arg2, char *result);
void list_InsertBefore(ThreeACList *list, OperationType opType, char *arg1, char *arg2, char *result);
void list_GetValue(ThreeACList *list, OperationType *opType, char **arg1, char **arg2, char **result);
void list_DeleteAfter(ThreeACList *list);
void list_DeleteBefore(ThreeACList *list);
void list_InsertFirst(ThreeACList *list, OperationType opType, char *arg1, char *arg2, char *result);

void emit(OperationType op, const char *arg1, const char *arg2, const char *result, ThreeACList *list);


char *threeAC_create_temp(ThreeACList *list);
char *threeAC_create_label(ThreeACList *list);
char *threeAC_get_current_label(ThreeACList *list);


extern ThreeACList threeACcode;

void list_print(ThreeACList *list);
const char *operation_to_string(OperationType op);

#endif