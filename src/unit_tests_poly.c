/** @file cmocka_tests.c
 * Testy jednostkowe z wykorzystaniem biblioteki CMocka
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include "poly.h"


int tested_main(int argc, char **argv);

//**********************************************************************************************************************
// unit_tests/test_utils
/**
 * Przechowuje dane atrapy strumienia wyjściowego.
 */
typedef struct
{
    ///Strumień (bo możemy mieć jednocześnie wiele atrap na wiele strumieni)
    FILE *stream;

    ///Oczekiwane wyjście na strumieniu
    const char *expected;

    ///Bufor wyjścia strumienia
    char *outputBuffer;

    ///<c>strlen(expected) + 1</c>
    int length;

    ///Liczba znaków wypisanych do tej atrapy
    int outputOffset;

    ///Czy zdażył się overflow?
    bool overflow;
} MockOutputBuffer;


/**
 * Atrapa strumienia wejściowego.
 */
typedef struct
{
    ///Strumień, którego atrapuje bufor
    FILE *stream;

    ///Wskaźnik na początek wejścia
    const char *input;

    ///Ostatni znak
    const char *end;

    ///Bierząca pozycja na wejściu
    const char *pos;
} MockInputBuffer;


///Tablica atrap strumieni wyjściowych (można mieć jednocześnie kilka: na stdout, stderr itd)
static MockOutputBuffer *MockOutputs = NULL;

///Liczba utworzonych atrap strumieni wyjściowych
static unsigned int MockOutputsCounter = 0;

///Rozmiar alokacji tablicy <c>MockFiles</c>
static unsigned int MockOutputsAllocated = 0;

///Czy wypisano coś na inny strumień?
static bool WrongInputTouched = false;

///Tablica atrap strumieni wejściowych (można mieć jednocześnie kilka: na stdout, stderr itd)
static MockInputBuffer *MockInputs = NULL;

///Liczba utworzonych atrap strumieni wejściowych
static unsigned int MockInputsCounter = 0;

///Rozmiar alokacji tablicy <c>MockInputs</c>
static unsigned int MockInputsAllocated = 0;

///Czy wczytano coś z innego strumienia?
static bool WrongOutputTouched = false;

///Liczba znaków przeczytana przez scanf (magicznie dodawana na końcu każdego scanfa przez makro z mock_tricks.h)
int ReadCharCount;


/**
 * Atrapa funkcji fprintf.
 * Przy pomocy vsnprintf wypisuje dane do bufora. Jeżeli dany strumień nie ma przydzielonego bufora, to ustawiana jest
 * falga <c>WrongStreamTouched</c> i dane są wypisywane przy pomocy zwykłej funkcji fprintf.
 * @param stream strumień
 * @param format format wypisanych danych
 * @param ... ...
 * @return liczbę wypisanych znaków (lub tych, co powinny być wypisane) ("wypisane" może też oznaczać "wstawione do
 * bufora")
 */
int mock_fprintf(FILE *stream, const char *format, ...)
{
    //Znajdź właściwy bufor
    MockOutputBuffer *out = NULL;
    for (unsigned int i = 0; i < MockOutputsCounter; ++i) {
        if (MockOutputs[i].stream == stream) {
            out = MockOutputs + i;
            break;
        }
    }

    va_list args;
    va_start(args, format);
    if (out == NULL) {
        WrongInputTouched = true;
        if (stream)
            return vfprintf(stream, format, args);
        return 0;
    }

    int result = vsnprintf(out->outputBuffer + out->outputOffset, out->length - out->outputOffset, format, args);
    va_end(args);

    if (result > 0)
        out->outputOffset += result;
    out->overflow = out->overflow || out->outputOffset >= out->length;
    return result;
}


/**
 * Atrapuje <c>fputc</c>
 */
int mock_fputc(int c, FILE *stream)
{
    //Znajdź właściwy bufor
    MockOutputBuffer *out = NULL;
    for (unsigned int i = 0; i < MockOutputsCounter; ++i) {
        if (MockOutputs[i].stream == stream) {
            out = MockOutputs + i;
            break;
        }
    }

    if (out == NULL) {
        WrongInputTouched = true;
        if (stream)
            return fputc(c, stream);
        return EOF;
    }

    if (out->outputOffset + 1 >= out->length) {
        out->overflow = true;
        return EOF;
    }

    out->outputBuffer[out->outputOffset++] = (char)c;
    out->outputBuffer[out->outputOffset] = 0;
    return c;
}


/**
 * Atrapuje <c>fgetc</c>
 */
