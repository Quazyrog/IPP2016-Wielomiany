/** @file
   Interfejs klasy wielomianów

   @author Jakub Pawlewicz <pan@mimuw.edu.pl>, TODO
   @copyright Uniwersytet Warszawski
   @date 2017-04-09, TODO
*/

#ifndef __POLY_H__
#define __POLY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Typ współczynników wielomianu */
typedef long poly_coeff_t;

/** Typ wykładników wielomianu */
typedef int poly_exp_t;

/**
 * Struktura przechowująca wielomian
 *
 * Biblioteka nie alokuje pamięci bezpośrednio na instancje jej struktury, tylko na jej dane w razie ptrzeby. W związku
 * z tym wszystkie funkcje usuwające ją z pamięci usuną jedynie jej dane (pomimo tego, że wymagają wskaźnika na tę
 * strukturę, co na pozór może się wydawać lekką niespójnością w interfejsie).
 */
typedef struct Poly
{
    ///Wskaźnik na jednomiany wielomianu; <c>NULL</c>, kiedy wielomian jest stały ze względu na wszystkie zmienne
    ///(aka ,,jest współczynnikiem'').
    struct Mono *monos;
    union
    {
        ///Zawiera prawidłowe dane, tylko gdy <c>monos == NULL</c>. Wówczas zawiera stałą wartość wielomianu
        poly_coeff_t asCoef;

        ///Zawiera prawidłowe dane, tylko gdy <c>monos != NULL</c>. Wówczas zawiera liczbę jednomianów w wielomianie
        poly_exp_t length;
    };
} Poly;

/**
  * Struktura przechowująca jednomian
  * Jednomian ma postać `p * x^e`.
  * Współczynnik `p` może też być wielomianem.
  * Będzie on traktowany jako wielomian nad kolejną zmienną (nie nad x).
  */
typedef struct Mono
{
    Poly p; ///< współczynnik
    poly_exp_t exp; ///< wykładnik
} Mono;

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p);

/**
 * Robi pełną kopię wielomianu.
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p);

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p + q`
 */
Poly PolyAdd(const Poly *p, const Poly *q);

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(unsigned count, const Mono monos[]);

/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p * q`
 */
Poly PolyMul(const Poly *p, const Poly *q);

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian
 * @return `-p`
 */
Poly PolyNeg(const Poly *p);

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p - q`
 */
Poly PolySub(const Poly *p, const Poly *q);

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru).
 * Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx);

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p);

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p = q`
 */
bool PolyIsEq(const Poly *p, const Poly *q);

/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianem
 * i zmniejszane są indeksy zmiennych w takim wielomianie o jeden.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p
 * @param[in] x
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x);

/**
 * Robi pełną kopię jednomianu.
 * @param[in] m : jednomian
 * @return skopiowany jednomian
 */
static inline Mono MonoClone(const Mono *m) {
    Mono clone;
    clone.exp = m->exp;
    clone.p = PolyClone(&m->p);
    return clone;
}

/**
 * Tworzy wielomian, który jest współczynnikiem.
 * @param[in] c : wartość współczynnika
 * @return wielomian
 */
static inline Poly PolyFromCoeff(poly_coeff_t c)
{
    Poly p; //Polip
    p.monos = NULL;
    p.asCoef = c;
    return p;
}

/**
 * Tworzy wielomian tożsamościowo równy zeru.
 * @return wielomian
 */
static inline Poly PolyZero() {
    return PolyFromCoeff(0);
}

/**
 * Tworzy jednomian `p * x^e`.
 * Tworzony jednomian przejmuje na własność (kopiuje) wielomian @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] e : wykładnik
 * @return jednomian `p * x^e`
 */
static inline Mono MonoFromPoly(Poly *p, poly_exp_t e) {
    //TODO Kopiować, czy nie kopiować?
    Mono m;
    m.exp = e;
    m.p = *p;
    return m;
}

/**
 * Sprawdza, czy wielomian jest współczynnikiem.
 * @param[in] p : wielomian
 * @return Czy wielomian jest współczynnikiem?
 */
static inline bool PolyIsCoeff(const Poly *p)
{
    return p->monos == NULL;
}

/**
 * Sprawdza, czy wielomian jest tożsamościowo równy zeru.
 * @param[in] p : wielomian
 * @return Czy wielomian jest równy zero?
 */
static inline bool PolyIsZero(const Poly *p) {
    return p->monos == NULL && p->asCoef == 0;
}

/**
 * Usuwa jednomian z pamięci.
 * @param[in] m : jednomian
 */
static inline void MonoDestroy(Mono *m) {
    PolyDestroy(&m->p);
}

#endif /* __POLY_H__ */
