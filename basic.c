#include "basic.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

Expression newExp(void)
{
    Expression exp;
    exp.capacity = 5;
    exp.count = 0;
    exp.parts = malloc(exp.capacity * sizeof(ExpressionPart));
    return exp;
}

void appendPart(Expression* exp, ExpressionPart part)
{
    if (exp->count >= exp->capacity)
    {
        exp->capacity *= 2;
        exp->parts = realloc(exp->parts, exp->capacity*sizeof(ExpressionPart));
    }
    exp->parts[exp->count++] = part;
}

void appendVariable(Expression* exp, char var)
{
    ExpressionPart part;
    part.mode = 0;
    part.unit.variable = var;
    appendPart(exp, part);
}

void appendOperation(Expression* exp, ExpressionOperation operation)
{
    ExpressionPart part;
    part.mode = 2;
    part.unit.operation = operation;
    appendPart(exp, part);
}

void appendValue(Expression* exp, double val)
{
    ExpressionPart part;
    part.mode = 1;
    part.unit.value = val;
    appendPart(exp, part);
}

void appendExpressionOfLen(Expression* exp, char* string, int len);

bool isBinaryOp(char c)
{
    return c=='+' || c=='-' || c=='/' || c=='*' || c=='^';
}

// give left-associative binary operation
// input isn't all nested
// returns whether binary op was found
bool analyzeForBinaryOp(Expression* exp, char* string, int len,
    ExpressionOperation op)
{
    int opLen = strlen(op);
    int parenDepth = 0;
    for (int i = len - 1; i >= 0; i--)
    {
        if (string[i] == ')') parenDepth++;
        else if (string[i] == '(') parenDepth--;
        else if (parenDepth == 0 && strncmp(string+i, op, opLen)==0)
        {
            // make sure it isn't unary
            if (i==0 ||string[i-1] == '(' || isBinaryOp(string[i-1])) continue;
            assert(i<len-1);
            appendExpressionOfLen(exp, string, i);
            appendExpressionOfLen(exp, string+i+1, len-i-1);
            appendOperation(exp, op);
            return true;
        }
    }
    return false;
}

bool analyzeForDouble(char* string, int len, double* value)
{
    if (len <= 0) return false;
    bool neg = string[0] == '-';
    if (neg)
    {
        string++;
        len--;
    }
    if (len <= 0) return false;
    double val = 0;
    int decimalPlaces = 0;
    bool foundDecimal = false;
    for (int i = 0; i < len; i++)
    {
        if (string[i] == '.' && !foundDecimal) foundDecimal = true;
        else
        {
            if (string[i] < '0' || string[i] > '9') return false;
            int digit = string[i] - '0';
            if (foundDecimal) decimalPlaces++;
            val *= 10;
            val += digit;
        }
    }
    for (int i = 0; i < decimalPlaces; i++)
    {
        val /= 10;
    }
    if (neg) val *= -1;
    *value = val;
    return true;
}

bool analyzeForValue(Expression* exp, char* string, int len)
{
    double val;
    bool found = analyzeForDouble(string, len, &val);
    if (found) appendValue(exp, val);
    return found;
}

// converts infix notation to RPN
void appendExpressionOfLen(Expression* exp, char* string, int len)
{
    if (len <= 0)
    {
        char* n = NULL;
        *n = 4;
    }
    assert(len > 0);
    ExpressionOperation unary = NULL;
    if (strncmp(string, "sqrt(", 5) == 0) unary = "sqrt";
    else if (strncmp(string, "asin(", 5) == 0) unary = "asin";
    else if (strncmp(string, "acos(", 5) == 0) unary = "asin";
    else if (strncmp(string, "atan(", 5) == 0) unary = "atan";
    else if (strncmp(string, "sin(", 4) == 0) unary = "sin";
    else if (strncmp(string, "cos(", 4) == 0) unary = "cos";
    else if (strncmp(string, "tan(", 4) == 0) unary = "tan";
    else if (strncmp(string, "ln(", 3) == 0) unary = "ln";
    else if (strncmp(string, "log(", 4) == 0) unary = "log";
    else if (strncmp(string, "-(", 2) == 0) unary = "-";
    if (unary) string += strlen(unary), len -= strlen(unary);
    int parenDepth = 0;
    bool allNested = true;
    for (int i = 0; i < len; i++)
    {
        if (string[i] == '(')
        {
            parenDepth++;
        }
        else if (string[i] == ')')
        {
            parenDepth--;
        }
        else if (parenDepth == 0) allNested = false;
    }
    if (allNested)
    {
        appendExpressionOfLen(exp, string+1, len-2);
        if (unary) appendOperation(exp, unary);
        return;
    }
    // unary is part of sub-operand. don't pay attention to it now
    if (unary) string -= strlen(unary), len += strlen(unary);
    if (!analyzeForBinaryOp(exp, string, len, "+"))
    if (!analyzeForBinaryOp(exp, string, len, "-"))
    if (!analyzeForBinaryOp(exp, string, len, "*"))
    if (!analyzeForBinaryOp(exp, string, len, "/"))
    if (!analyzeForBinaryOp(exp, string, len, "^"))
    if (!analyzeForValue(exp, string, len))
    {
        // could be constant (represented by no-operand operation)
        if (strcmp(string, "pi") == 0) return appendOperation(exp, "pi");
        // must be variable
        assert(len == 1);
        appendVariable(exp, string[0]);
    }
}