int mock_fgetc(FILE *stream)
{
    MockInputBuffer *in = NULL;
    for (unsigned int i = 0; i < MockInputsCounter; ++i) {
        if (MockInputs[i].stream == stream) {
            in = MockInputs + i;
            break;
        }
    }

    if (in == NULL) {
        WrongOutputTouched = true;
        if (stream)
            return fgetc(stream);
        return EOF;
    }

    assert_true(in->pos <= in->end);
    if (in->pos == in->end)
        return EOF;
    return *(in->pos++);
}


/**
 * Atrapa <c>scanf</c>.
 * Wymaga, aby odpowiednie makro samo dopisało do formatu <c>%n</c> oraz dodało jako ostatni argument
 * <c>ReadCharCount</c>.
 * @param format format z dopisanym na końcu <c>%n</c>
 * @param ... lista parametrów z dołożonym na końcu <c>ReadCharCount</c>
 * @return liczbę przeczytanych rzeczy
 */
int mock_scanf(const char *format, ...)
{
    MockInputBuffer *in = NULL;
    for (unsigned int i = 0; i < MockInputsCounter; ++i) {
        if (MockInputs[i].stream == stdin) {
            in = MockInputs + i;
            break;
        }
    }

    va_list args;
    int ret;
    assert_true(in != NULL);
    va_start(args, format);
    ret = vsscanf(in->pos, format, args);
    va_end(args);

    if (ret < 0) {
        in->pos = in->end;
    } else {
        assert_true(ReadCharCount >= 0);
        in->pos += ReadCharCount;
        if (in->pos > in->end) {
            //Ale jak tak, to chyba coś jest źle...
            in->pos = in->end;
        }
    }
    return ret;
}


/**
 * Przygotowuje atrapę dla podanego strumienia, która spodziewa się podanych danych wyjściowych.
 * @param stream strumień atrapy
 * @param expect oczekiwane dane
 */
void ExpectOutput(FILE *stream, const char *expect)
{
    MockOutputBuffer next;
    next.stream = stream;
    next.expected = expect;
    next.length = strlen(expect) + 1;
    next.outputBuffer = malloc(next.length);
    *next.outputBuffer = 0;
    next.outputOffset = 0;
    next.overflow = false;

    if (MockOutputsAllocated == MockOutputsCounter) {
        MockOutputsAllocated = MockOutputsAllocated == 0 ? 1 : 2 * MockOutputsAllocated;
        MockOutputs = realloc(MockOutputs, MockOutputsAllocated * sizeof(MockOutputBuffer));
    }
    MockOutputs[MockOutputsCounter++] = next;
}


/**
 * Przygotowuje atrapę strumnienia wejściowego.
 * @param stream strumień wejściowy
 * @param input dane wejściowe
 * @param length długość danych wejściowych
 */
static void FeedInput(FILE *stream, const char *input, unsigned int length)
{
    MockInputBuffer next = (MockInputBuffer) {
            .stream = stream,
            .input = input,
            .pos = input,
            .end = input + length,
    };

    if (MockInputsAllocated == MockInputsCounter) {
        MockInputsAllocated = MockInputsAllocated == 0 ? 1 : 2 * MockInputsAllocated;
        MockInputs = realloc(MockInputs, MockInputsAllocated * sizeof(MockOutputBuffer));
    }
    MockInputs[MockInputsCounter++] = next;
}

/**
 * Sprawdza, czy wszystkie atrapy strumieni wyjściowych dostały poprawne oczekiwane dane.
 * @param ignore_other czy ignorować dane wypisane na strumienie bez atrap (jeżeli <c>false</c> to znaczy, że atrapami
 * są wszystkie używane strumienie)
 * @return czy wyjście jest zgodne z oczekiwanym
 */
static bool CheckOutput(bool ignore_other)
{
    bool check = (!WrongInputTouched || ignore_other) && (!WrongOutputTouched || ignore_other);

    for (unsigned int i = 0; i < MockOutputsCounter; ++i) {
        MockOutputBuffer next = MockOutputs[i];
        check = check && !next.overflow && memcmp(next.expected, next.outputBuffer, (size_t)next.length) == 0;
//        printf("\n{%s}{%s}=%i;;%i\n", next.expected, next.outputBuffer, memcmp(next.expected, next.outputBuffer, (size_t)next.length), (int)check);
        free(next.outputBuffer);
    }

    free(MockOutputs);
    MockOutputs = NULL;
    MockOutputsCounter = 0;
    MockOutputsAllocated = 0;
    WrongOutputTouched = false;

    free(MockInputs);
    MockInputs = NULL;
    MockInputsCounter = 0;
    MockInputsAllocated = 0;
    WrongInputTouched = false;

    return check;
}



//**********************************************************************************************************************
// unit_tests/poly_compose
/**
 * <c>p</c> wielomian zerowy, <c>count</c> równe <c>0</c>
 */
