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
    OPP_WRITE,
    OP_LABEL,
    OP_JUMP,
    OP_CALL,
    OP_RETURN,
    OP_PARAM,
    NO_OP
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

extern ThreeACList threeACcode;

void list_print(ThreeACList *list);
const char *operation_to_string(OperationType op);

#endif