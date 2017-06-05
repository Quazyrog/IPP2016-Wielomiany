/** @file cmocka_tests.c
 * Testy jednostkowe z wykorzystaniem biblioteki CMocka
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>
#include "poly_compose.h"
#include "tests_utils.h"

int tested_main(int argc, char **argv);



/**
 * Testy atrap strumieni (czyli testowanie jednostkowe testów jednostkowych).
 * @param state
 */
static void TestMocks(void **state)
{
    (void)state;

    UnitTestingExpectOutput(stdout, "");
    mock_fprintf(stdout, "");
    mock_fprintf(stdout, "");
    mock_fprintf(stdout, "");
    assert_true(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "ABC");
    mock_fprintf(stdout, "ABC");
    assert_true(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "ABC");
    mock_fprintf(stdout, "A");
    mock_fprintf(stdout, "B");
    mock_fprintf(stdout, "C");
    assert_true(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "Hello World!\nSaid Dregon Reborn and then flew away.");
    mock_fprintf(stdout, "Hello");
    mock_fprintf(stdout, " World!\n");
    mock_fprintf(stdout, "Said %s and then flew away.", "Dregon Reborn");
    assert_true(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "");
    mock_fprintf(stdout, "assss");
    assert_false(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout,
                            "/home/saphir/Documents/MIMUW/2016-2017/IPP/cmocka_example/MyMock/tests.c:87:15: error: ‘long long long’ is too long for GCC");
    mock_fprintf(stdout, "/home/saphir/Documents/MIMUW/2016-2017/IPP/cmocka_example/MyMock/tests.c:87:15: error: ");
    mock_fprintf(stdout, "‘long long long’");
    mock_fprintf(stdout, " is too long for GCC\n");
    assert_false(UnitTestingCheckOutput(true));
}


/**
 * Testy atrap strumieni (czyli testowanie jednostkowe testów jednostkowych) -- ciąg dalszy.
 * @param state
 */
static void TestMocksManyStreams(void **state)
{
    (void)state;

    UnitTestingExpectOutput(stdout, "0\n(1,1)\n");
    UnitTestingExpectOutput(stderr, "ERROR 2 NO ERROR");
    mock_fprintf(stderr, "ERROR");
    mock_fprintf(stdout, "0");
    mock_fprintf(stderr, " 2 ");
    mock_fprintf(stdout, "\n(1,1)\n");
    mock_fprintf(stderr, "NO ERROR");
    assert_true(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "0\n(1,1)\n");
    UnitTestingExpectOutput(stderr, "ERROR 2 NO ERROR");
    mock_fprintf(stderr, "ERROR");
    mock_fprintf(stdout, "0");
    mock_fprintf(stderr, " 3 ");
    mock_fprintf(stdout, "\n(1,1)\n");
    mock_fprintf(stderr, "NO ERROR");
    assert_false(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "HELLO!");
    mock_fprintf(stdout, "HELLO!");
    mock_fprintf(NULL, "NO ERROR");
    assert_true(UnitTestingCheckOutput(true));

    UnitTestingExpectOutput(stdout, "HELLO!");
    mock_fprintf(stdout, "HELLO!");
    mock_fprintf(NULL, "NO ERROR");
    assert_false(UnitTestingCheckOutput(false));
}


/**
 * Cat do testowania atrap.
 */
static void MainCat()
{
    int c;
    while ((c = mock_fgetc(stdin)) != EOF) {
        mock_fputc(c, stdout);
//        printf("[%i]", c);
    }
}


static void TestMocksCat(void **state)
{
    (void)state;

    UnitTestingExpectOutput(stdout, "ABC");
    UnitTestingFeedInput(stdin, "ABC", 3);
    MainCat();
    assert_true(UnitTestingCheckOutput(false));

    UnitTestingExpectOutput(stdout, "ABC");
    UnitTestingFeedInput(stdin, "ABCA", 4);
    MainCat();
    assert_false(UnitTestingCheckOutput(false));
}


/**
 * Testuje kompatybilność atrap z testowanym programem.
 */
void TestMocksCompat(void **state)
{
    (void)state;

    const char *in = "1\nDEG\nPRINT\n";
    UnitTestingFeedInput(stdin, in, strlen(in));
    UnitTestingExpectOutput(stdout, "0\n1\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_true(UnitTestingCheckOutput(false));

    in = "1\nPRINT\nPRINT\n";
    UnitTestingFeedInput(stdin, in, strlen(in));
    UnitTestingExpectOutput(stdout, "0\n1\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_false(UnitTestingCheckOutput(false));

    in = "1\nDEG_BY\nPRINT\n";
    UnitTestingFeedInput(stdin, in, strlen(in));
    UnitTestingExpectOutput(stdout, "1\n");
    UnitTestingExpectOutput(stderr, "ERROR 2 WRONG VARIABLE\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_true(UnitTestingCheckOutput(false));

    in = "1\nDEG_BY\nPRINT\n";
    UnitTestingFeedInput(stdin, in, strlen(in));
    UnitTestingExpectOutput(stdout, "1\n");
    UnitTestingExpectOutput(stderr, "ERROR 1 WRONG VARIABLE\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_false(UnitTestingCheckOutput(false));
}


int main(int argc, char **argv)
{
    int failed = 0;

    if (argc > 1 && strcmp(argv[1], "all") == 0) {
        const struct CMUnitTest hello_tests[] = {
                cmocka_unit_test(TestMocks),
                cmocka_unit_test(TestMocksManyStreams),
                cmocka_unit_test(TestMocksCat),
                cmocka_unit_test(TestMocksCompat),
        };
        failed += cmocka_run_group_tests_name("Mocks tests", hello_tests, NULL, NULL);
    }

    //Testy PolyCompose
    const struct CMUnitTest compose_tests[] = {
            cmocka_unit_test(TestPolyComposeZeroZero),
            cmocka_unit_test(TestPolyComposeZeroConst),
            cmocka_unit_test(TestPolyComposeConstZero),
            cmocka_unit_test(TestPolyComposeConstConst),
            cmocka_unit_test(TestPolyComposeLinZero),
            cmocka_unit_test(TestPolyComposeLinConst),
            cmocka_unit_test(TestPolyComposeLinLin),
    };
    failed += cmocka_run_group_tests_name("PolyCompose tests", compose_tests, NULL, NULL);

    return failed;
}

