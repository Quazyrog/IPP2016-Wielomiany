#include <assert.h>
#include "poly.h"


void TestPolynomialBuilding(void)
{
    Poly zero = PolyZero();
    assert(zero.asCoef == 0);
    assert(zero.monos == NULL);

    //1 + 3x + 3x^2 + x^3
    Mono monos[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
        (Mono){.p = PolyFromCoeff(3), .exp = 1},
        (Mono){.p = PolyFromCoeff(3), .exp = 2},
        (Mono){.p = PolyFromCoeff(1), .exp = 3},
    };
    Poly summed_poly = PolyAddMonos(4, monos);
    assert(summed_poly.length == 4);
    assert(summed_poly.monos[0].exp == 0);
    assert(summed_poly.monos[1].p.asCoef == 3);
    assert(summed_poly.monos[2].exp == 2);
    assert(summed_poly.monos[3].p.asCoef == 1);
    PolyDestroy(&summed_poly);

    //1 + 3x + 3x^2 + x^3 (permuted)
    Mono monos2[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 3},
        (Mono){.p = PolyFromCoeff(3), .exp = 1},
        (Mono){.p = PolyFromCoeff(3), .exp = 2},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
    };
    Poly summed_poly2 = PolyAddMonos(4, monos2);
    assert(summed_poly2.length == 4);
    assert(summed_poly2.monos[0].exp == 0);
    assert(summed_poly2.monos[1].p.asCoef == 3);
    assert(summed_poly2.monos[2].exp == 2);
    assert(summed_poly2.monos[3].p.asCoef == 1);
    PolyDestroy(&summed_poly2);

    //1 + 3x + 3x^2 + x^3 (permuted + needs summation)
    Mono monos3[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 3},
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 2},
        (Mono){.p = PolyFromCoeff(2), .exp = 2},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
        (Mono){.p = PolyFromCoeff(0), .exp = 1},
        (Mono){.p = PolyFromCoeff(0), .exp = 3},
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
    };
    Poly summed_poly3 = PolyAddMonos(9, monos3);
    assert(summed_poly3.length == 4);
    assert(summed_poly3.monos[0].exp == 0);
    assert(summed_poly3.monos[1].p.asCoef == 3);
    assert(summed_poly3.monos[2].exp == 2);
    assert(summed_poly3.monos[3].p.asCoef == 1);
    PolyDestroy(&summed_poly3);
}

