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
}

int main()
{
    TestPolynomialBuilding();
    TestPolyAdd();
}