static void TestPolyComposeZeroZero(void **state)
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


/**
 * <c>p</c> wielomian zerowy, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian stały
 */
static void TestPolyComposeZeroConst(void **state)
{
    (void)state;

    Poly p = PolyZero();
    Poly expect = PolyZero();
    Poly x[] = {PolyFromCoeff(42)};

    Poly got = PolyCompose(&p, 1, x);
    assert_true(PolyIsEq(&got, &expect));

    PolyDestroy(&p);
    PolyDestroy(&expect);
    PolyDestroy(&got);
    PolyDestroy(x + 0);
}


/**
 * <c>p</c> wielomian stały, <c>count</c> równe <c>0</c>
 */
static void TestPolyComposeConstZero(void **state)
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


/**
 * <c>p</c> wielomian stały, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian stały różny od <c>p</c>
 */
static void TestPolyComposeConstConst(void **state)
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


/**
 * <c>p</c> wielomian liniowy, <c>count</c> równe <c>0</c>
 */
static void TestPolyComposeLinZero(void **state)
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


/**
 * <c>p</c> wielomian liniowy, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian stały
 */
static void TestPolyComposeLinConst(void **state)
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


/**
 * <c>p</c> wielomian liniowy, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian linoowy
 */
static void TestPolyComposeLinLin(void **state)
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


//**********************************************************************************************************************
// unit_tests/calc_compose
/**
 * Uruchamia program z podanym wejściem i sprawdza wyjścia.
 * @param in wejście
 * @param expected_out oczekiwane wyjście
 * @param expected_err oczekiwane wyjście błędu
 */
static void TestCore(const char *in, const char *expected_out, const char *expected_err)
{
    FeedInput(stdin, in, strlen(in));
    ExpectOutput(stdout, expected_out);
    ExpectOutput(stderr, expected_err);
    tested_main(1, (char*[]){"./calc_poly"});
    assert_true(CheckOutput(false));
}


/**
 * <c>COMPOSE</c> bez parametru.
 */
static void TestCalcComposeNoParam(void **state)
{
    (void)state;
    const char *in = "COMPOSE\n";
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


/**
 * <c>COMPOSE 0</c>
 */
static void TestCalcComposeZero(void **state)
{
    (void)state;
    const char *in = "(42,0)+(1,1)\nCOMPOSE 0\nPRINT\n";
    const char *expected_out = "42\n";
    const char *expected_err = "";
    TestCore(in, expected_out, expected_err);
}


/**
 * <c>COMPOSE UINT_MAX</c>
 */
static void TestCalcComposeUMax(void **state)
{
    (void)state;
    char *in = malloc(256);
    snprintf(in, 256, "(42,0)+(1,1)\nCOMPOSE %lu\n", (long unsigned)UINT_MAX);
    const char *expected_out = "";
    const char *expected_err = "ERROR 2 STACK UNDERFLOW\n";
    TestCore(in, expected_out, expected_err);
    free(in);
}

/**
 * <c>COMPOSE -1</c>
 */
static void TestCalcComposeNegative(void **state)
{
    (void)state;
    char *in = "COMPOSE -1\n";
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


/**
 * <c>COMPOSE UINT_MAX + 1</c>
 */
static void TestCalcComposeOverflow(void **state)
{
    (void)state;
    char *in = malloc(256);
    snprintf(in, 256, "COMPOSE %lu\n", (long unsigned)UINT_MAX + 1);
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
    free(in);
}


/**
 * <c>COMPOSE UINT_MAX + dużo</c>
 */
static void TestCalcComposeOooveeerflooow(void **state)
{
    (void)state;
    char *in = "COMPOSE 8364889373929365739284365876348912907120974358679243537901234097234689234\n"; //Powinno starczyć
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


/**
 * <c>COMPOSE kapibara</c>
 */
static void TestCalcComposeCapybara(void **state)
{
    (void)state;
    char *in = "COMPOSE kapibara\n"; //.-.
    const char *expected_out = "";
    const char *expected_err = "ERROR 1 WRONG COUNT\n";
    TestCore(in, expected_out, expected_err);
}


/**
 * <c>COMPOSE 44kapibary</c>
 */
static void TestCalcCompose44Capybaras(void **state)
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


/**
 * Testy compose z przykładu
 */
static void TestCalcComposeExample(void **state)
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


//**********************************************************************************************************************
// unit_tests/tests_main
/**
 * Testy atrap strumieni (czyli testowanie jednostkowe testów jednostkowych).
 * @param state
 */
static void TestMocks(void **state)
{
    (void)state;

    ExpectOutput(stdout, "");
    mock_fprintf(stdout, "");
    mock_fprintf(stdout, "");
    mock_fprintf(stdout, "");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "ABC");
    mock_fprintf(stdout, "ABC");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "ABC");
    mock_fprintf(stdout, "A");
    mock_fprintf(stdout, "B");
    mock_fprintf(stdout, "C");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "Hello World!\nSaid Dregon Reborn and then flew away.");
    mock_fprintf(stdout, "Hello");
    mock_fprintf(stdout, " World!\n");
    mock_fprintf(stdout, "Said %s and then flew away.", "Dregon Reborn");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "");
    mock_fprintf(stdout, "assss");
    assert_false(CheckOutput(true));

    ExpectOutput(stdout,
                 "/home/saphir/Documents/MIMUW/2016-2017/IPP/cmocka_example/MyMock/tests.c:87:15: error: ‘long long long’ is too long for GCC");
    mock_fprintf(stdout, "/home/saphir/Documents/MIMUW/2016-2017/IPP/cmocka_example/MyMock/tests.c:87:15: error: ");
    mock_fprintf(stdout, "‘long long long’");
    mock_fprintf(stdout, " is too long for GCC\n");
    assert_false(CheckOutput(true));
}