void TestPolyAdd(void)
{
    //1 + 3x + 3x^2 + x^3
    Mono monos[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
        (Mono){.p = PolyFromCoeff(3), .exp = 1},
        (Mono){.p = PolyFromCoeff(3), .exp = 2},
        (Mono){.p = PolyFromCoeff(1), .exp = 3},
    };
    Poly summed_poly = PolyAddMonos(4, monos);

    //2 + 6x + 6x^2 + 2x^3
    Poly times2 = PolyAdd(&summed_poly, &summed_poly);
    assert(times2.length == 4);
    assert(times2.monos[0].exp == 0);
    assert(times2.monos[1].p.asCoef == 6);
    assert(times2.monos[2].exp == 2);
    assert(times2.monos[3].p.asCoef == 2);

    //(1 + 3y + 3y^2 + y^3)x
    Mono m1x = MonoFromPoly(&summed_poly, 1);
    Poly p1x = PolyAddMonos(1, &m1x);
    assert(p1x.length == 1);
    assert(p1x.monos[0].p.length == 4);
    assert(p1x.monos[0].p.monos[1].exp == 1);
    assert(p1x.monos[0].p.monos[1].p.asCoef == 3);
    assert(p1x.monos[0].p.monos[2].exp == 2);
    assert(p1x.monos[0].p.monos[2].p.asCoef == 3);
    assert(p1x.monos[0].p.monos[3].exp == 3);
    assert(p1x.monos[0].p.monos[3].p.asCoef == 1);

    //4 + (2 + 6y + 6y^2 + 2y^3)x + 2x
    Poly m20x_no_p = PolyFromCoeff(4);
    Mono m20x = MonoFromPoly(&m20x_no_p, 0);
    Mono m21x = MonoFromPoly(&times2, 1);
    Poly m22x_no_p = PolyFromCoeff(2);
    Mono m22x = MonoFromPoly(&m22x_no_p, 2);
    Mono mmmm[] = {m20x, m21x, m22x};
    Poly just_too_complex_to_name_it = PolyAddMonos(3, mmmm);
    assert(just_too_complex_to_name_it.length == 3);
    assert(just_too_complex_to_name_it.monos[0].p.asCoef == 4);
    assert(just_too_complex_to_name_it.monos[2].p.asCoef == 2);
    assert(just_too_complex_to_name_it.monos[1].p.length == 4);
    assert(just_too_complex_to_name_it.monos[1].p.monos[0].exp == 0);
    assert(just_too_complex_to_name_it.monos[1].p.monos[1].p.asCoef == 6);
    assert(just_too_complex_to_name_it.monos[1].p.monos[3].exp == 3);
    assert(just_too_complex_to_name_it.monos[1].p.monos[3].p.asCoef == 2);

    //4 + (3 + 9y + 9y^2 + 3y^3)x + 2x
    Poly delirium = PolyAdd(&p1x, &just_too_complex_to_name_it);
    assert(delirium.length == 3);
    assert(delirium.monos[0].p.asCoef == 4);
    assert(delirium.monos[2].p.asCoef == 2);
    assert(delirium.monos[1].p.length == 4);
    assert(delirium.monos[1].p.monos[0].exp == 0);
    assert(delirium.monos[1].p.monos[1].p.asCoef == 9);
    assert(delirium.monos[1].p.monos[3].exp == 3);
    assert(delirium.monos[1].p.monos[3].p.asCoef == 3);

    PolyDestroy(&delirium);
    PolyDestroy(&just_too_complex_to_name_it);
    PolyDestroy(&p1x);

    //Przetestuj dodawanie wielomianu do współczynnika, bo nie działało
    //x + 1   &   1
    Mono monos1[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
    };
    Poly poly_add_pc_p = PolyAddMonos(2, monos1);
    Poly poly_add_pc_c = PolyFromCoeff(1);
    Poly sum = PolyAdd(&poly_add_pc_c, &poly_add_pc_p);
    assert(sum.length == 2);
    assert(sum.monos[0].p.asCoef == 2);  assert(sum.monos[0].exp == 0);
    assert(sum.monos[1].p.asCoef == 1);  assert(sum.monos[1].exp == 1);
    PolyDestroy(&poly_add_pc_c);
    PolyDestroy(&poly_add_pc_p);
    PolyDestroy(&sum);
    PolyDestroy(&summed_poly);
    PolyDestroy(&times2);
}

//Sprawdza, czy wielomian jest równy Ax^2 +2Ax + A
void TestPolyMultiplyAssertHelper1(Poly p, poly_coeff_t a)
{
    assert(p.length == 3);
    assert(p.monos[0].exp == 0);
    assert(p.monos[0].p.asCoef == a);
    assert(p.monos[1].exp == 1);
    assert(p.monos[1].p.asCoef == 2 * a);
    assert(p.monos[2].exp == 2);
    assert(p.monos[2].p.asCoef == a);
}

