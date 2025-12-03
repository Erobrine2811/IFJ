#include <stdbool.h>
#include <stdio.h>

#include "3AC_patterns.h"
#include "error.h"
#include "expr_parser.h"
#include "parser.h"
#include "scanner.h"
#include "symtable.h"

void generate_program_entrypoint(tThreeACList *list)
{
    emit(NO_OP, NULL, NULL, NULL, list);
    tOperand *startJumpLabel = create_operand_from_label("%start");
    emit(OP_JUMP, startJumpLabel, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);

    emit_comment("####################", list);
    emit_comment("Program entry point", list);
    emit_comment("####################", list);

    emit(OP_LABEL, startJumpLabel, NULL, NULL, list);
    tOperand *mainLabel = create_operand_from_label("main$0%func");
    emit(OP_CREATEFRAME, NULL, NULL, NULL, list);
    emit(OP_PUSHFRAME, NULL, NULL, NULL, list);
    emit(OP_CALL, mainLabel, NULL, NULL, list);
    emit(OP_POPFRAME, NULL, NULL, NULL, list);

    tOperand *exitValue = create_operand_from_constant_int(0);
    emit(OP_EXIT, exitValue, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);
    emit(NO_OP, NULL, NULL, NULL, list);
}

void generate_return(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isOneLine)
{
    if (!isOneLine)
    {
        get_next_token(file, currentToken);
    }

    parse_expression(file, currentToken, stack);
    tOperand *retvalVar = create_operand_from_variable("%retval", false);
    emit(OP_POPS, retvalVar, NULL, NULL, &threeACcode);
    emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
}

tDataType generate_ifj_write(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.write call", &threeACcode);

    tOperand *writeArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, writeArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, writeArg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, writeArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    tOperand *afterChecking = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, afterChecking, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, writeArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, afterChecking, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, writeArg, writeArg, NULL, &threeACcode);
    emit(OP_LABEL, afterChecking, NULL, NULL, &threeACcode);

    emit(OP_WRITE, writeArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_nil(), NULL, NULL, &threeACcode);

    return TYPE_NULL;
}

tDataType generate_ifj_read_str(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.read_str call", &threeACcode);

    tOperand *resultVar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, resultVar, NULL, NULL, &threeACcode);
    emit(OP_READ, resultVar, create_operand_from_type("string"), NULL, &threeACcode);
    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

