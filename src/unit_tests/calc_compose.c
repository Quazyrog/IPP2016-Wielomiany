/** @file calc_compose.c
 * Testy całego programu pod kątem funkcji PolyCompose()
 */
#include <limits.h>
#include "calc_compose.h"
#include "tests_utils.h"


static void TestCore(const char *in, const char *expected_out, const char *expected_err)
{
    UnitTestingFeedInput(stdin, in, strlen(in));
    UnitTestingExpectOutput(stdout, expected_out);
    UnitTestingExpectOutput(stderr, expected_err);
    tested_main(1, (char*[]){"./calc_poly"});
    assert_true(UnitTestingCheckOutput(false));
}


void TestCalcComposeNoParam(void **state)
{
    (void)state;
    const char *in = "COMPOSE\n";
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


void TestCalcComposeZero(void **state)
{
    (void)state;
    const char *in = "(42,0)+(1,1)\nCOMPOSE 0\nPRINT\n";
    const char *expected_out = "42\n";
    const char *expected_err = "";
    TestCore(in, expected_out, expected_err);
}


void TestCalcComposeUMax(void **state)
{
    (void)state;
    char *in = malloc(256);
    snprintf(in, 256, "(42,0)+(1,1)\nCOMPOSE %lu\n", (long unsigned)UINT_MAX);
    const char *expected_out = "";
    const char *expected_err = "ERROR 2 STACK UNDERFLOW\n";
    TestCore(in, expected_out, expected_err);
    free(in);
}


void TestCalcComposeNegative(void **state)
{
    (void)state;
    char *in = "COMPOSE -1\n";
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


void TestCalcComposeOverflow(void **state)
{
    (void)state;
    char *in = malloc(256);
    snprintf(in, 256, "COMPOSE %lu\n", (long unsigned)UINT_MAX + 1);
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
    free(in);
}


void TestCalcComposeOooveeerflooow(void **state)
{
    (void)state;
    char *in = "COMPOSE 8364889373929365739284365876348912907120974358679243537901234097234689234\n"; //Powinno starczyć
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


void TestCalcComposeCapybara(void **state)
{
    (void)state;
    char *in = "COMPOSE kapibara\n"; //.-.
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


void TestCalcCompose44Capybaras(void **state)
{
    (void)state;
    //.-. .-. .-. .-. .-. .-. .-. .-. .-. .-. .-.
    //.-. .-. .-. .-. .-. .-. .-. .-. .-. .-. .-.
    //.-. .-. .-. .-. .-. .-. .-. .-. .-. .-. .-.
    //.-. .-. .-. .-. .-. .-. .-. .-. .-. .-. .-.
    char *in = "COMPOSE 44kapibary\n";
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


void TestCalcComposeExample(void **state)
{
    (void)state;
    const char *in = "(1,2)\n"
            "(2,0)+(1,1)\n"
            "COMPOSE 1\n"
            "PRINT\n"
            "(1,3)\n"
            "COMPOSE 1\n"
            "PRINT\n"
            "POP\n"
            "((1,0)+(1,1),1)\n"
            "(1,4)\n"
            "(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\n"
            "COMPOSE 2\n"
            "PRINT\n"
            "((1,0)+(1,1),1)\n"
            "(1,4)\n"
            "COMPOSE -1\n";
    const char *expected_out = "(2,0)+(1,2)\n"
            "(8,0)+(12,2)+(6,4)+(1,6)\n"
            "(1,12)+((1,0)+(2,1)+(1,2),14)+(5,28)\n";
    const char *expected_err = "ERROR 16 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}