void TestPolyMultiply(void)
{
    //x + 1
    Mono monos1[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
    };
    Poly poly1p1 = PolyAddMonos(2, monos1);
    Poly poly1p2 = PolyMul(&poly1p1, &poly1p1);
    Poly poly1p3 = PolyMul(&poly1p2, &poly1p1);
    Poly poly1p5 = PolyMul(&poly1p3, &poly1p2);
    assert(poly1p5.length == 6);
    assert(poly1p5.monos[0].p.asCoef == 1);   assert(poly1p5.monos[0].exp == 0);
    assert(poly1p5.monos[1].p.asCoef == 5);   assert(poly1p5.monos[1].exp == 1);
    assert(poly1p5.monos[2].p.asCoef == 10);  assert(poly1p5.monos[2].exp == 2);
    assert(poly1p5.monos[3].p.asCoef == 10);  assert(poly1p5.monos[3].exp == 3);
    assert(poly1p5.monos[4].p.asCoef == 5);   assert(poly1p5.monos[4].exp == 4);
    assert(poly1p5.monos[5].p.asCoef == 1);   assert(poly1p5.monos[5].exp == 5);
    PolyDestroy(&poly1p1);
    PolyDestroy(&poly1p2);
    PolyDestroy(&poly1p3);
    PolyDestroy(&poly1p5);

    //(y + 1)(x + 1) = (y + 1)x + (y + 1)
    Mono monos21[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
    };
    Poly poly21 = PolyAddMonos(2, monos21);
    Mono monos221[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
    };
    Mono monos22 = (Mono){.p = PolyAddMonos(2, monos221), .exp = 0};
    Poly poly22 = PolyAddMonos(1, &monos22);
    MonoDestroy(&monos22);
    Poly poly2 = PolyMul(&poly21, &poly22);
    assert(poly2.length == 2);
    assert(poly2.monos[0].exp == 0);
    assert(poly2.monos[0].p.length == 2);
    assert(poly2.monos[0].p.monos[0].exp == 0);
    assert(poly2.monos[0].p.monos[0].p.asCoef == 1);
    assert(poly2.monos[0].p.monos[1].exp == 1);
    assert(poly2.monos[0].p.monos[1].p.asCoef == 1);
    assert(poly2.monos[1].exp == 1);
    assert(poly2.monos[1].p.monos[0].exp == 0);
    assert(poly2.monos[1].p.monos[0].p.asCoef == 1);
    assert(poly2.monos[1].p.monos[1].exp == 1);
    assert(poly2.monos[1].p.monos[1].p.asCoef == 1);
    PolyDestroy(&poly21);
    PolyDestroy(&poly22);

    //((y + 1)x + (y + 1))^2 = (y^2 + 2y + 1)x^2 + (2y^2 + 4y + 2)x + (y^2 + 2y + 1)
    Poly poly3 = PolyMul(&poly2, &poly2);
    assert(poly3.length == 3);
    assert(poly3.monos[0].exp == 0);
    TestPolyMultiplyAssertHelper1(poly3.monos[0].p, 1);
    assert(poly3.monos[1].exp == 1);
    TestPolyMultiplyAssertHelper1(poly3.monos[1].p, 2);
    assert(poly3.monos[2].exp == 2);
    TestPolyMultiplyAssertHelper1(poly3.monos[2].p, 1);

    //((y^2 + 2y + 1)x^2 + (2y^2 + 4y + 2)x + (y^2 + 2y + 1)) * (x + 1)
    //= (y^2 + 2y + 1)x^3 + (3y^2 + 6y + 3)x^2 + (3y^2 + 6y + 3)x + (y^2 + 2y + 1)
    Mono monos4[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 1},
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
    };
    Poly poly4factor = PolyAddMonos(2, monos4);
    Poly poly4 = PolyMul(&poly3, &poly4factor);
    assert(poly4.length == 4);
    assert(poly4.monos[0].exp == 0);
    TestPolyMultiplyAssertHelper1(poly4.monos[0].p, 1);
    assert(poly4.monos[1].exp == 1);
    TestPolyMultiplyAssertHelper1(poly4.monos[1].p, 3);
    assert(poly4.monos[2].exp == 2);
    TestPolyMultiplyAssertHelper1(poly4.monos[2].p, 3);
    assert(poly4.monos[3].exp == 3);
    TestPolyMultiplyAssertHelper1(poly4.monos[3].p, 1);
    PolyDestroy(&poly4factor);

    //poly4 * 42
    Poly poly5scalar = PolyFromCoeff(42);
    Poly poly5 = PolyMul(&poly4, &poly5scalar);
    assert(poly5.length == 4);
    assert(poly5.monos[0].exp == 0);
    TestPolyMultiplyAssertHelper1(poly5.monos[0].p, 1 * 42);
    assert(poly5.monos[1].exp == 1);
    TestPolyMultiplyAssertHelper1(poly5.monos[1].p, 3 * 42);
    assert(poly5.monos[2].exp == 2);
    TestPolyMultiplyAssertHelper1(poly5.monos[2].p, 3 * 42);
    assert(poly5.monos[3].exp == 3);
    TestPolyMultiplyAssertHelper1(poly5.monos[3].p, 1 * 42);
    PolyDestroy(&poly5scalar);

    PolyDestroy(&poly2);
    PolyDestroy(&poly3);
    PolyDestroy(&poly4);
    PolyDestroy(&poly5);
}

