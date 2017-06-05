/** @file units_tests_poly_compose.c
 * Testy jednostkowe dla funkcji PolyCompose
 */
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>
#include "poly_compose.h"
#include "../poly.h"

void TestPolyComposeZeroZero(void **state)
{
    (void)state;

    Poly p = PolyZero();
    Poly expect = PolyZero();

    Poly got = PolyCompose(&p, 0, NULL);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
}


void TestPolyComposeZeroConst(void **state)
{
    (void)state;

    Poly p = PolyZero();
    Poly expect = PolyZero();
    Poly x[] = {PolyZero()};

    Poly got = PolyCompose(&p, 1, x);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
    PolyDestroy(x + 0);
}


void TestPolyComposeConstZero(void **state)
{
    (void)state;

    Poly p = PolyFromCoeff(42);
    Poly expect = PolyFromCoeff(42);

    Poly got = PolyCompose(&p, 0, NULL);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
}


void TestPolyComposeConstConst(void **state)
{
    (void)state;

    Poly p = PolyFromCoeff(42);
    Poly expect = PolyFromCoeff(42);
    Poly x[] = {PolyFromCoeff(44)};

    Poly got = PolyCompose(&p, 1, x);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
    PolyDestroy(x + 0);
}


/**
 * Tworzy nowy wielomian liniowy \f$ x_0 \f$
 * @return
 */
static Poly MakeLinear()
{
    Poly one = PolyFromCoeff(1);
    Mono m = MonoFromPoly(&one, 1);
    return PolyAddMonos(1, &m);
}


void TestPolyComposeLinZero(void **state)
{
    (void)state;

    Poly p = MakeLinear();
    Poly expect = PolyZero();

    Poly got = PolyCompose(&p, 0, NULL);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
}


void TestPolyComposeLinConst(void **state)
{
    (void)state;

    Poly p = MakeLinear();
    Poly expect = PolyFromCoeff(42);
    Poly x[] = {PolyFromCoeff(42)};

    Poly got = PolyCompose(&p, 1, x);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
    PolyDestroy(x + 0);
}


void TestPolyComposeLinLin(void **state)
{
    (void)state;

    Poly p = MakeLinear();
    Poly expect = MakeLinear();
    Poly x[] = {MakeLinear()};

    Poly got = PolyCompose(&p, 1, x);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
    PolyDestroy(x + 0);
}
