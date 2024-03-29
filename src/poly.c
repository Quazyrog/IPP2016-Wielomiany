/**
 * @file poly.c
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "poly.h"
#include "mock_tricks.h"

/**
 * Żeby działało na overflow testach
 *
 * Gdy nie uwzglądniamy overflowów, to można załoąyć, że niektóre operacje po prostu nie mogą dać wielomianów zerowych,
 * i można tego nie sprawdzać. Overflowy naruszają to założenie i mogą w nieoczekiwanym momencie dać zerowy wielomian.
 * Logika testów nakazuje wtedy uprościć ten wielomian do zerowego i zoverflowowany wielomian stanie się poprawnym
 * wielomianem, czego testy z core nie uwzględniały.
 */
#define WILL_RUN_ILL_TESTS


/**
 * Upraszcza wielomian, jeśli ten jest zerowy i zwraca ten wielomian (uproszczony wielomian, nie uproszczoną kopię).
 * @param p wielomian do uproszczenia
 * @return <code>p</code> po uptrzednim uproszczeniu
 */
static Poly PolySimplifyCoeff(Poly p)
{
    if (p.monos != NULL) {
        bool first_zero = PolyIsZero(&p.monos[0].p);
        bool other_zeros = true;
        for (poly_exp_t i = 1; i < p.length && other_zeros; ++i)
            other_zeros &= PolyIsZero(&p.monos[i].p);
        if (first_zero && other_zeros) {
            free(p.monos);
            p.monos = NULL;
            p.asCoef = 0;
        } else if (other_zeros && p.monos[0].exp == 0 && PolyIsCoeff(&p.monos[0].p)) {
            Poly nested = p.monos[0].p;
            free(p.monos);
            return nested;
        }
    }
    return p;
}


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
    assert(q->monos == NULL && p->monos != NULL);
    if (q->asCoef == 0)
        return PolyClone(p);

    Poly result;
    if (p->monos[0].exp == 0) {
        //Trzeba zsumować
        result.length = p->length;
        result.monos = malloc(sizeof(Mono) * result.length);
        assert(result.monos != NULL);
        result.monos[0] = (Mono){.p = PolyAdd(q, &p->monos[0].p), .exp = 0};
        for (poly_exp_t i = 1; i < p->length; ++i)
            result.monos[i] = MonoClone(p->monos + i);
    } else {
        //Nie sumujemy
        result.length = p->length + 1;
        result.monos = malloc(sizeof(Mono) * result.length);
        assert(result.monos != NULL);
        result.monos[0] = (Mono){.p = PolyClone(q), .exp = 0};
        for (poly_exp_t i = 0; i < p->length; ++i)
            result.monos[1 + i] = MonoClone(p->monos + i);
    }

#ifdef WILL_RUN_ILL_TESTS
    return PolySimplifyCoeff(result);
#else
    return result;
#endif
}


/**
 * Oblicz \f$ p + q \f$ zakładając, że żąden z nich nie jest współczynnikiem.
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
    assert(result.monos != NULL);

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

    return PolySimplifyCoeff(result);
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


void PolyScaleInplace(Poly *p, poly_coeff_t scalar)
{
    if (scalar == 0) {
        free(p->monos);
        p->monos = NULL;
        p->asCoef = 0;
    } else if (PolyIsCoeff(p)) {
        p->asCoef *= scalar;
    } else {
        for (poly_exp_t i = 0; i < p->length; ++i)
            PolyScaleInplace(&p->monos[i].p, scalar);
    }
}


/**
 * Zwraca wielomian-nie-współczynnik pomnożony przez jednomian.
 * @param p wielomian; nie może być współczynnikiem
 * @param q jednomian
 * @return \f$ q * q \f$
 */