void TestEqualAndNeg(void)
{
    Mono some_monos[] = {
        (Mono){.p = PolyFromCoeff(1), .exp = 0},
        (Mono){.p = PolyFromCoeff(3), .exp = 1},
        (Mono){.p = PolyFromCoeff(3), .exp = 2},
        (Mono){.p = PolyFromCoeff(1), .exp = 3},
    };
    Poly some_poly = PolyAddMonos(4, some_monos);
    Mono some_monos_neg[] = {
        (Mono){.p = PolyFromCoeff(-1), .exp = 0},
        (Mono){.p = PolyFromCoeff(-3), .exp = 1},
        (Mono){.p = PolyFromCoeff(-3), .exp = 2},
        (Mono){.p = PolyFromCoeff(-1), .exp = 3},
    };
    Poly some_poly_neg = PolyAddMonos(4, some_monos_neg);
    Poly zero = PolyZero();

    Poly poly0 = PolyAdd(&some_poly, &some_poly_neg);
    assert(PolyIsZero(&poly0));
    assert(PolyIsEq(&zero, &poly0));
    PolyDestroy(&some_poly);
    PolyDestroy(&some_poly_neg);
    PolyDestroy(&poly0);

    //(x + y)^2 + (x - y)^2
    Mono m = (Mono){.p = PolyFromCoeff(1), 1};
    Poly poly_x = PolyAddMonos(1, &m);
    Poly poly_y_pre = PolyAddMonos(1, &m);
    Mono mono_y = MonoFromPoly(&poly_y_pre, 0);
    Poly poly_y = PolyAddMonos(1, &mono_y);
    MonoDestroy(&mono_y);
    Poly x_plus_y = PolyAdd(&poly_x, &poly_y);
    Poly x_subt_y = PolySub(&poly_x, &poly_y);
    assert(x_plus_y.length == 2);
    assert(x_plus_y.monos[0].p.length == 1);
    assert(x_plus_y.monos[0].p.monos[0].exp == 1);
    assert(x_plus_y.monos[0].p.monos[0].p.asCoef == 1);
    assert(x_plus_y.monos[1].exp == 1);
    assert(x_plus_y.monos[1].p.asCoef == 1);

    Poly sum_pow2 = PolyMul(&x_plus_y, &x_plus_y);
    Poly dif_pow2 = PolyMul(&x_subt_y, &x_subt_y);
    Poly sumA = PolyAdd(&sum_pow2, &dif_pow2);
    assert(sumA.monos[0].p.monos[0].p.asCoef == 2);
    assert(sumA.monos[0].p.monos[0].exp == 2);
    assert(sumA.monos[1].p.asCoef == 0);
    assert(sumA.monos[2].p.asCoef == 2);
    assert(sumA.monos[2].exp == 2);

    Poly sqrx = PolyMul(&poly_x, &poly_x);
    Poly sqry = PolyMul(&poly_y, &poly_y);
    Poly sqr_sum = PolyAdd(&sqrx, &sqry);
    Poly sqr_sum_dbl = PolyAdd(&sqr_sum, &sqr_sum);
    assert(sqr_sum_dbl.monos[0].p.monos[0].p.asCoef == 2);
    assert(sqr_sum_dbl.monos[0].p.monos[0].exp == 2);
    assert(sqr_sum_dbl.monos[1].p.asCoef == 2);
    assert(sqr_sum_dbl.monos[1].exp == 2);

    assert(PolyIsEq(&sqr_sum_dbl, &sumA));

    PolyDestroy(&poly_x);
    PolyDestroy(&poly_y);
    PolyDestroy(&x_plus_y);
    PolyDestroy(&sum_pow2);
    PolyDestroy(&dif_pow2);
    PolyDestroy(&sumA);
    PolyDestroy(&sqrx);
    PolyDestroy(&sqry);
    PolyDestroy(&sqr_sum);
    PolyDestroy(&sqr_sum_dbl);
    PolyDestroy(&x_subt_y);
}

int main()
{
    TestPolynomialBuilding();
    TestPolyAdd();
    TestPolyMultiply();
    TestEqualAndNeg();
}
