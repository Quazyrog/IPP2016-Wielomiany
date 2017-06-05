/** @file tests_utils.c
 * Funkcje potrzebne do testowania (na ogół obsługa atrap): implementacja
 */
#include "tests_utils.h"


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


void UnitTestingExpectOutput(FILE *stream, const char *expect)
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


void UnitTestingFeedInput(FILE *stream, const char *input, unsigned int length)
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


bool UnitTestingCheckOutput(bool ignore_other)
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
