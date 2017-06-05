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


///Tablica atrap strumieni wejściowych (można mieć jednocześnie kilka: na stdout, stderr itd)
static MockOutputBuffer *MockFiles = NULL;

///Liczba utworzonych atrap strumieni wyjściowych
static unsigned int MockFilesCounter = 0;

///Rozmiar alokacji tablicy <c>MockFiles</c>
static unsigned int MockFilesAllocated = 0;

///Czy wypisano coś na inny strumień?
static bool WrongStreamTouched = false;


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
    for (unsigned int i = 0; i < MockFilesCounter; ++i) {
        if (MockFiles[i].stream == stream) {
            out = MockFiles + i;
            break;
        }
    }

    va_list args;
    va_start(args, format);
    if (out == NULL) {
        WrongStreamTouched = true;
        return vfprintf(stream, format, args);
    }

    assert_true(out->expected != NULL);
    int result = vsnprintf(out->outputBuffer + out->outputOffset, out->length - out->outputOffset, format, args);
    va_end(args);

    if (result > 0)
        out->outputOffset += result;
    out->overflow = out->overflow || out->outputOffset >= out->length;
    return result;
}


/**
 * Przygotowuje atrapę dla podanego strumienia, która spodziewa się podanych danych wyjściowych.
 * @param stream strumień atrapy
 * @param expect oczekiwane dane
 */
static void ExpectOutput(FILE *stream, const char *expect)
{
    MockOutputBuffer next;
    next.stream = stream;
    next.expected = expect;
    next.length = strlen(expect) + 1;
    next.outputBuffer = malloc(next.length);
    *next.outputBuffer = 0;
    next.outputOffset = 0;
    next.overflow = false;

    if (MockFilesAllocated == MockFilesCounter) {
        MockFilesAllocated = MockFilesAllocated == 0 ? 1 : 2 * MockFilesAllocated;
        MockFiles = realloc(MockFiles, MockFilesAllocated * sizeof(MockOutputBuffer));
    }
    MockFiles[MockFilesCounter++] = next;
}


/**
 * Sprawdza, czy wszystkie atrapy strumieni wyjściowych dostały poprawne oczekiwane dane.
 * @param ignore_other czy ignorować dane wypisane na strumienie bez atrap (jeżeli <c>false</c> to znaczy, że atrapami
 * są wszystkie używane strumienie)
 * @return czy wyjście jest zgodne z oczekiwanym
 */
static bool CheckOutput(bool ignore_other)
{
    bool check = !WrongStreamTouched || ignore_other;
    for (unsigned int i = 0; i < MockFilesCounter; ++i) {
        MockOutputBuffer next = MockFiles[i];
        check = check && !next.overflow && memcmp(next.expected, next.outputBuffer, next.length) == 0;
//        printf("\n{%s}{%s}\n", next.expected, next.outputBuffer);
        next.expected = NULL;
        next.length = 0;
        free(next.outputBuffer);
        next.outputBuffer = NULL;
        next.outputOffset = 0;
        next.overflow = false;
    }
    free(MockFiles);
    MockFiles = NULL;
    MockFilesCounter = 0;
    MockFilesAllocated = 0;
    WrongStreamTouched = false;
    return check;
}


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

    ExpectOutput(stdout, "/home/saphir/Documents/MIMUW/2016-2017/IPP/cmocka_example/MyMock/tests.c:87:15: error: ‘long long long’ is too long for GCC");
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
    mock_fprintf(stderr, "NO ERROR");
    assert_true(CheckOutput(true));

    ExpectOutput(stdout, "HELLO!");
    mock_fprintf(stdout, "HELLO!");
    mock_fprintf(stderr, "NO ERROR");
    assert_false(CheckOutput(false));
}


int main(int argc, char **argv)
{
    int failed = 0;

    if (argc > 1 && strcmp(argv[1], "all") == 0) {
        const struct CMUnitTest hello_tests[] = {
                cmocka_unit_test(TestMocks),
                cmocka_unit_test(TestMocksManyStreams),
        };
        failed += cmocka_run_group_tests_name("Mocks tests", hello_tests, NULL, NULL);
    }
}

