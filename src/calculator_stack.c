#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "calculator_stack.h"


static struct CSStackHunk *CSAllocHunk();
static Poly *CSTopPtr(CalculatorStack *cs);
static Poly CSPopPolynomial(CalculatorStack *cs);



static struct CSStackHunk *CSAllocHunk()
{
    struct CSStackHunk *result = malloc(sizeof(struct CSStackHunk));
    assert(result != NULL);
    result->nextHunk = NULL;
    result->prevHunk = NULL;
    return result;
}


CalculatorStack CSInit()
{
    CalculatorStack result;

    result.bottomHunk = CSAllocHunk();
    result.topHunk = result.bottomHunk;
    result.size = 0;
    result.topHunkTop = 0;

    return result;
}


void CSDestroy(CalculatorStack *cs)
{
    for (struct CSStackHunk *it = cs->bottomHunk; it != NULL;) {
        struct CSStackHunk *next_hunk = it->nextHunk;
        free(it);
        it = next_hunk;
    }
}


void CSPushPolynomial(CalculatorStack *cs, Poly poly)
{
    if (cs->topHunkTop >= CS_HUNK_SIZE) {
        struct CSStackHunk *new_hunk = CSAllocHunk();
        cs->topHunk->nextHunk = new_hunk;
        new_hunk->prevHunk = cs->topHunk;
        cs->topHunk = new_hunk;
        cs->topHunkTop = 0;
    }
    cs->topHunk->data[cs->topHunkTop++] = poly;
    ++cs->size; //http://en.cppreference.com/w/c/language/operator_precedence :)
}


static Poly CSPopPolynomial(CalculatorStack *cs)
{
    assert(cs->size > 0);
    if (cs->topHunkTop == 0) {
        cs->topHunk = cs->topHunk->prevHunk;
        free(cs->topHunk->nextHunk);
        cs->topHunk->nextHunk = NULL;
        cs->topHunkTop = CS_HUNK_SIZE;
    }
    --cs->size;
    return cs->topHunk->data[--cs->topHunkTop];
}


static Poly *CSTopPtr(CalculatorStack *cs)
{
    assert(cs->size > 0);
    if (cs->topHunkTop != 0)
        return cs->topHunk->data + (cs->topHunkTop - 1);
    return cs->topHunk->prevHunk->data + (CS_HUNK_SIZE - 1);
}


bool CSCanExecute(CalculatorStack *cs, CSOperation op)
{
    switch (op) {
        case OPERATION_INVALID:
            return false;
        case OPERATION_ZERO:
            return true;
        case OPERATION_IS_COEFF:
        case OPERATION_IS_ZERO:
        case OPERATION_CLONE:
        case OPERATION_NEG:
        case OPERATION_DEG:
        case OPERATION_DEG_BY:
        case OPERATION_AT:
        case OPERATION_PRINT:
        case OPERATION_POP:
            return cs->size > 0;
        case OPERATION_ADD:
        case OPERATION_MUL:
        case OPERATION_SUB:
        case OPERATION_IS_EQ:
            return cs->size > 1;
    }
    return false;
}


CSOperation CSOperationFromString(const char *op_name) {
    if (strcmp(op_name, "ZERO") == 0)
        return OPERATION_ZERO;
    if (strcmp(op_name, "IS_COEFF") == 0)
        return OPERATION_IS_COEFF;
    if (strcmp(op_name, "IS_ZERO") == 0)
        return OPERATION_IS_ZERO;
    if (strcmp(op_name, "CLONE") == 0)
        return OPERATION_CLONE;
    if (strcmp(op_name, "NEG") == 0)
        return OPERATION_NEG;
    if (strcmp(op_name, "DEG") == 0)
        return OPERATION_DEG;
    if (strcmp(op_name, "DEG_BY") == 0)
        return OPERATION_DEG_BY;
    if (strcmp(op_name, "AT") == 0)
        return OPERATION_AT;
    if (strcmp(op_name, "PRINT") == 0)
        return OPERATION_PRINT;
    if (strcmp(op_name, "POP") == 0)
        return OPERATION_POP;
    if (strcmp(op_name, "ADD") == 0)
        return OPERATION_ADD;
    if (strcmp(op_name, "MUL") == 0)
        return OPERATION_MUL;
    if (strcmp(op_name, "SUB") == 0)
        return OPERATION_SUB;
    if (strcmp(op_name, "IS_EQ") == 0)
        return OPERATION_IS_EQ;
    return OPERATION_INVALID;
}


static void CSBinaryOperator(CalculatorStack *cs, Poly (*op)(const Poly *, const Poly *))
{
    Poly rarg = CSPopPolynomial(cs);
    Poly larg = CSPopPolynomial(cs);
    Poly result = op(&rarg, &larg);
    CSPushPolynomial(cs, result);
}


void CSExecute(CalculatorStack *cs, CSOperation op, FILE *out) {
    assert(CSCanExecute(cs, op));
    Poly p1, p2;

    switch (op) {
        case OPERATION_INVALID:
            break;
        case OPERATION_ZERO:
            CSPushPolynomial(cs, PolyZero());
            break;
        case OPERATION_IS_COEFF:
            fprintf(out, "%u\n", (unsigned int)PolyIsCoeff(CSTopPtr(cs)));
            break;
        case OPERATION_IS_ZERO:
            fprintf(out, "%u\n", (unsigned int)PolyIsZero(CSTopPtr(cs)));
            break;
        case OPERATION_CLONE:
            CSPushPolynomial(cs, PolyClone(CSTopPtr(cs)));
            break;
        case OPERATION_ADD:
            CSBinaryOperator(cs, PolyAdd);
            break;
        case OPERATION_MUL:
            CSBinaryOperator(cs, PolyMul);
            break;
        case OPERATION_NEG:
            PolyScaleInplace(CSTopPtr(cs), -1);
            break;
        case OPERATION_SUB:
            CSBinaryOperator(cs, PolySub);
            break;
        case OPERATION_IS_EQ:
            p1 = CSPopPolynomial(cs);
            p2 = CSPopPolynomial(cs);
            fprintf(out, "%u\n", (unsigned int)PolyIsEq(&p1, &p2));
            CSPushPolynomial(cs, p2);
            CSPushPolynomial(cs, p1);
            break;
        case OPERATION_DEG:
            fprintf(out, "%u\n", (unsigned int)PolyDeg(CSTopPtr(cs)));
            break;
        case OPERATION_DEG_BY:
            fprintf(out, "%u\n", (unsigned int)PolyDegBy(CSTopPtr(cs), (unsigned int)cs->peArg));
            break;
        case OPERATION_AT:
            p1 = CSPopPolynomial(cs);
            CSPushPolynomial(cs, PolyAt(&p1, cs->pcArg));
            break;
        case OPERATION_PRINT:
            PolyPrint(CSTopPtr(cs), out);
            break;
        case OPERATION_POP:
            PolyDestroy(CSTopPtr(cs));
            CSPopPolynomial(cs);
            break;
    }
}

