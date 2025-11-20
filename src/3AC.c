
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
    list->var_counter = 0;
    list->expression_result = NULL; 
    list->return_used = false;
    list->while_used = false;
    list->if_used = false;
    list->global_def_head = NULL;
    list->global_def_tail = NULL;
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

    current = list->global_def_head;
	while (current != NULL) 
    { 
        next = current->next;
        free(current);
		current = next;
	}
    list->global_def_head = NULL;
    list->global_def_tail = NULL;
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

void list_add_global_def(ThreeACList *list, OperationType op, Operand *result, Operand *arg1, Operand *arg2) {
    InstructionNode *newNode = (InstructionNode *) safeMalloc (sizeof(InstructionNode));
    newNode->opType = op;
    newNode->result = result;
    newNode->arg1 = arg1;
    newNode->arg2 = arg2;
    newNode->next = NULL;
    newNode->prev = list->global_def_tail;

    if (list->global_def_tail) {
        list->global_def_tail->next = newNode;
    } else {
        list->global_def_head = newNode;
    }
    list->global_def_tail = newNode;
}

const char *operation_to_string(OperationType op) 
{
    switch (op) 
    {
   case OP_LABEL:        return "LABEL";
    case OP_JUMP:         return "JUMP";
    case OP_JUMPIFEQ:     return "JUMPIFEQ";
    case OP_JUMPIFNEQ:    return "JUMPIFNEQ";
    case OP_JUMPIFEQS:    return "JUMPIFEQS";
    case OP_JUMPIFNEQS:   return "JUMPIFNEQS";

    case OP_DEFVAR:       return "DEFVAR";
    case OP_MOVE:         return "MOVE";

    case OP_CREATEFRAME:  return "CREATEFRAME";
    case OP_PUSHFRAME:    return "PUSHFRAME";
    case OP_POPFRAME:     return "POPFRAME";

    case OP_ADD:          return "ADD";
    case OP_SUB:          return "SUB";
    case OP_MUL:          return "MUL";
    case OP_DIV:          return "DIV";
    case OP_IDIV:         return "IDIV";

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
    case OP_IDIVS:        return "IDIVS";

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
    case OP_INT2CHAR:     return "INT2CHAR";
    case OP_STRI2INT:     return "STRI2INT";
    case OP_INT2STR:      return "INT2STR";

    case OP_INT2FLOATS:   return "INT2FLOATS";
    case OP_FLOAT2INTS:   return "FLOAT2INTS";
    case OP_INT2CHARS:    return "INT2CHARS";
    case OP_STRI2INTS:    return "STRI2INTS";
    case OP_FLOAT2STRS:   return "FLOAT2STRS";
    case OP_INT2STRS:     return "INT2STRS";

    case OP_TYPE:         return "TYPE";
    case OP_ISINT:        return "ISINT";
    case OP_TYPES:         return "TYPES";
    case OP_ISINTS:        return "ISINTS";

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
    printf(".IFJcode25\n");

    InstructionNode *current_global = list->global_def_head;
    while (current_global != NULL) {
        printf("DEFVAR %s\n", operand_to_string(current_global->result));
        printf("MOVE %s %s\n", operand_to_string(current_global->result), "nil@nil");
        current_global = current_global->next;
    }

    bool indent = false;
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

        // if (indent) {
        //     printf("    ");
        // }

        printf("%s %s %s %s\n", operation_to_string(opType), operand_to_string(result), operand_to_string(arg1), operand_to_string(arg2));

        if (opType == OP_LABEL) {
            indent = true;
        } else if (opType == OP_POPFRAME) {
            indent = false;
        }

        list_next(list);
    }
}