static Poly PolyMulM(const Poly *p, const Mono *q)
{
    assert(p->monos != NULL);
    if (PolyIsZero(&q->p))
        return PolyZero();

    Poly result;
    result.length = p->length;
    result.monos = malloc(sizeof(Mono) * result.length);
    assert(result.monos != NULL);

    //Trzeba ,,rozpakować'' wielomian p, bo inaczej zmniejszami indeksy zmiennych w q, a nie w p
    //i dzieją się dziwne rzeczy
    for (poly_exp_t i = 0; i < p->length; ++i) {
        result.monos[i].exp = q->exp + p->monos[i].exp;
        result.monos[i].p = PolyMul(&p->monos[i].p, &q->p);
    }

#ifdef WILL_RUN_ILL_TESTS
    return PolySimplifyCoeff(result);
#else
    return result;
#endif
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
        memcpy(out + i, in1 + j1, (len1 - j1) * sizeof(Mono));
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
    assert(sub1 != NULL);
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
 * Te nadmiarowe jednomiany będą znajdować się na końcu tablicy. Pole <c>length</c> struktury wielomianu będzie
 * zawierało liczbę tylko prawidłowych jednomianów w tablicy, więc może być mniejsza niż wejściowa długość tablicy
 * jednomianów.
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
        }
    }

    Poly p;
    p.monos = monos;
    p.length = i + 1;
    return PolySimplifyCoeff(p);
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
    if (PolyIsCoeff(q)) {
        Poly result = PolyClone(p);
        PolyScaleInplace(&result, q->asCoef);
#ifdef WILL_RUN_ILL_TESTS
        return PolySimplifyCoeff(result);
#else
        return result;
#endif
    }
    //W innek kolenjości się zapętli
    if (PolyIsCoeff(p))
        return PolyMul(q, p); //Także nie należy zapominać, że C nie jest funkcyjny: trzeba pisać return...

    Poly fold = PolyZero();
    for (poly_exp_t i = 0; i < q->length; ++i) {
        if (PolyIsZero(&q->monos[i].p))
            continue;
        Poly old_fold = fold;
        Poly next = PolyMulM(p, q->monos + i);
        fold = PolyAdd(&old_fold, &next);
        PolyDestroy(&old_fold);
        PolyDestroy(&next);
    }

    return PolySimplifyCoeff(fold);
}


Poly PolyAddMonos(unsigned count, const Mono *monos)
{
    Mono *m_copy = malloc(sizeof(Mono) * count);
    assert(m_copy != NULL);
    for (poly_exp_t i = 0; i < (poly_exp_t )count; ++i)
        m_copy[i] = monos[i];
    return PolyFromMonos(m_copy, count);
}


Poly PolyAddCopiedMonos(unsigned count, const Mono *monos)
{
    Mono *m_copy = malloc(sizeof(Mono) * count);
    assert(m_copy != NULL);
    for (poly_exp_t i = 0; i < (poly_exp_t )count; ++i)
        m_copy[i] = MonoClone(monos + i);
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
        assert(result.monos != NULL);
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
        assert(result.monos != NULL);
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
    return PolySimplifyCoeff(result);
}


poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx)
{
    if (PolyIsZero(p))
        return -1;
    if (PolyIsCoeff(p))
        return 0;

    if (var_idx == 0) {
        for (poly_exp_t i = p->length - 1; i >= 0; --i) {
            if (!PolyIsZero(&p->monos[i].p))
                return p->monos[i].exp;
        }
        assert(false);
    }

    poly_exp_t max = 0;
    for (poly_exp_t i = 0; i < p->length; ++i) {
        poly_exp_t ith_deg = PolyDegBy(&p->monos[i].p, var_idx - 1);
        max = max > ith_deg ? max : ith_deg;
    }
    return max;
}


poly_exp_t PolyDeg(const Poly *p)
{
    if (PolyIsZero(p))
        return -1;
    if (PolyIsCoeff(p))
        return 0;

    poly_exp_t max = 0;
    for (poly_exp_t i = 0; i < p->length; ++i) {
        if (PolyIsZero(&p->monos[i].p))
            continue;
        poly_exp_t ith_deg = PolyDeg(&p->monos[i].p) + p->monos[i].exp;
        max = max > ith_deg ? max : ith_deg;
    }

    return max;
}


/**
 * Porównuje wielomian is współczynnik.
 * Sprawdza czy wielomian <c>p</c>, który nie jest współczynnikiem, jest równy wielomianowi <c>q</c>,
 * który jest współczynnikiem.
 * @param p wielomian-nie-współczynnik
 * @param q wielomian-współczynnik
 * @return <c>true</c> gdy <c>p == q</c>, a <c>false</c> w przeciwnym wypadku
 */
static bool PolyIsEqPC(const Poly *p, const Poly *q)
{
    assert(p->monos != NULL);
    assert(q->monos == NULL);
    if (p->monos[0].exp != 0)
        return false;
    if (!PolyIsEq(&p->monos[0].p, q))
        return false;
    for (poly_exp_t i = 1; i < p->length; ++i) {
        if (!PolyIsZero(&p->monos[i].p))
            return false;
    }
    return true;
}