tDataType generate_ifj_read_num(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.read_num call", &threeACcode);

    tOperand *resultVar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultVar, NULL, NULL, &threeACcode);
    emit(OP_READ, resultVar, create_operand_from_type("float"), NULL, &threeACcode);

    tOperand *resultIsNotIntOrNull = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, resultIsNotIntOrNull, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, resultIsNotIntOrNull, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, resultVar, resultVar, NULL, &threeACcode);

    emit(OP_LABEL, resultIsNotIntOrNull, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultVar, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_strcmp(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.strcmp call", &threeACcode);

    tOperand *s2Arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, s2Arg, NULL, NULL, &threeACcode);

    tOperand *s1Arg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_POPS, s1Arg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    tOperand *labelTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelContinueStrcmp = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_JUMP, labelContinueStrcmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueStrcmp, NULL, NULL, &threeACcode);

    tOperand *resultCmp = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultCmp, NULL, NULL, &threeACcode);

    tOperand *labelEqual = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelLess = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelGreater = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelEndCmp = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelEqual, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, s1Arg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, s2Arg, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelLess, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelGreater, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultCmp, create_operand_from_constant_int(1), NULL, &threeACcode);
    emit(OP_JUMP, labelEndCmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEqual, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultCmp, create_operand_from_constant_int(0), NULL, &threeACcode);
    emit(OP_JUMP, labelEndCmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelLess, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultCmp, create_operand_from_constant_int(-1), NULL, &threeACcode);
    emit(OP_JUMP, labelEndCmp, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEndCmp, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultCmp, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_ord(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.ord call", &threeACcode);

    tOperand *iArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, iArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, iArg, NULL, NULL, &threeACcode);

    tOperand *sArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, sArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, sArg, NULL, NULL, &threeACcode);

    tOperand *typeI = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeI, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeI, iArg, NULL, &threeACcode);

    tOperand *labelTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelContinueOrd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, sArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelContinueOrd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, iArg, iArg, NULL, &threeACcode);
    emit(OP_JUMP, labelContinueOrd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_JUMP, labelContinueOrd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelContinueOrd, NULL, NULL, &threeACcode);

    tOperand *lenS = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, lenS, NULL, NULL, &threeACcode);
    emit(OP_STRLEN, lenS, sArg, NULL, &threeACcode);

    tOperand *resultOrd = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultOrd, NULL, NULL, &threeACcode);

    tOperand *labelReturnZero = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelEndOrd = create_operand_from_label(threeAC_create_label(&threeACcode));

    // (lenS == 0)
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnZero, NULL, NULL, &threeACcode);

    // Check if i < 0
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnZero, NULL, NULL, &threeACcode);

    // Check if i >= lenS
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnZero, NULL, NULL, &threeACcode);

    emit(OP_STRI2INT, resultOrd, sArg, iArg, &threeACcode);
    emit(OP_JUMP, labelEndOrd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelReturnZero, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultOrd, create_operand_from_constant_int(0), NULL, &threeACcode);

    emit(OP_LABEL, labelEndOrd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultOrd, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_floor(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.floor call", &threeACcode);

    tOperand *labelIsInt = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsNotNum = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelEndFloor = create_operand_from_label(threeAC_create_label(&threeACcode));

    tOperand *argVal = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, argVal, NULL, NULL, &threeACcode);
    emit(OP_POPS, argVal, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelIsInt, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelIsNotNum, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INTS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, labelEndFloor, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelIsInt, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, argVal, NULL, NULL, &threeACcode);

    emit(OP_JUMP, labelEndFloor, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelIsNotNum, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelEndFloor, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_str(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.str call", &threeACcode);

    tOperand *argVal = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, argVal, NULL, NULL, &threeACcode);
    emit(OP_POPS, argVal, NULL, NULL, &threeACcode);

    tOperand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);

    tOperand *typeCheckVar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeCheckVar, NULL, NULL, &threeACcode);

    tOperand *labelEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    tOperand *labelIsString = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsInt = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsFloat = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelIsNil = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_TYPE, typeCheckVar, argVal, NULL, &threeACcode);

    emit(OP_JUMPIFEQ, labelIsNil, argVal, create_operand_from_constant_nil(), &threeACcode);
    emit(OP_JUMPIFEQ, labelIsString, typeCheckVar, create_operand_from_constant_string("string"),
         &threeACcode);
    emit(OP_JUMPIFEQ, labelIsInt, typeCheckVar, create_operand_from_constant_string("int"),
         &threeACcode);
    emit(OP_JUMPIFEQ, labelIsFloat, typeCheckVar, create_operand_from_constant_string("float"),
         &threeACcode);

    emit(OP_MOVE, resultStr, create_operand_from_constant_string(""), NULL,
         &threeACcode); // Empty string or error
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle string
    emit(OP_LABEL, labelIsString, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, argVal, NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle int
    emit(OP_LABEL, labelIsInt, NULL, NULL, &threeACcode);
    emit(OP_INT2STR, resultStr, argVal, NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle float
    emit(OP_LABEL, labelIsFloat, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2STR, resultStr, argVal, NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    // Handle nil
    emit(OP_LABEL, labelIsNil, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, create_operand_from_constant_string("null"), NULL, &threeACcode);
    emit(OP_JUMP, labelEnd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

tDataType generate_ifj_length(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.length call", &threeACcode);

    tOperand *strArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, strArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, strArg, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, strArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    tOperand *labelContinueLength = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelContinueLength, NULL, NULL, &threeACcode);

    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueLength, NULL, NULL, &threeACcode);

    tOperand *resultLen = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultLen, NULL, NULL, &threeACcode);

    emit(OP_STRLEN, resultLen, strArg, NULL, &threeACcode);
    emit(OP_PUSHS, resultLen, NULL, NULL, &threeACcode);

    return TYPE_NUM;
}

tDataType generate_ifj_substring(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.substring call", &threeACcode);

    tOperand *jArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, jArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, jArg, NULL, NULL, &threeACcode);

    tOperand *iArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, iArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, iArg, NULL, NULL, &threeACcode);

    tOperand *sArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, sArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, sArg, NULL, NULL, &threeACcode);

    tOperand *typeI = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    tOperand *typeJ = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeI, NULL, NULL, &threeACcode);
    emit(OP_DEFVAR, typeJ, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeI, iArg, NULL, &threeACcode);
    emit(OP_TYPE, typeJ, jArg, NULL, &threeACcode);

    emit(OP_PUSHS, sArg, NULL, NULL, &threeACcode);
    emit(OP_TYPES, NULL, NULL, NULL, &threeACcode);

    tOperand *labelParamTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelContinueSubstring =
        create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelCheckJEnd = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelCheckIEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelParamTypeError, NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQ, labelCheckJEnd, typeJ, create_operand_from_constant_string("int"),
         &threeACcode);
    emit(OP_JUMPIFNEQ, labelParamTypeError, typeJ, create_operand_from_constant_string("float"),
         &threeACcode);
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelParamTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, jArg, jArg, NULL, &threeACcode);
    emit(OP_LABEL, labelCheckJEnd, NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQ, labelContinueSubstring, typeI, create_operand_from_constant_string("int"),
         &threeACcode);
    emit(OP_JUMPIFNEQ, labelParamTypeError, typeI, create_operand_from_constant_string("float"),
         &threeACcode);
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelParamTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, iArg, iArg, NULL, &threeACcode);
    emit(OP_LABEL, labelCheckIEnd, NULL, NULL, &threeACcode);

    emit(OP_JUMP, labelContinueSubstring, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelParamTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueSubstring, NULL, NULL, &threeACcode);

    // Get length of s
    tOperand *lenS = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, lenS, NULL, NULL, &threeACcode);
    emit(OP_STRLEN, lenS, sArg, NULL, &threeACcode);

    // Labels for null return
    tOperand *labelReturnNull = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelEndSubstring = create_operand_from_label(threeAC_create_label(&threeACcode));

    // Boundary checks
    // i < 0
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // j < 0
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // i > j
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // i >= Ifj.length(s)  => NOT (i < Ifj.length(s))
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    // j > Ifj.length(s)
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, lenS, NULL, NULL, &threeACcode);
    emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelReturnNull, NULL, NULL, &threeACcode);

    tOperand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, create_operand_from_constant_string(""), NULL, &threeACcode);

    tOperand *loopCounter = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, loopCounter, NULL, NULL, &threeACcode);
    emit(OP_MOVE, loopCounter, iArg, NULL, &threeACcode);

    tOperand *currentChar = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, currentChar, NULL, NULL, &threeACcode);

    tOperand *loopStartLabelSub = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *loopEndLabelSub = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_LABEL, loopStartLabelSub, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, loopCounter, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, jArg, NULL, NULL, &threeACcode);
    emit(OP_LTS, NULL, NULL, NULL, &threeACcode); // Result (bool) is on stack
    emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, loopEndLabelSub, NULL, NULL, &threeACcode);

    emit(OP_GETCHAR, currentChar, sArg, loopCounter, &threeACcode);
    emit(OP_CONCAT, resultStr, resultStr, currentChar, &threeACcode);

    emit(OP_ADD, loopCounter, loopCounter, create_operand_from_constant_int(1), &threeACcode);
    emit(OP_JUMP, loopStartLabelSub, NULL, NULL, &threeACcode);
    emit(OP_LABEL, loopEndLabelSub, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);
    emit(OP_JUMP, labelEndSubstring, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelReturnNull, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_nil(), NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelEndSubstring, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

tDataType generate_ifj_chr(tSymTableStack *stack)
{
    emit(NO_OP, NULL, NULL, NULL, &threeACcode);
    emit_comment("Ifj.chr call", &threeACcode);

    tOperand *iArg = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, iArg, NULL, NULL, &threeACcode);
    emit(OP_POPS, iArg, NULL, NULL, &threeACcode);

    tOperand *typeI = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, typeI, NULL, NULL, &threeACcode);
    emit(OP_TYPE, typeI, iArg, NULL, &threeACcode);

    tOperand *labelTypeError = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *labelContinueChr = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, labelContinueChr, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, typeI, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_ISINTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INT, iArg, iArg, NULL, &threeACcode);
    emit(OP_JUMP, labelContinueChr, NULL, NULL, &threeACcode);

    emit(OP_LABEL, labelTypeError, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_PARAM_TYPE_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, labelContinueChr, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, iArg, NULL, NULL, &threeACcode);
    emit(OP_INT2CHARS, NULL, NULL, NULL, &threeACcode);

    return TYPE_STRING;
}

