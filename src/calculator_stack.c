#include <stdlib.h>
#include <assert.h>
#include "calculator_stack.h"


struct CSStackHunk *CSAllocHunk()
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

bool CSCanExecute(CalculatorStack *cs, CSOperation op)
{
    switch (op) {
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


static Poly CSPopPoly(CalculatorStack *cs)
{
    assert(cs->size > 0);
    if (cs->topHunkTop == 0) {
        cs->topHunk = cs->topHunk->prevHunk;
        free(cs->topHunk->nextHunk);
        cs->topHunk->nextHunk = NULL;
        cs->topHunkTop = CS_HUNK_SIZE;
    }
    return cs->topHunk->data[--cs->topHunkTop];
}

