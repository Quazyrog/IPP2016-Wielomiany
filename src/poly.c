/**
 * @file poly.c
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "poly.h"

/**
 * Oblicz liczbę jednomianów w \f$ p + q \f$
 * @param p pierwszy sumy wielomian
 * @param q drugi wielomian sumy wielomian
 * @return liczbę jednomianów w wynikowym wielomianie \f$ p + q \f$
 */
static inline poly_exp_t CountSumLength(const Poly *p, const Poly *q)
{
    assert(p->monos != NULL && q->monos != NULL);

    poly_exp_t sum_length = 0;
    poly_exp_t i = 0, j = 0;

    while (i < p->length && j < q->length) {
        if (p->monos[i].exp < q->monos[j].exp) {
            ++i;
        } else if (p->monos[i].exp > q->monos[j].exp) {
            ++j;
        } else {
            ++i, ++j;
        }
        ++sum_length;
    }

    return sum_length + (p->length - i) + (q->length - j);
}

/**
 * Oblicz \f$ p + q \f$ zakładając, że tylko <c>q</c> jest współczynnikiem.
 * To jest podprzypadek dodawania wielomianów, kiedy jeden z nich jest wspołczynnikiem, a drug nie.
 * @param p wielomian niebędący współczynnikiem
 * @param q wielomian będący współvczynnikiem
 * @return wielomian \f$ p + q \f$
 */
static inline Poly PolyAddPC(const Poly *p, const Poly *q)
{
    assert(q->monos == NULL);
    Poly result;

    //[p] trzeba przekopiować do wyniku: jeśli [p] ma współczynnik x^0, to kopiujemy bez przesunięcia;
    //jeśli nie, to z przesunięciem o 1, żeby dodac ten współczynnik
    int start_i = (p->monos[0].exp == 0) ? 0 : 1;
    result.monos = malloc(sizeof(Mono) * (p->length + start_i));

    result.monos[0] = (Mono){.p = PolyFromCoeff(0), .exp = 0};
    for (int i = 0; i < p->length; ++i)
        result.monos[start_i] = MonoClone(p->monos + start_i + i);

    Mono summed = {.p = PolyAdd(&result.monos[0].p, q), .exp = 0};
    MonoDestroy(&result.monos[0]);
    result.monos[0] = summed;

    return result;
}

/**
 * Oblicz \f$ p + q \f$ zakładając, że tylko żąden z nich nie jest współczynnikiem.
 * To jest podprzypadek dodawania wielomianów, kiedy żaden z nich nie jest wspołczynnikiem.
 * @param p wielomian niebędący współczynnikiem
 * @param q wielomian niebędący współczynnikiem
 * @return wielomian \f$ p + q \f$
 */
static inline Poly PolyAddPP(const Poly *p, const Poly *q)
{
    assert(p->monos != NULL);
    assert(q->monos != NULL);

    Poly result;
    result.length = CountSumLength(p, q);
    result.monos = malloc(sizeof(Mono) * result.length);

    poly_exp_t p_index = 0, q_index = 0, result_index = 0;
    while (p_index < p->length && q_index < q->length) {
        if (p->monos[p_index].exp < q->monos[q_index].exp) {
            result.monos[result_index] = MonoClone(p->monos + p_index);
            ++p_index;
        } else if (p->monos[p_index].exp > q->monos[q_index].exp) {
            result.monos[result_index] = MonoClone(q->monos + q_index);
            ++q_index;
        } else {
            result.monos[result_index] = (Mono){
                .p = PolyAdd(&p->monos[p_index].p, &q->monos[q_index].p),
                .exp = p->monos[p_index].exp
            };
            ++p_index;
            ++q_index;
        }
        ++result_index;
    }

    for (; p_index < p->length; ++p_index)
        result.monos[result_index++] = MonoClone(&p->monos[p_index]);
    for (; q_index < q->length; ++q_index)
        result.monos[result_index++] = MonoClone(&q->monos[q_index]);

    return result;
}

/**
 * Ogonowa implementacja szybkiego potęgowania dla wykładników większych od 0.
 * @param base baza potęgi
 * @param exponent wykładnik
 * @param accumulator akumulator
 * @return \f$ \text{acccumulator} \cdot \text{base}^\text{exponent} \f$
 */
static poly_coeff_t QuickPowerTail(poly_coeff_t base, poly_exp_t exponent, poly_coeff_t accumulator)
{
    if (exponent == 0)
        return accumulator;
    if (exponent % 2 == 0)
        return QuickPowerTail(base * base, exponent / 2, accumulator);
    return QuickPowerTail(base * base, exponent / 2, accumulator * base);
}