void generate_truthiness_check(tThreeACList *list, tOperand *expr_val)
{
    tOperand *finalBoolResult = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, finalBoolResult, NULL, NULL, list);

    tOperand *labelIsNull = create_operand_from_label(threeAC_create_label(list));
    tOperand *labelIsBool = create_operand_from_label(threeAC_create_label(list));
    tOperand *labelIsOther = create_operand_from_label(threeAC_create_label(list));
    tOperand *labelEndTruthiness = create_operand_from_label(threeAC_create_label(list));

    // Check if null
    emit(OP_JUMPIFEQ, labelIsNull, expr_val, create_operand_from_constant_nil(), list);

    // Check if boolean (using TYPE instruction)
    tOperand *typeCheckVar = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, typeCheckVar, NULL, NULL, list);
    emit(OP_TYPE, typeCheckVar, expr_val, NULL, list);

    emit(OP_JUMPIFEQ, labelIsBool, typeCheckVar, create_operand_from_constant_string("bool"), list);

    // If not null and not bool, it's true
    emit(OP_JUMP, labelIsOther, NULL, NULL, list);

    // Case: is null
    emit(OP_LABEL, labelIsNull, NULL, NULL, list);
    emit(OP_MOVE, finalBoolResult, create_operand_from_constant_bool(false), NULL, list);
    emit(OP_JUMP, labelEndTruthiness, NULL, NULL, list);

    // Case: is boolean
    emit(OP_LABEL, labelIsBool, NULL, NULL, list);
    emit(OP_MOVE, finalBoolResult, expr_val, NULL, list);
    emit(OP_JUMP, labelEndTruthiness, NULL, NULL, list);

    // Case: is other (number, string, etc.)
    emit(OP_LABEL, labelIsOther, NULL, NULL, list);
    emit(OP_MOVE, finalBoolResult, create_operand_from_constant_bool(true), NULL, list);
    emit(OP_JUMP, labelEndTruthiness, NULL, NULL, list);

    emit(OP_LABEL, labelEndTruthiness, NULL, NULL, list);
    emit(OP_PUSHS, finalBoolResult, NULL, NULL, list); // Push the final boolean result
}

