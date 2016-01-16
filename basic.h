#ifndef BASIC_H
#define BASIC_H

#include <stdio.h>

typedef char* ExpressionOperation; // null terminated, global statically alloced

typedef union {
    char variable;
    double value;
    ExpressionOperation operation;
} ExpressionUnit;

typedef struct {
    ExpressionUnit unit;
    int mode;
} ExpressionPart;

typedef struct {
    ExpressionPart* parts;
    int capacity;
    int count;
} Expression;

Expression makeExpression(char* string);

// exp isn't changed
double evalExpression(Expression exp, double* variables);

// returns null-terminated malloced array
char* variablesInExpression(Expression exp);

double getDouble(FILE* file);

#endif