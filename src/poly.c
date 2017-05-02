/**
 * @file poly.c
 */
#include <stdlib.h>
#include <assert.h>
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

    result.monos[0] = {.p = PolyFromCoeff(0), .exp = 0};
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
            result.monos[result_index] = {
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
