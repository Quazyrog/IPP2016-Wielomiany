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

int main()
{
    TestPolynomialBuilding();
}