Expression makeExpression(char* string)
{
    Expression exp = newExp();
    appendExpressionOfLen(&exp, string, strlen(string));
    return exp;
}

Expression copyExpression(Expression exp)
{
    Expression e;
    e.count = exp.count;
    e.capacity = exp.count+1;
    e.parts = malloc(e.capacity * sizeof(ExpressionPart));
    for (int i = 0; i < exp.count; i++)
    {
        e.parts[i] = exp.parts[i];
    }
    return e;
}

ExpressionPart popPart(Expression* exp)
{
    return exp->parts[--exp->count];
}

double popValue(Expression* exp, double* variables)
{
    ExpressionPart part = popPart(exp);
    if (part.mode == 1) return part.unit.value;
    if (part.mode == 0) return variables[part.unit.variable];
    ExpressionOperation op = part.unit.operation;
    if (strcmp(op, "pi") == 0) return 3.1415926535897932384626433832795;
    double operand = popValue(exp, variables);
    if (strcmp(op, "+") == 0)
    {
        return popValue(exp, variables) + operand;
    }
    else if (strcmp(op, "-") == 0)
    {
        if (exp->count) return popValue(exp, variables) - operand;
        else return -operand;
    }
    else if (strcmp(op, "*") == 0)
    {
        return popValue(exp, variables) * operand;
    }
    else if (strcmp(op, "/") == 0)
    {
        return popValue(exp, variables) / operand;
    }
    else if (strcmp(op, "^") == 0)
    {
        return pow(popValue(exp, variables), operand);
    }
    else if (strcmp(op, "sqrt") == 0) return sqrt(operand);
    else if (strcmp(op, "sin") == 0) return sin(operand);
    else if (strcmp(op, "cos") == 0) return cos(operand);
    else if (strcmp(op, "tan") == 0) return tan(operand);
    else if (strcmp(op, "asin") == 0) return asin(operand);
    else if (strcmp(op, "acos") == 0) return acos(operand);
    else if (strcmp(op, "atan") == 0) return atan(operand);
    else if (strcmp(op, "log") == 0) return log10(operand);
    else if (strcmp(op, "ln") == 0) return log(operand);
    fprintf(stderr, "Unknown operation %s\n", op);
    exit(1);
}

double evalExpression(Expression originalExp, double* variables)
{
    Expression exp = copyExpression(originalExp);
    double evaled = popValue(&exp, variables);
    assert(exp.count == 0);
    free(exp.parts);
    return evaled;
}

char* variablesInExpression(Expression exp)
{
    bool vars[128] = {false};
    for (int i = 0; i < exp.count; i++)
    {
        if (exp.parts[i].mode == 0)
        {
            vars[exp.parts[i].unit.variable] = true;
        }
    }
    int count = 0;
    int capacity = 10;
    char* variables = malloc(capacity);
    for (int i = 1; i < 128; i++)
    {
        if (vars[i])
        {
            if (count >= capacity)
            {
                capacity *= 2;
                variables = realloc(variables, capacity);
            }
            variables[count++] = i;
        }
    }
    variables[count] = '\0';
    return variables;
}

double getDouble(FILE* file)
{
    int count = 0;
    int capacity = 10;
    char* expression = malloc(capacity);
    int c;
    while (isprint(c = fgetc(file)))
    {
        if (count >= capacity)
        {
            capacity *= 2;
            expression = realloc(expression, capacity);
        }
        expression[count++] = c;
    }
    expression[count] = '\0';
    double value;
    analyzeForDouble(expression, count, &value);
    free(expression);
    return value;
}
