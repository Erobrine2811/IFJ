
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

void list_setValue( ThreeACList *list, OperationType opType, Operand *arg1, Operand *arg2, Operand *result ) 
{
  if (list->active != NULL) 
  {
      list->active->opType = opType;
      list->active->arg1 = arg1;
      list->active->arg2 = arg2;
      list->active->result = result;
  }
}

void list_getValue( ThreeACList *list, OperationType *opType, Operand **arg1, Operand **arg2, Operand **result ) 
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

void list_InsertFirst( ThreeACList *list,  OperationType opType, Operand* result, Operand *arg1, Operand *arg2 ) 
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


void list_InsertAfter( ThreeACList *list,  OperationType opType, Operand* result, Operand *arg1, Operand *arg2 ) 
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

void list_InsertBefore( ThreeACList *list,  OperationType opType, Operand* result, Operand *arg1, Operand *arg2 ) 
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
   case OP_LABEL:        return "LABEL";
    case OP_JUMP:         return "JUMP";
    case OP_JUMPIFEQ:     return "JUMPIFEQ";
    case OP_JUMPIFNEQ:    return "JUMPIFNEQ";

    case OP_DEFVAR:       return "DEFVAR";
    case OP_MOVE:         return "MOVE";

    case OP_CREATEFRAME:  return "CREATEFRAME";
    case OP_PUSHFRAME:    return "PUSHFRAME";
    case OP_POPFRAME:     return "POPFRAME";

    case OP_ADD:          return "ADD";
    case OP_SUB:          return "SUB";
    case OP_MUL:          return "MUL";
    case OP_DIV:          return "DIV";

    case OP_AND:          return "AND";
    case OP_OR:           return "OR";
    case OP_NOT:          return "NOT";

    case OP_LT:           return "LT";
    case OP_GT:           return "GT";
    case OP_EQ:           return "EQ";

    case OP_CONCAT:       return "CONCAT";
    case OP_STRLEN:       return "STRLEN";
    case OP_GETCHAR:      return "GETCHAR";
    case OP_SETCHAR:      return "SETCHAR";

    case OP_PUSHS:        return "PUSHS";
    case OP_POPS:         return "POPS";

    case OP_ADDS:         return "ADDS";
    case OP_SUBS:         return "SUBS";
    case OP_MULS:         return "MULS";
    case OP_DIVS:         return "DIVS";

    case OP_ANDS:         return "ANDS";
    case OP_ORS:          return "ORS";
    case OP_NOTS:         return "NOTS";

    case OP_LTS:          return "LTS";
    case OP_GTS:          return "GTS";
    case OP_EQS:          return "EQS";

    case OP_CALL:         return "CALL";
    case OP_RETURN:       return "RETURN";

    case OP_INT2FLOAT:    return "INT2FLOAT";
    case OP_FLOAT2INT:    return "FLOAT2INT";
    case OP_FLOAT2STR:    return "FLOAT2STR";

    case OP_TYPE:         return "TYPE";
    case OP_ISINT:        return "ISINT";

    case OP_READ:         return "READ";
    case OP_WRITE:        return "WRITE";

    case OP_EXIT:         return "EXIT";
    case OP_COMMENT:      return "#";

    case NO_OP:           return "NO_OP";
        
        default: return "UNKNOWN_OP";
    }
}

void list_print(ThreeACList *list) { 
    list_first(list);
    printf("----- 3AC CODE -----\n");
    while (list_isActive(list)) { 
        OperationType opType;
        Operand *arg1;
        Operand *arg2;
        Operand *result;
  
        list_getValue(list, &opType, &arg1, &arg2, &result);

        if (opType == NO_OP){
            printf("\n");
            list_next(list);
            continue;
        }

        printf("%s %s %s %s\n", operation_to_string(opType), operand_to_string(result), operand_to_string(arg1), operand_to_string(arg2));
        list_next(list);
    }
    printf("----- -------- ------\n");
}

const char *operand_to_string(const Operand *operand) {
    if (operand == NULL) return "";

    switch (operand->type) {
        case OPP_COMMENT_TEXT:
            return operand->value.strval;
        case OPP_GLOBAL: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "GF@%s", operand->value.varname);
            return buf;
        }
        case OPP_VAR: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "LF@%s", operand->value.varname);
            return buf;
        }
        case OPP_TEMP: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "TF@%s", operand->value.varname);
            return buf;
        }
        case OPP_CONST_INT: {
            int len = snprintf(NULL, 0, "%d", operand->value.intval);
            char *str = safeMalloc(len + 5);
            sprintf(str, "int@%d", operand->value.intval);
            return str;
        }
        case OPP_CONST_FLOAT: {
            int len = snprintf(NULL, 0, "%f", operand->value.floatval);
            char *str = safeMalloc(len + 7);
            sprintf(str, "float@%f", operand->value.floatval);
            return str;
        }
        case OPP_CONST_STRING: {
            int len = snprintf(NULL, 0, "%s", operand->value.strval);
            char *str = safeMalloc(len + 8);
            sprintf(str, "string@%s", operand->value.strval);
            return str;
      }
        case OPP_CONST_BOOL: {
            int bool_len = operand->value.boolval ? 5 : 6;
            char *bool_str = safeMalloc(bool_len);
            sprintf(bool_str, "bool@%s", operand->value.boolval ? "true" : "false");
            return bool_str;
      }
        case OPP_CONST_NIL:
            return "nil@nil";
        case OPP_LABEL:
            return operand->value.label;
        default:
            return "UNKNOWN_OPERAND";
    }
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


void emit_comment(const char *text, ThreeACList *list) {
    Operand *commentOp = safeMalloc(sizeof(Operand));
    commentOp->type = OPP_COMMENT_TEXT;
    commentOp->value.strval = safeMalloc(strlen(text) + 1);
    strcpy(commentOp->value.strval, text);
    emit(OP_COMMENT, commentOp, NULL, NULL, list);
}

void emit(OperationType op, Operand *result, Operand *arg1,  Operand *arg2, ThreeACList *list) {
    if (list_isActive(list)) 
    { 
        list_InsertAfter(list, op, result, arg1, arg2);
    }
    else if (list->length == 0){ 
        list_InsertFirst(list, op, result, arg1, arg2);
    } else { 
        list_last(list);
        list_InsertAfter(list, op, result, arg1, arg2);
    }
}