void generate_add_op(tThreeACList *list)
{
    tOperand *op2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    tOperand *op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

    tOperand *type1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type1, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);

    tOperand *type2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type2, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    tOperand *endAddLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    tOperand *numAddLabelCheck = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *numAddLabelType = create_operand_from_label(threeAC_create_label(&threeACcode));

    tOperand *typeErrorLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, numAddLabelCheck, NULL, NULL, &threeACcode);

    tOperand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);
    emit(OP_CONCAT, resultStr, op1, op2, &threeACcode);
    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);
    emit(OP_JUMP, endAddLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numAddLabelCheck, NULL, NULL, &threeACcode);

    tOperand *operand1CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand1CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);

    emit(OP_LABEL, operand1CheckEnd, NULL, NULL, &threeACcode);

    tOperand *operand2CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand2CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);

    emit(OP_LABEL, operand2CheckEnd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numAddLabelType, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_ADDS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, endAddLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, endAddLabel, NULL, NULL, &threeACcode);
}

void generate_mult_op(tThreeACList *list)
{
    tOperand *op2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    tOperand *op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

    tOperand *type1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type1, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);

    tOperand *type2 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, type2, NULL, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    tOperand *endMultLabel = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *numMultLabelCheck = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *numMultLabelType = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *typeErrorLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ORS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_EQS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ORS, NULL, NULL, NULL, &threeACcode);
    emit(OP_ANDS, NULL, NULL, NULL, &threeACcode);

    emit(OP_ORS, NULL, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);

    emit(OP_JUMPIFNEQS, numMultLabelCheck, NULL, NULL, &threeACcode);

    tOperand *replaceOperand =
        create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    tOperand *replaceEndLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_DEFVAR, replaceOperand, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("string"), NULL, NULL, &threeACcode);

    emit(OP_JUMPIFEQS, replaceEndLabel, NULL, NULL, &threeACcode);

    emit(OP_MOVE, replaceOperand, op1, NULL, &threeACcode);
    emit(OP_MOVE, op1, op2, NULL, &threeACcode);
    emit(OP_MOVE, op2, replaceOperand, NULL, &threeACcode);
    emit(OP_TYPE, type1, op1, NULL, &threeACcode);
    emit(OP_TYPE, type2, op2, NULL, &threeACcode);

    emit(OP_LABEL, replaceEndLabel, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);

    tOperand *op2IsIntLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_JUMPIFNEQS, op2IsIntLabel, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_FLOAT2INTS, NULL, NULL, NULL, &threeACcode);
    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

    emit(OP_LABEL, op2IsIntLabel, NULL, NULL, &threeACcode);

    tOperand *resultStr = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
    emit(OP_DEFVAR, resultStr, NULL, NULL, &threeACcode);
    emit(OP_MOVE, resultStr, create_operand_from_constant_string(""), NULL, &threeACcode);

    tOperand *loopStart = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *loopEnd = create_operand_from_label(threeAC_create_label(&threeACcode));
    tOperand *condition = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);

    emit(OP_DEFVAR, condition, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loopStart, NULL, NULL, &threeACcode);

    emit(OP_GT, condition, op2, create_operand_from_constant_int(0), &threeACcode);
    emit(OP_JUMPIFNEQ, loopEnd, condition, create_operand_from_constant_bool(true), &threeACcode);

    emit(OP_CONCAT, resultStr, resultStr, op1, &threeACcode);

    emit(OP_SUB, op2, op2, create_operand_from_constant_int(1), &threeACcode);
    emit(OP_JUMP, loopStart, NULL, NULL, &threeACcode);

    emit(OP_LABEL, loopEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, resultStr, NULL, NULL, &threeACcode);

    emit(OP_JUMP, endMultLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numMultLabelCheck, NULL, NULL, &threeACcode);

    tOperand *operand1CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand1CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);

    emit(OP_LABEL, operand1CheckEnd, NULL, NULL, &threeACcode);

    tOperand *operand2CheckEnd = create_operand_from_label(threeAC_create_label(&threeACcode));

    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFEQS, operand2CheckEnd, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, type2, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, &threeACcode);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_INT2FLOAT, op2, op2, NULL, &threeACcode);

    emit(OP_LABEL, operand2CheckEnd, NULL, NULL, &threeACcode);

    emit(OP_LABEL, numMultLabelType, NULL, NULL, &threeACcode);

    emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
    emit(OP_PUSHS, op2, NULL, NULL, &threeACcode);
    emit(OP_MULS, NULL, NULL, NULL, &threeACcode);

    emit(OP_JUMP, endMultLabel, NULL, NULL, &threeACcode);

    emit(OP_LABEL, typeErrorLabel, NULL, NULL, &threeACcode);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL,
         &threeACcode);

    emit(OP_LABEL, endMultLabel, NULL, NULL, &threeACcode);
}

