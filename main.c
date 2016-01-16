
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "basic.h"
#include "integral.h"
#include <stdlib.h>

int main(int argc, char** argv)
{
    Expression xSquared = makeExpression("(x+3)^2/-4");
    double variables[128];
    variables['x'] = 3.0;
    double evaled = evalExpression(xSquared, variables);
    printf("Result is %g\n", evaled);
    // should be -167117
    double result = evalDefiniteIntegral(xSquared, 'x', -100, 100, 1000, 10);
    printf("Integral is %g\n", result);
    free(xSquared.parts);

    Expression sqrtX = makeExpression("sqrt(x)+2");
    result = evalDefiniteIntegral(sqrtX, 'x', 1, 10, 100, 2);
    free(sqrtX.parts);
    // should be 38.4152
    printf("Integral is %g\n", result);
    while (true)
    {
        printf("Input expression: ");
        int count = 0;
        int capacity = 10;
        char* expression = malloc(capacity);
        int c;
        while (isprint(c = getchar()))
        {
            if (count >= capacity)
            {
                capacity *= 2;
                expression = realloc(expression, capacity);
            }
            expression[count++] = c;
        }
        expression[count] = '\0';
        if (count == 0) {free(expression); break;}
        Expression exp = makeExpression(expression);
        free(expression);
        char* vars = variablesInExpression(exp);
        for (int i = 0, n=strlen(vars); i < n; i++)
        {
            printf("Input value for variable %c: ", vars[i]);
            double value = getDouble(stdin);
            variables[vars[i]] = value;
        }
        free(vars);
        evaled = evalExpression(exp, variables);
        printf("Result is %g\n", evaled);
        free(exp.parts);
    }
    return 0;
}
