/** @file
   Interfejs klasy wielomianów

   @author Jakub Pawlewicz <pan\@mimuw.edu.pl>, Wojciech Matusiak <wm382710\@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017
*/

#ifndef __POLY_H__
#define __POLY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/** Typ współczynników wielomianu */
typedef long poly_coeff_t;

/** Typ wykładników wielomianu */
typedef int poly_exp_t;

/**
 * Struktura przechowująca wielomian
 *
 * Biblioteka nie alokuje pamięci bezpośrednio na instancje jej struktury, tylko na jej dane w razie ptrzeby. W związku
 * z tym wszystkie funkcje usuwające ją z pamięci usuną jedynie jej dane.
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
 * Robi pełną, głęboką kopię wielomianu.
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
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(unsigned count, const Mono monos[]);

/**
 * Wersja PolyAddMonos(), która kopiuje dogłębnie tablicę monos
 * Sumuje listę jednomianów i tworzy z nich wielomian. Nie przejmuje na własność zawartości tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddCopiedMonos(unsigned count, const Mono *monos);

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
 * Mnoży wielomian przez skalar.
 * Mnożenie wielomiianu odbywa się w miejscu: mnożony wielomian nie jest kopiowany, tylko sam mnożony.
 * @param p wielomian, który ma zostać pomnożony
 * @param scalar skalar
 */
void PolyScaleInplace(Poly *p, poly_coeff_t scalar);

/**
 * Zwraca wielomian \f$ p \f$ po serii podstawień w postaci \f$ x_i = \text{vars_subs[i]} \f$.
 * Jeśli liczba elementów <c>vars_subs</c> jest mniejsza od liczby zmiennych wielomianu, to zmienne o indeksach
 * większych lub równych liczbie zdefiniowanych podstawień są zamieniane na 0.
 * @param p wielomian, w którym podstawiamy zmienne
 * @param vars_subs_count liczba elementów tablicy <c>vars_subs</c>
 * @param tablica z podstawieniami dla kolejnych zmiennych
 * @return wielomian \f$ p \left( \text{vars_subs[0]} \right) \dots \left( \text{vars_subs[vars_subs_count]} \right)
 * \left( 0 \right) \dots \f$
 */
Poly PolyCompose(const Poly *p, poly_exp_t vars_subs_count, const Poly *vars_subs);

/**
 * Wypisuje wielomian do podanego w argumencie strumienia.
 * Wypisany wielomian jest zgodny ze specyfikacją zadania, tj:
 *   - Wielomian-współczynnik jest wypisany jako liczba
 *   - Jednomian <c>m</c> jest wypisany jako <c>(m.p,m.exp)</c>: wykładnik jako liczba, współczynnik jako wielomian
 *   – Jednomiany (jeśli więcej niż jeden) są oddzielone znakiem '+'
 *   – Jednomiany o zerowym współczynniku sa pomijane
 *   — Złamanie linii NIE jest dodawane
 * @param p wielomian do wypisania
 * @param stream strumień wyjściowy
 */
void PolyPrint(const Poly *p, FILE *stream);

/**
 * Robi pełną, głęboką kopię jednomianu.
 * @param[in] m : jednomian
 * @return skopiowany jednomian
 */
static inline Mono MonoClone(const Mono *m)
{
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
static inline Poly PolyZero()
{
    return PolyFromCoeff(0);
}

/**
 * Tworzy jednomian `p * x^e`.
 * Przejmuje na własność zawartość struktury wskazywanej przez @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] e : wykładnik
 * @return jednomian `p * x^e`
 */
static inline Mono MonoFromPoly(Poly *p, poly_exp_t e)
{
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
static inline bool PolyIsZero(const Poly *p)
{
    if (p->monos == NULL)
        return p->asCoef == 0;
    return false;
}

/**
 * Usuwa jednomian z pamięci.
 * @param[in] m : jednomian
 */
static inline void MonoDestroy(Mono *m)
{
    PolyDestroy(&m->p);
}

#endif /* __POLY_H__ */