void generate_numeric_op(tThreeACList *list, char *op)
{
    tOperand *op2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op2, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);

    tOperand *op1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op1, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);

    tOperand *type1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type1, NULL, NULL, list);
    emit(OP_TYPE, type1, op1, NULL, list);

    tOperand *type2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type2, NULL, NULL, list);
    emit(OP_TYPE, type2, op2, NULL, list);

    tOperand *typeErrorLabel = create_operand_from_label(threeAC_create_label(list));
    tOperand *afterNumTypeCheckLabel = create_operand_from_label(threeAC_create_label(list));

    emit(OP_PUSHS, type1, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, list);
    emit(OP_EQS, NULL, NULL, NULL, list);
    emit(OP_PUSHS, type1, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_EQS, NULL, NULL, NULL, list);
    emit(OP_ORS, NULL, NULL, NULL, list);

    emit(OP_PUSHS, type2, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, list);
    emit(OP_EQS, NULL, NULL, NULL, list);
    emit(OP_PUSHS, type2, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_EQS, NULL, NULL, NULL, list);
    emit(OP_ORS, NULL, NULL, NULL, list);

    emit(OP_ANDS, NULL, NULL, NULL, list);

    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, list);

    emit(OP_JUMP, afterNumTypeCheckLabel, NULL, NULL, list);

    emit(OP_LABEL, typeErrorLabel, NULL, NULL, list);
    emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL, NULL,
         list);

    emit(OP_LABEL, afterNumTypeCheckLabel, NULL, NULL, list);

    tOperand *intType = create_operand_from_constant_string("int");

    tOperand *isInt1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, isInt1, NULL, NULL, list);
    emit(OP_EQ, isInt1, type1, intType, list);
    tOperand *op1OkLabel = create_operand_from_label(threeAC_create_label(list));
    emit(OP_JUMPIFNEQ, op1OkLabel, isInt1, create_operand_from_constant_bool(true), list);
    emit(OP_INT2FLOAT, op1, op1, NULL, list);
    emit(OP_LABEL, op1OkLabel, NULL, NULL, list);

    tOperand *isInt2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, isInt2, NULL, NULL, list);
    emit(OP_EQ, isInt2, type2, intType, list);
    tOperand *op2OkLabel = create_operand_from_label(threeAC_create_label(list));
    emit(OP_JUMPIFNEQ, op2OkLabel, isInt2, create_operand_from_constant_bool(true), list);
    emit(OP_INT2FLOAT, op2, op2, NULL, list);
    emit(OP_LABEL, op2OkLabel, NULL, NULL, list);

    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_PUSHS, op2, NULL, NULL, list);

    if (strcmp(op, "-") == 0)
    {
        emit(OP_SUBS, NULL, NULL, NULL, list);
    }
    else if (strcmp(op, "/") == 0)
    {
        emit(OP_DIVS, NULL, NULL, NULL, list);
    }
    else if (strcmp(op, "*") == 0)
    {
        emit(OP_MULS, NULL, NULL, NULL, list);
    }
    else if (strcmp(op, "+") == 0)
    {
        emit(OP_ADDS, NULL, NULL, NULL, list);
    }
}