/**
 * Implementacja szybkiego potęgowania.
 * Ta funkcja wewnętrznie odwołuje się do QuickPowerTail(), po sprawdzeniu, czy <c>exponent >= 0</c>.
 * @param base baza potęgowania
 * @param exponent wykładnik
 * @return \f$ \text{base}^\text{exponent} \f$
 */
static inline poly_coeff_t QuickPower(poly_coeff_t base, poly_exp_t exponent)
{
    assert(exponent >= 0);
    return QuickPowerTail(base, exponent, 1);
}

/**
 * Mnoży wielomian przez \f$ x^n \f$.
 * Mnoży/przesówa wielomian o zadany wykładnik: dodaje <c>shift</c> do wysztskich jego wykładników. Jeżeli <c>p</c> jest
 * współczynnikiem, to zostanie przekształcony w wielomian o jednym monomianie.
 * @param p wielomian, który zostanie przesunięty
 * @param shift przesunięcie
 */
static void PolyShift(Poly *p, poly_exp_t shift)
{
    if (p->monos == NULL) {
        p->monos = malloc(sizeof(Mono));
        p->monos[0].p = PolyFromCoeff(p->asCoef);
        p->monos->exp = shift;
        p->length = 1;
    } else {
        for (poly_exp_t i = 0; i < p->length; ++i)
            p->monos[i].exp += shift;
    }
}

/**
 * Mnoży wielomian przez skalar.
 * @param p wielomian, który ma zostać pomnożony
 * @param scalar skalar
 */
static void PolyScale(Poly *p, poly_coeff_t scalar)
{
    if (PolyIsCoeff(p)) {
        p->asCoef *= scalar;
    } else {
        for (poly_exp_t i = 0; i < p->length; ++i)
            PolyScale(&p->monos[i].p, scalar);
    }
}

/**
 * Zwraca wielomian pomnożony przez jednomian.
 * @param p wielomian
 * @param q jednomian
 * @return \f$ q * q \f$
 */
static Poly PolyMulM(const Poly *p, const Mono *q)
{
    Poly result = PolyMul(p, &q->p);
    PolyShift(&result, q->exp);
    return result;
}

/**
 * Scal dwie tablice posortowanych jednomianów do w tablicy wynikowej.
 * Metoda zakłada, że <c>in1</c> jest na w swapowanej tablicy, a <c>in2</c> to sufiks tablicy <c>out</c>. Wynik scalenia
 * umieszczony będzie w tablicy <c>out</c>, a tablica <c>in1</c> zostanie nietknięta. Jednomiany o tym samym wykładniku
 * nie są scalane.
 * @param in1 pierwsza z posortowanych tablic; rozłączna z <c>out</c>
 * @param in2  druga z posortowanych tablic; sufiks tablicy <c>out</c>
 * @param out tablica, w której zostanie zapisany wynik
 * @param len1 długość tablicy <c>in1</c>
 * @param len2 długość tablicy <c>in2</c>
 */
static void PolyMergeSortedMonos(Mono *in1, Mono *in2, Mono *out, int len1, int len2)
{
    poly_exp_t i = 0, j1 = 0, j2 = 0;
    while (j1 < len1 && j2 < len2) {
        Mono m;
        if (in1[j1].exp < in2[j2].exp) {
            m = in1[j1];
            ++j1;
        } else {
            //Może być więcej niż dwa o równym stopniu, więc nie ma sensu teraz zajmować się równością stopni
            m = in2[j2];
            ++j2;
        }
        out[i++] = m;
    }

    if (j1 < len1)
        memcpy(out + i, in1 + j1, (size_t)(len1 - j1));
    //Zgodnie z założeniami, jeśli in2 się nie skończyło, to i tak jest już na swoim miejscu
}

/**
 * Sortuje tablicę jednomianów niemalejąco według ich wykładnika.
 * Jednomiany o tych samych wykładnikach nie są scalane.
 * @param monos tablica jednomianów do posortowania
 * @param length długość tablicy <c>monos</c>
 */
static void PolySortMonos(Mono *monos, poly_exp_t length)
{
    if (length == 1)
        return;

    poly_exp_t len1 = length / 2;
    poly_exp_t len2 = length - len1;
    Mono *sub1 = malloc(sizeof(Mono) * len1);
    Mono *sub2 = monos + len1;

    memcpy(sub1, monos, sizeof(Mono) * len1);
    PolySortMonos(sub1, len1);
    PolySortMonos(monos + len1, len2);

    PolyMergeSortedMonos(sub1, sub2, monos, len1, len2);
    free(sub1);
}

