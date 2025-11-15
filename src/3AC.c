
#include "3AC.h"
#include "error.h"
#include <stdlib.h>
#include <stdbool.h>
#include "helper.h"




void list_init(ThreeACList *list)
{ 
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
    list->active = list->head;
    list->temp_counter = 0;
    list->expression_result = NULL; 
    list->return_used = false;
    list->while_used = false;
    list->if_used = false;
}

void list_dispose( ThreeACList *list ) 
{
	
	InstructionNode *current = list->head;
    InstructionNode *next;
   

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
}

void list_first( ThreeACList *list ) 
{
    list->active = list->head;
}

void list_last( ThreeACList *list ) 
{
    list->active = list->tail;
}

void list_next( ThreeACList *list ) 
{
    if (list->active != NULL) 
    {
        if ( list->tail != list->active ) 
        { 
            list->active = list->active->next;
        } else {
            list->active = NULL;
        }
    }
}
void list_previous( ThreeACList *list ) 
{
    if (list->active != NULL)
     {
        if ( list->head != list->active ) 
        { 
            list->active = list->active->prev;
        } else {
            list->active = NULL;
        }
    }
}

void list_setValue( ThreeACList *list, OperationType opType, char *arg1, char *arg2, char *result ) 
{
  if (list->active != NULL) 
  {
      list->active->opType = opType;
      list->active->arg1 = arg1;
      list->active->arg2 = arg2;
      list->active->result = result;
  }
}

void list_getValue( ThreeACList *list, OperationType *opType, char **arg1, char **arg2, char **result ) 
{
    if (list->active != NULL) 
    {
        *opType = list->active->opType;
        *arg1 = list->active->arg1;
        *arg2 = list->active->arg2;
        *result = list->active->result;
    }
}

bool list_isActive( ThreeACList *list )
{
    return (list->active != NULL);
}

void list_InsertFirst( ThreeACList *list, OperationType opType, char *arg1, char *arg2, char *result ) 
{
    InstructionNode *newNode = (InstructionNode *) safeMalloc (sizeof(InstructionNode));
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


void list_InsertAfter( ThreeACList *list,  OperationType opType, char *arg1, char *arg2, char *result ) 
{
    if (list_isActive(list))
    { 
        InstructionNode *newNode = (InstructionNode *) safeMalloc (sizeof(InstructionNode));
        InstructionNode *next = list->active->next;
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

void list_InsertBefore( ThreeACList *list,  OperationType opType, char *arg1, char *arg2, char *result ) 
{
    if (list_isActive(list))
    { 
        InstructionNode *newNode = (InstructionNode *) safeMalloc (sizeof(InstructionNode));
        InstructionNode *prev = list->active->prev;
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

void list_DeleteAfter( ThreeACList *list ) 
{
    if (list_isActive(list) && list->active != list->tail) 
    {
       InstructionNode *nextNode = list->active->next;

       if (nextNode == list->tail) 
       {
           list->tail = list->active;
           list->active->next = NULL;
       } 
       else 
       {
           InstructionNode *nextToNext = nextNode->next;
           list->active->next = nextToNext;
           nextToNext->prev = list->active;
       }
       list->length--;
       free(nextNode);
    }
}

void list_DeleteBefore( ThreeACList *list ) 
{
    if (list_isActive(list) && list->active != list->head) 
    {
       InstructionNode *prevNode = list->active->prev;

       if (prevNode == list->head) 
       {
           list->head = list->active;
           list->active->prev = NULL;
       } 
       else 
       {
           InstructionNode *prevToPrev = prevNode->prev;
           list->active->prev = prevToPrev;
           prevToPrev->next = list->active;
       }
       list->length--;
       free(prevNode);
    }
}



const char *operation_to_string(OperationType op) 
{
    switch (op) 
    {
        case OPP_ADD: return "ADD";
        case OPP_SUB: return "SUB";
        case OPP_MUL: return "MUL";
        case OPP_DIV: return "DIV";
        case OPP_ASSIGN: return "ASSIGN";
        case OP_LABEL: return "LABEL";
        case OP_JUMP: return "JUMP";
        case OP_CALL: return "CALL";
        case OP_RETURN: return "RETURN";
        case OP_PARAM: return "PARAM";
        case OPP_GT : return "GT";
        case OPP_GTE : return "GTE";
        case OPP_LT : return "LT";
        case OPP_LTE : return "LTE";
        case OPP_EQ : return "EQ";
        case OPP_NEQ : return "NEQ";
        case NO_OP : return "NO_OP";
        case while_start : return "WHILE_START";
        case while_end : return "WHILE_END";
        case OP_JUMP_IF_FALSE : return "JUMP_IF_FALSE";
        case if_start : return "IF_START";
        case if_end : return "IF_END";
        case if_else : return "IF_ELSE";
        case OP_DEFVAR : return "DEFVAR";
        
        default: return "UNKNOWN_OP";
    }
}

void list_print(ThreeACList *list) { 
    list_first(list);
    printf("----- 3AC CODE -----\n");
    while (list_isActive(list)) { 
        OperationType opType;
        char *arg1;
        char *arg2;
        char *result;
    
        list_getValue(list, &opType, &arg1, &arg2, &result);
        if (opType == NO_OP) { 
            printf("\n");
            list_next(list);
            continue;
        } else if (opType == OP_PARAM) { 
            printf("%s %s\n", operation_to_string(opType), result ? result : "");
            list_next(list);
            continue;
        } else if (opType == if_start) { 
            printf("%s %s\n", operation_to_string(opType), result ? result : "");
            list_next(list);
            continue;
        }

        printf("%s %s %s %s\n", operation_to_string(opType), arg1 ? arg1 : "", arg2 || arg2 != NULL ? arg2 : "", result ? result : "");

        list_next(list);
    }
    printf("----- -------- ------\n");
}



char *threeAC_create_temp(ThreeACList *list)
{
    int len = snprintf(NULL, 0, "t%d", list->temp_counter);
    char *name = safeMalloc(len + 1);
    sprintf(name, "t%d", list->temp_counter++);
    return name;
}


char *threeAC_create_label(ThreeACList *list)
{
    int len = snprintf(NULL, 0, "t%d", list->loop_counter);
    char *name = safeMalloc(len + 1);
    sprintf(name, "l%d", list->loop_counter++);
    return name;
}

char *threeAC_get_current_label(ThreeACList *list) {
    if (list->loop_counter == 0) return NULL;  

    int curr = list->loop_counter - 1;
    int len = snprintf(NULL, 0, "l%d", curr);
    char *label = safeMalloc(len + 1);
    sprintf(label, "l%d", curr);
    return label;
}


void emit(OperationType op, const char *arg1, const char *arg2, const char *result, ThreeACList *list) {
    if (list_isActive(list)) 
    { 
        list_InsertAfter(list, op, (char *)arg1, (char *)arg2, (char *)result);
    }
    else if (list->length == 0){ 
        list_InsertFirst(list, op, (char *)arg1, (char *)arg2, (char *)result);
    } else { 
        list_last(list);
        list_InsertAfter(list, op, (char *)arg1, (char *)arg2, (char *)result);
    }
}