void generate_relational_op(tThreeACList *list, char *op)
{
    tOperand *op2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op2, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);

    tOperand *op1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, op1, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);

    tOperand *type1 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type1, NULL, NULL, list);
    emit(OP_TYPE, type1, op1, NULL, list);

    tOperand *type2 = create_operand_from_variable(threeAC_create_temp(list), false);
    emit(OP_DEFVAR, type2, NULL, NULL, list);
    emit(OP_TYPE, type2, op2, NULL, list);

    if (strcmp(op, "==") != 0 && strcmp(op, "!=") != 0)
    {
        tOperand *typeErrorLabel = create_operand_from_label(threeAC_create_label(list));
        tOperand *afterNumTypeCheckLabel = create_operand_from_label(threeAC_create_label(list));

        emit(OP_PUSHS, type1, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_PUSHS, type1, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_ORS, NULL, NULL, NULL, list);

        emit(OP_PUSHS, type2, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("float"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_PUSHS, type2, NULL, NULL, list);
        emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
        emit(OP_EQS, NULL, NULL, NULL, list);
        emit(OP_ORS, NULL, NULL, NULL, list);

        emit(OP_ANDS, NULL, NULL, NULL, list);

        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, list);
        emit(OP_JUMPIFNEQS, typeErrorLabel, NULL, NULL, list);

        emit(OP_JUMP, afterNumTypeCheckLabel, NULL, NULL, list);

        emit(OP_LABEL, typeErrorLabel, NULL, NULL, list);
        emit(OP_EXIT, create_operand_from_constant_int(RUNTIME_TYPE_COMPATIBILITY_ERROR), NULL,
             NULL, list);

        emit(OP_LABEL, afterNumTypeCheckLabel, NULL, NULL, list);
    }

    tOperand *op1ToFloatIfInt = create_operand_from_label(threeAC_create_label(list));
    emit(OP_PUSHS, type1, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, op1ToFloatIfInt, NULL, NULL, list);

    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_INT2FLOATS, NULL, NULL, NULL, list);
    emit(OP_POPS, op1, NULL, NULL, list);
    emit(OP_MOVE, type1, create_operand_from_constant_string("float"), NULL, list);

    emit(OP_LABEL, op1ToFloatIfInt, NULL, NULL, list);

    tOperand *op2ToFloatIfInt = create_operand_from_label(threeAC_create_label(list));
    emit(OP_PUSHS, type2, NULL, NULL, list);
    emit(OP_PUSHS, create_operand_from_constant_string("int"), NULL, NULL, list);
    emit(OP_JUMPIFNEQS, op2ToFloatIfInt, NULL, NULL, list);

    emit(OP_PUSHS, op2, NULL, NULL, list);
    emit(OP_INT2FLOATS, NULL, NULL, NULL, list);
    emit(OP_POPS, op2, NULL, NULL, list);
    emit(OP_MOVE, type2, create_operand_from_constant_string("float"), NULL, list);

    emit(OP_LABEL, op2ToFloatIfInt, NULL, NULL, list);
    tOperand *performOpLabelOnSameType = create_operand_from_label(threeAC_create_label(list));
    tOperand *endRelOpLabel = create_operand_from_label(threeAC_create_label(list));

    emit(OP_JUMPIFEQ, performOpLabelOnSameType, type1, type2, list);
    if (strcmp(op, "!=") == 0)
    {
        emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, list);
    }
    else
    {
        emit(OP_PUSHS, create_operand_from_constant_bool(false), NULL, NULL, list);
    }

    emit(OP_JUMP, endRelOpLabel, NULL, NULL, list);

    emit(OP_LABEL, performOpLabelOnSameType, NULL, NULL, list);
    emit(OP_PUSHS, op1, NULL, NULL, list);
    emit(OP_PUSHS, op2, NULL, NULL, list);

    tOperationType opType;
    bool useNot = false;

    if (strcmp(op, "<") == 0)
        opType = OP_LTS;
    else if (strcmp(op, ">") == 0)
        opType = OP_GTS;
    else if (strcmp(op, "==") == 0)
        opType = OP_EQS;
    else if (strcmp(op, "!=") == 0)
    {
        opType = OP_EQS;
        useNot = true;
    }
    else if (strcmp(op, "<=") == 0)
    {
        opType = OP_GTS;
        useNot = true;
    }
    else if (strcmp(op, ">=") == 0)
    {
        opType = OP_LTS;
        useNot = true;
    }

    emit(opType, NULL, NULL, NULL, list);
    if (useNot)
    {
        emit(OP_NOTS, NULL, NULL, NULL, list);
    }
    emit(OP_LABEL, endRelOpLabel, NULL, NULL, list);
}