bool PolyIsEq(const Poly *p, const Poly *q)
{
    if (!PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyIsEqPC(p, q);
    if (PolyIsCoeff(p) && !PolyIsCoeff(q))
        return PolyIsEqPC(q, p);
    if (p->monos == NULL)
        return p->asCoef == q->asCoef;

    poly_exp_t i = 0, j = 0;
    while (i < p->length && j < q->length) {
        if (p->monos[i].exp < q->monos[j].exp) {
            if (!PolyIsZero(&p->monos[i].p))
                return false;
            ++i;
        } else if (p->monos[i].exp > q->monos[j].exp) {
            if (!PolyIsZero(&q->monos[j].p))
                return false;
            ++j;
        } else {
            if (!PolyIsEq(&p->monos[i].p, &q->monos[j].p))
                return false;
            ++i, ++j;
        }
    }

    for (; i < p->length; ++i) {
        if (!PolyIsZero(&p->monos[i].p))
            return false;
    }
    for (; j < q->length; ++j) {
        if (!PolyIsZero(&p->monos[j].p))
            return false;
    }

    return true;
}


Poly PolyAt(const Poly *p, poly_coeff_t x)
{
    if (PolyIsCoeff(p))
        return *p;
    Poly result = PolyZero();

    for (poly_exp_t i = 0; i < p->length; ++i) {
        Poly old_result = result;
        poly_coeff_t power = QuickPower(x, p->monos[i].exp);
        Poly evaluated_mono = PolyClone(&p->monos[i].p);
        PolyScaleInplace(&evaluated_mono, power);
        result = PolyAdd(&old_result, &evaluated_mono);

        PolyDestroy(&old_result);
        PolyDestroy(&evaluated_mono);
    }

    return result;
}


void PolyPrint(const Poly *p, FILE *stream)
{
    if (p->monos == NULL) {
        fprintf(stream, "%lli", (long long int) p->asCoef);
    } else {
        bool prepend_plus = false;
        for (poly_exp_t i = 0; i < p->length; ++i) {
            if (PolyIsZero(&p->monos[i].p))
                continue;
            if (prepend_plus)
                fputc('+', stream);
            fputc('(', stream);
            PolyPrint(&p->monos[i].p, stream);
            fputc(',', stream);
            fprintf(stream, "%lu", (long unsigned)p->monos[i].exp);
            fputc(')', stream);
            prepend_plus = true;
        }
    }
}


/**
 * Szybkie potęgowanie wielomianów.
 * Dla podanego wielomianu \f$ p \f$ zwraca \f$ p^\text{exp} \f$. Implementacja nie jest ogonowa, gdyż nie
 * i tak potrzebujemy dużo pamięci na wynik.
 * @param p potęgowany wielomian
 * @param exp wykładnik (człkowity, nieujemny)
 * @return wielomian \f$ p^\text{exp} \f$
 */
static Poly PolyQuickPower(const Poly *p, poly_exp_t exp)
{
    assert(exp >= 0);
    if (PolyIsZero(p))
        return PolyZero();
    if (exp == 0)
        return PolyFromCoeff(1);
    if (exp == 1)
        return PolyClone(p);

    Poly root = PolyQuickPower(p, exp / 2);
    Poly result;
    if (exp % 2 == 0) {
        result = PolyMul(&root, &root);
    } else {
        Poly half_result = PolyMul(&root, &root);
        result = PolyMul(&half_result, p);
        PolyDestroy(&half_result);
    }
    PolyDestroy(&root);
    return result;
}


/**
 * Zwraca wyraz wolny wielomianu.
 * Zwrócony wielomian jest współczynnikiem. Moze zostać zniszczony (wywołujący dostaje go na własność: wielomian
 * jest alokowany przez tę funkcję).
 * @param p odpytywany wielmian
 * @return wyraz wolny wielomianu \f$ p \f$
 */
static Poly ExactCoefficient(const Poly *p)
{
    if (PolyIsCoeff(p))
        return PolyClone(p);
    if (p->monos[0].exp == 0)
        return ExactCoefficient(&p->monos[0].p);
    return PolyZero();
}


Poly PolyCompose(const Poly *p, poly_exp_t vars_subs_count, const Poly *vars_subs)
{
    if (PolyIsCoeff(p))
        return PolyClone(p);
    if (vars_subs_count == 0)
        return ExactCoefficient(p);

    Poly result = PolyZero();
    for (poly_exp_t i = 0; i < p->length; ++i) {
        Poly composed_coef = PolyCompose(&p->monos[i].p, vars_subs_count - 1, vars_subs + 1);

        if (!PolyIsZero(&composed_coef)) {
            Poly cpow = PolyQuickPower(vars_subs + 0, p->monos[i].exp);
            Poly composed_mono = PolyMul(&cpow, &composed_coef);
            Poly new_result = PolyAdd(&result, &composed_mono);
            PolyDestroy(&cpow);
            PolyDestroy(&composed_mono);
            PolyDestroy(&result);
            result = new_result;
        }

        PolyDestroy(&composed_coef);
    }

    return result;
}
