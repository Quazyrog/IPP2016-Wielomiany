/** @file calculator_stack.h
 * Struktura stosu kalkulatora odpowiedzialna za wykonywanie tych operacji.
 */
#include "poly.h"

#ifndef WIELOMIANY_CALCULATOR_STACK_H
#define WIELOMIANY_CALCULATOR_STACK_H


/**
 * Struktura przechowująca dane stosu kalkulatora.
 * Stos jest zbudowany ze stosu tablic, zawierajacych więcej niż jeden wielomian.
 */
typedef struct {
    ///Indeks pierwszego wolnego slotu w wierzchnim segmencie stosu
    uint32_t topHunkTop;

    ///Liczba wszystkich elementów na stosie
    uint32_t size;

    ///Argument dodatkowy dla operacji <c>OPERATION_DEG_BY</c> oraz <c>OPERATION_COMPOSE</c>
    unsigned int uiArg;

    ///Argument dodatkowy dla operacji <c>OPERATION_AT</c>
    poly_coeff_t pcArg;

    ///Wskaźnik na wierzchni segment stosu
    struct CSStackHunk *topHunk;

    ///Wskaźnik na pierwszy segment stosu
    struct CSStackHunk *bottomHunk;
} CalculatorStack;


/**
 * Wszystkie typy operacji, jakie potrafi wykonywać kalkulator.
 */
typedef enum {
    ///Zwracane przez <c>CSOperationFromString()</c>, kiedy podano niepoprawny napis
    OPERATION_INVALID,

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

    ///Wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną o numerze idx
    ///(−1 dla wielomianu tożsamościowo równego zeru); wymaga ustawienia wartości odpowiedniego parametru
    ///typu <c>unsigned int</c>
    ///@see CSSetUIArg()
    OPERATION_DEG_BY,

    ///Wylicza wartość wielomianu w punkcie x, usuwa wielomian z wierzchołka i wstawia na stos wynik operacji;
    ///wymaga ustawienia wartości odpowiedniego parametru typu <c>poly_coeff_t</c>
    ///@see CSSetPCArg()
    OPERATION_AT,

    ///Wypisuje na standardowe wyjście wielomian z wierzchołka stosu w formacie akceptowanym przez parser
    OPERATION_PRINT,

    ///Usuwa wielomian z wierzchołka stosu
    OPERATION_POP,

    ///Składa wielomiany metodą PolyCompose; wymaga ustawienia wartości odpowiedniego oapametru
    ///@see CSSetUIArg()
    OPERATION_COMPOSE,
} CSOperation;


/**
 * Konwertuje tekstowe polecenie dla kalkulatora na jego kod operacji.
 * @param op_name tekstowe polecenia zapisane wielkimi literami ASCII oraz podkreślnikami
 * @return kod operacji <c>CSOperation</c> odpowiadający danemu poleceniu tekstowemy; <c>OPERATION_INVALID</c> gdy
 * polecenie jest niepoprawne
 */
CSOperation CSOperationFromString(const char *op_name);

/**
 * Inicjalizuje stos i allokuje mu pamięć.
 * Wartosci <c>peArg</c> i <c>pcArg</c> nie są inicjowane. Stworzony stos należy potem zwolnić używając CSDestroy.
 * @return Zainicjowana strukturę
 */
CalculatorStack CSInit();

/**
 * Zwalnia pamięć strukruty stosu.
 * Bezpieczne jest wielokrotne wywoływanie tej funkcji na jednej instancji.
 * @param cs struktura do usunięcia
 */
void CSDestroy(CalculatorStack *cs);

/**
 * Wpycha wielomian na stos.
 * @param cs stos wielomianów
 * @param poly wielomian wo wepchnięcia
 */
void CSPushPolynomial(CalculatorStack *cs, Poly poly);

/**
 * Sprawdza, czy na stosie jest wystarczająco wiele argumentów, aby wykonać żądaną ofrrację.
 * Zawsze zwraca <c>false</c>, gdy żądana operacja to <c>OPERATION_INVALID</c> lub po prostu niepoprawny kod operacji.
 * W pozostałych przypadkach wykonuje jedynie sprawdzenie, czy na stosie znajduje się wystarczająco wiele argumentów.
 * Nie sprawdza jednak argumentów <c>CalculatorStack.peArg</c> oraz <c>CalculatorStack.pcArg</c> (czy zostały
 * zainicjowane przed wykonaniem operacji).
 * @param cs stos, na którym chcemy wykonać operację
 * @param op kod operacji, którą chcemy wykonać
 * @return
 */
bool CSCanExecute(CalculatorStack *cs, CSOperation op);

/**
 * Wykonuje operacje na stosie.
 * Próba wykonania operacji, która nie może zostać wykonana, może zabić program <c>assert</c>em. Wielomianowe argumenty
 * do operacji są brane z wierzchołka stosu. Dodatkowe argumenty dla operacji <c>AT</c> oraz <c>DEG_BY</c> powinny być
 * zainicjowane przy użyciu odpowiednio <c>CSSetPCArg()</c> oraz <c>CSSetPEArg()</c>.
 * @param cs stosk klakulatora
 * @param op kod operacji
 * @param out plik wyjściowy, potrzebny operacjom wypisującym dane (<c>OPERATION_IS_ZERO</c>, <c>OPERATION_IS_COEFF</c>,
 * <c>OPERATION_IS_EQ</c>, <c>OPERATION_DEG</c>, <c>OPERATION_DEG_BY</c>, <c>OPERATION_PRINT</c>
 */
void CSExecute(CalculatorStack *cs, CSOperation op, FILE *out);

/**
 * Ustawia dodatkowy argument dla wszystkich kolejnych operacji <c>OPERATION_DEG_BY</c> oraz <c>OPERATION_COMPOSE</c>.
 * Wszystkie te operacje będą używały tego argumentu, az do kolejnego wywołania tej metody z inną wartoscią.
 * @param cs struktura stosu
 * @param arg wartosć argumentu
 */
static inline void CSSetUIArg(CalculatorStack *cs, unsigned int arg)
{
    cs->uiArg = arg;
}

/**
 * Ustawia argument dla wszystkich kolejnych operacji <c>OPERATION_AT</c>.
 * Wszystkie te operacje będą używały tego argumentu, az do kolejnego wywołania tej metody z inną wartoscią.
 * @param cs struktura stosu
 * @param arg wartosć argumentu
 */
static inline void CSSetPCArg(CalculatorStack *cs, poly_coeff_t arg)
{
    cs->pcArg = arg;
}


#endif //WIELOMIANY_CALCULATOR_STACK_H
