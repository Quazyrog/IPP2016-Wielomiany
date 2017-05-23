#include "poly.h"

#ifndef WIELOMIANY_CALCULATOR_STACK_H
#define WIELOMIANY_CALCULATOR_STACK_H

#define CS_HUNK_SIZE 254


struct CSStackHunk
{
    struct CSStackHunk *prevHunk;
    struct CSStackHunk *nextHunk;
    Poly data[CS_HUNK_SIZE];
};


/**
 * Struktura przechowująca dane stosu kalkulatora.
 */
typedef struct {
    uint32_t topHunkTop;
    uint32_t size;
    poly_exp_t peArg;
    poly_coeff_t pcArg;
    struct CSStackHunk *topHunk;
    struct CSStackHunk *bottomHunk;
} CalculatorStack;


typedef enum {
    ///Wstawia na wierzchołek stosu wielomian tożsamościowo równy zeru
    OPERATION_ZERO,

    ///Sprawdza, czy wielomian na wierzchołku stosu jest współczynnikiem – wypisuje na standardowe wyjście 0 lub 1
    OPERATION_IS_COEFF,

    ///Sprawdza, czy wielomian na wierzchołku stosu jest tożsamościowo równy zeru – wypisuje na standardowe wyjście 0 lub 1
    OPERATION_IS_ZERO,

    ///Wstawia na stos kopię wielomianu z wierzchołka
    OPERATION_CLONE,

    ///Dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek stosu ich sumę
    OPERATION_ADD,

    ///Mnoży dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek stosu ich iloczyn
    OPERATION_MUL,

    ///Neguje wielomian na wierzchołku stosu
    OPERATION_NEG,

    ///Odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem, usuwa je i wstawia na wierzchołek stosu różnicę
    OPERATION_SUB,

    ///Wprawdza, czy dwa wielomiany na wierzchu stosu są równe – wypisuje na standardowe wyjście 0 lub 1
    OPERATION_IS_EQ,

    ///Wypisuje na standardowe wyjście stopień wielomianu (−1 dla wielomianu tożsamościowo równego zeru)
    OPERATION_DEG,

    ///Wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną o numerze idx (−1 dla wielomianu tożsamościowo równego zeru)
    OPERATION_DEG_BY,

    ///Wylicza wartość wielomianu w punkcie x, usuwa wielomian z wierzchołka i wstawia na stos wynik operacji
    OPERATION_AT,

    ///Wypisuje na standardowe wyjście wielomian z wierzchołka stosu w formacie akceptowanym przez parser
    OPERATION_PRINT,

    ///Usuwa wielomian z wierzchołka stosu
    OPERATION_POP,
} CSOperation;


CSOperation CSOperationFromString(const char *op_name);

CalculatorStack CSInit();

void CSDestroy(CalculatorStack *cs);

void CSPushPolynomial(CalculatorStack *cs, Poly poly);

bool CSCanExecute(CalculatorStack *cs, CSOperation op);

void CSExecute(CalculatorStack *cs, CSOperation op);

static inline void CSSetPEArg(CalculatorStack *cs, poly_exp_t arg)
{
    cs->peArg = arg;
}

static inline void CSSetPCArg(CalculatorStack *cs, poly_coeff_t arg)
{
    cs->pcArg = arg;
}


#endif //WIELOMIANY_CALCULATOR_STACK_H