const char *operand_to_string(const Operand *operand) {
    if (operand == NULL) return "";

    switch (operand->type) {
        case OPP_TYPE:
            return operand->value.type_name;
        case OPP_COMMENT_TEXT:
            return operand->value.strval;
        case OPP_GLOBAL: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "GF@%s", operand->value.varname);
            return buf;
        }
        case OPP_TF_VAR: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "TF@%s", operand->value.varname);
            return buf;
}
        case OPP_VAR: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "LF@%s", operand->value.varname);
            return buf;
        }
        case OPP_TEMP: {
            char* buf = safeMalloc(strlen(operand->value.varname) + 4);
            sprintf(buf, "LF@%s", operand->value.varname);
            return buf;
        }
        case OPP_CONST_INT: {
            int len = snprintf(NULL, 0, "%d", operand->value.intval);
            char *str = safeMalloc(len + 5);
            sprintf(str, "int@%d", operand->value.intval);
            return str;
        }
        case OPP_CONST_FLOAT: {
            int len = snprintf(NULL, 0, "%a", operand->value.floatval);
            char *str = safeMalloc(len + 7);
            sprintf(str, "float@%a", operand->value.floatval);
            return str;
        }
        case OPP_CONST_STRING: {
            const char* s = operand->value.strval;
            size_t s_len = strlen(s);
            char* escaped_str = safeMalloc(s_len * 4 + 1); 
            char* p = escaped_str;
            for (size_t i = 0; i < s_len; i++) {
                unsigned char c = s[i];
                if (c <= 32 || c == '#' || c == '\\') {
                    sprintf(p, "\\%03d", c);
                    p += 4;
                } else {
                    *p++ = c;
                }
            }
            *p = '\0';

            int final_len = snprintf(NULL, 0, "string@%s", escaped_str);
            char *str = safeMalloc(final_len + 1);
            sprintf(str, "string@%s", escaped_str);
            
            free(escaped_str);
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
    int len = snprintf(NULL, 0, "%%L%d", list->loop_counter);
    char *name = safeMalloc(len + 1);
    sprintf(name, "%%L%d", list->loop_counter++);
    return name;
}

char *threeAC_get_current_label(ThreeACList *list) {
    if (list->loop_counter == 0) return NULL;  

    int curr = list->loop_counter - 1;
    int len = snprintf(NULL, 0, "%%L%d", curr);
    char *label = safeMalloc(len + 1);
    sprintf(label, "%%L%d", curr);
    return label;
}

Operand* create_operand_from_constant_int(int value) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_CONST_INT;
    op->value.intval = value;
    return op;
}

Operand* create_operand_from_constant_float(double value) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_CONST_FLOAT;
    op->value.floatval = value;
    return op;
}

Operand* create_operand_from_constant_string(const char *value) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_CONST_STRING;
    op->value.strval = safeMalloc(strlen(value) + 1);
    strcpy(op->value.strval, value);
    return op;
}

Operand* create_operand_from_constant_bool(bool value) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_CONST_BOOL;
    op->value.boolval = value;
    return op;
}

Operand* create_operand_from_label(const char *label) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_LABEL;
    op->value.label = safeMalloc(strlen(label) + 1);
    strcpy(op->value.label, label);
    return op;
}

Operand* create_operand_from_variable(const char *varname, bool isGlobal) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = isGlobal ? OPP_GLOBAL : OPP_VAR;
    op->value.varname = safeMalloc(strlen(varname) + 1);
    strcpy(op->value.varname, varname);
    return op;
}

Operand* create_operand_from_tf_variable(const char *varname) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_TF_VAR;
    op->value.varname = safeMalloc(strlen(varname) + 1);
    strcpy(op->value.varname, varname);
    return op;
}

Operand* create_operand_from_type(const char *type_name) {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_TYPE;
    op->value.type_name = safeMalloc(strlen(type_name) + 1);
    strcpy(op->value.type_name, type_name);
    return op;
}


Operand* create_operand_from_constant_nil() {
    Operand *op = safeMalloc(sizeof(Operand));
    op->type = OPP_CONST_NIL;
    return op;
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