/**
 * Testy atrap strumieni (czyli testowanie jednostkowe testów jednostkowych) -- ciąg dalszy.
 * @param state
 */
static void TestMocksManyStreams(void **state)
{
    (void)state;

    ExpectOutput(stdout, "0\n(1,1)\n");
    ExpectOutput(stderr, "ERROR 2 NO ERROR");
    mock_fprintf(stderr, "ERROR");
    mock_fprintf(stdout, "0");
    mock_fprintf(stderr, " 2 ");
    mock_fprintf(stdout, "\n(1,1)\n");
    mock_fprintf(stderr, "NO ERROR");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "0\n(1,1)\n");
    ExpectOutput(stderr, "ERROR 2 NO ERROR");
    mock_fprintf(stderr, "ERROR");
    mock_fprintf(stdout, "0");
    mock_fprintf(stderr, " 3 ");
    mock_fprintf(stdout, "\n(1,1)\n");
    mock_fprintf(stderr, "NO ERROR");
    assert_false(CheckOutput(true));

    ExpectOutput(stdout, "HELLO!");
    mock_fprintf(stdout, "HELLO!");
    mock_fprintf(NULL, "NO ERROR");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "HELLO!");
    mock_fprintf(stdout, "HELLO!");
    mock_fprintf(NULL, "NO ERROR");
    assert_false(CheckOutput(false));
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

    ExpectOutput(stdout, "ABC");
    FeedInput(stdin, "ABC", 3);
    MainCat();
    assert_true(CheckOutput(false));

    ExpectOutput(stdout, "ABC");
    FeedInput(stdin, "ABCA", 4);
    MainCat();
    assert_false(CheckOutput(false));
}


/**
 * Testuje kompatybilność atrap z testowanym programem.
 */
static void TestMocksCompat(void **state)
{
    (void)state;

    const char *in = "1\nDEG\nPRINT\n";
    FeedInput(stdin, in, strlen(in));
    ExpectOutput(stdout, "0\n1\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_true(CheckOutput(false));

    in = "1\nPRINT\nPRINT\n";
    FeedInput(stdin, in, strlen(in));
    ExpectOutput(stdout, "0\n1\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_false(CheckOutput(false));

    in = "1\nDEG_BY\nPRINT\n";
    FeedInput(stdin, in, strlen(in));
    ExpectOutput(stdout, "1\n");
    ExpectOutput(stderr, "ERROR 2 WRONG VARIABLE\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_true(CheckOutput(false));

    in = "1\nDEG_BY\nPRINT\n";
    FeedInput(stdin, in, strlen(in));
    ExpectOutput(stdout, "1\n");
    ExpectOutput(stderr, "ERROR 1 WRONG VARIABLE\n");
    tested_main(1, (char*[]){"./calc_poly"});
    assert_false(CheckOutput(false));
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

    //Testy programu
    const struct CMUnitTest program_tests[] = {
            cmocka_unit_test(TestCalcComposeNoParam),
            cmocka_unit_test(TestCalcComposeZero),
            cmocka_unit_test(TestCalcComposeUMax),
            cmocka_unit_test(TestCalcComposeNegative),
            cmocka_unit_test(TestCalcComposeOverflow),
            cmocka_unit_test(TestCalcComposeOooveeerflooow),
            cmocka_unit_test(TestCalcComposeCapybara),
            cmocka_unit_test(TestCalcCompose44Capybaras),
//            cmocka_unit_test(TestCalcComposeExample),
    };
    failed += cmocka_run_group_tests_name("Program tests", program_tests, NULL, NULL);

    return failed;
}