/**
 * Tworzy wielomian zbudowany z sumy jednomianów podanych w tablicy.
 * Wielomian przejmuje na własność tablicę jednomianów i staje się ona jego wewnętrzną tablicą. W związku z tym, tablica
 * nowego wielomianu będzie za duża, kiedy w wejściowej tablicy <c>monos</c> będą jednomiany o takim samym wykładniku.
 * Te nadmiarowe jednomiany będą znajdować się na końcu tablicy, ich wykładnik zostanie ustawiony na -1. Pole
 * <c>length</c> struktury wielomianu będzie zawierało liczbę tylko prawidłowych jednomianów w tablicy, więc może być
 * mniejsza niż wejściowa długość tablicy jednomianów.
 * @param monos tablica jednomianów (przejmowana na własność)
 * @param length długosć tablicy <c>monos</c>
 * @return
 */
static Poly PolyFromMonos(Mono *monos, poly_exp_t length)
{
    PolySortMonos(monos, length);

    int i = 0;
    for (int j = 1; j < length; ++j) {
        if (monos[i].exp == monos[j].exp) {
            Poly p = PolyAdd(&monos[i].p, &monos[j].p);
            PolyDestroy(&monos[i].p);
            PolyDestroy(&monos[j].p);
            monos[i].p = p;
        } else {
            ++i;
            monos[i] = monos[j];
            monos[j].exp = -1;
        }
    }

    Poly p;
    p.monos = monos;
    p.length = i;
    return p;
}

void PolyDestroy(Poly *p)
{
    if (p->monos != NULL) {
        for (poly_exp_t i = 0; i < p->length; ++i)
            MonoDestroy(p->monos + i);
        free(p->monos);
    }
}

Poly PolyAdd(const Poly *p, const Poly *q)
{
    if (p->monos == NULL && q->monos == NULL) {
        Poly result;
        result.monos = NULL;
        result.asCoef = p->asCoef + q->asCoef;
        return result;
    }

    if (q->monos == NULL)
        return PolyAddPC(p, q);
    if (p->monos == NULL)
        return PolyAddPC(q, p);
    return PolyAddPP(p, q);
}

Poly PolyMul(const Poly *p, const Poly *q)
{
    if (PolyIsCoeff(p))
        PolyMul(q, p);
    if (PolyIsCoeff(q)) {
        Poly result = PolyClone(p);
        PolyScale(&result, q->asCoef);
        return result;
    }

    Poly fold = PolyZero();
    for (poly_exp_t i = 0; i < q->length; ++i) {
        Poly old_fold = fold;
        Poly next = PolyMulM(p, q->monos + i);
        fold = PolyAdd(&old_fold, &next);
        PolyDestroy(&old_fold);
    }

    return fold;
}

Poly PolyAddMonos(unsigned count, const Mono *monos)
{
    Mono *m_copy = malloc(sizeof(Mono) * count);
    memcpy(m_copy, monos, sizeof(Mono) * count);
    ///FIXME skoro przejmuje na własność to chyba nie może być const? Powinienem jeszcze pamięć zwolnić?
    return PolyFromMonos(m_copy, count);
}

Poly PolyClone(const Poly *p)
{
    Poly result;
    if (PolyIsCoeff(p)) {
        result.monos = NULL;
        result.asCoef = p->asCoef;
    } else {
        result.length = p->length;
        result.monos = malloc(sizeof(Mono) * result.length);
        for (poly_exp_t i = 0; i < result.length; ++i)
            result.monos[i] = MonoClone(&p->monos[i]);
    }
    return result;
}

Poly PolyNeg(const Poly *p)
{
    Poly result;
    if (PolyIsCoeff(p)) {
        result.monos = NULL;
        result.asCoef = -p->asCoef;
    } else {
        result.monos = malloc(sizeof(Mono) * p->length);
        result.length = p->length;
        for (poly_exp_t i = 0; i < result.length; ++i) {
            result.monos[i].exp = p->monos[i].exp;
            result.monos[i].p = PolyNeg(&p->monos[i].p);
        }
    }
    return result;
}

Poly PolySub(const Poly *p, const Poly *q)
{
    Poly q_neg = PolyNeg(q);
    Poly result = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    return result;
}
