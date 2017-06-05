/** @file parser.c
 * Plik z implementacją parsera.
 *
 * Pomocnicze funkcję parsujące zadeklarowane w tym pliku (takie jak <c>ParseInteger()</c>, <c>ParseNextLine()</c> ...)
 * stosują się do generalnej konwencji, jeśli chodzi o pochłanianie tokenów z wejscia:
 *   - W momencie wywołania funkcji oczekuje sie, że wczytanym do leksera tokenem będzie pierwszy token wchodzący
 *     w skład parsowanej jednostki składniowej
 *   - Jeżeli parsowanie zakończy się sukcesem, to wczytanym do leksera tokenem będzie pierwszy token kolejnej jednostki
 *     składniowej
 *   - Jeżeli parsowanie nie powiedzie się, wczytanym do leksera tokenem będzie token, który spowodował błąd.
 */
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "parser.h"
#include "poly.h"
#include "lexer.h"
#include "calculator_stack.h"
#include "unit_tests/mock_tricks.h"


/**
 * Parsuje wielomian.
 * W skład wielomianun wchodzi liczba ze znakiem, lub sekwencja jednomianów oddzielonych znakiem '+'.
 * @param p parser źródłowy
 * @param out miejsce w pamięci, do którego zostanie zapisany wynik
 * @return informacje o powodzeniu parsowania
 */
static bool ParsePolynomial(Parser *p, Poly *out);


/**
 * Parsuje współczynnik jednomianu (aka <c>long</c>).
 * To mogłoby byc po prostu ParseIntegere, ALE:
 *   - ParseInteger parsuje <c>long long int</c>, więc mogło by nie wykryć overflowu
 *   - Powyższe rzecz jasna nie byłoby problemem, bo można zrzucowac na <c>poly_coeff_t</c> i wtedy wykryć overflow.
 *     ALE musimy wiedzieć, która cyfra powoduje overflowa, co wszystko psuje: musimy mieć własne ewaluowanie liczb
 * Wartość <c>out</c> nie jest zmieniana w przypadku nieudanego parsowania.
 * @param p parser źródłowy
 * @param out miejsce do zapisania wyniku
 * @param invalid_digit tu zostanie zapisany offset zanku w buforze, który spowodował błąd parsowania
 * Może być <c>NULL</c>, jeśli pozycja nas nie interesuje
 * <c>Lexer.startColumn + *overflow_digit</c>
 * @return <c>false</c> jeżeli wystapił overflow; <c>true</c> w.p.p.
 */
static bool ParseCoefficient(Parser *p, poly_coeff_t *out, uint32_t *invalid_digit)
{
    poly_coeff_t sgn = 1;
    if (p->lexer.tokenType == TOKEN_SPECIAL_CHAR) {
        if (p->lexer.tokenBuffer[0] == '-')
            sgn = -1;
        else
            return false;
        LexerReadNextToken(&p->lexer);
    }

    if (p->lexer.tokenType != TOKEN_NUMBER) {
        if (invalid_digit != NULL)
            *invalid_digit = 0 + p->lexer.skippedZeros;
        return false;
    }
    const char *digits = p->lexer.tokenBuffer;
    poly_coeff_t value = 0;

    for (uint32_t next_digit = 0; digits[next_digit] != 0; ++next_digit) {
        poly_coeff_t digit = digits[next_digit] - '0';
        poly_coeff_t new_value;

        new_value = value * 10;
        if (new_value / 10 != value || (new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit + p->lexer.skippedZeros;
            return false;
        }

        new_value = new_value + sgn * digit;
        if ((new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit + p->lexer.skippedZeros;
            return false;
        }

        value = new_value;
    }

    *out = value;
    LexerReadNextToken(&p->lexer);
    return true;
}


/**
 * Parsuje wykładnik jednomianu (aka <c>int</c>, ale nie pozwala na znak).
 * To nie może być używane wcześniej parse unsigned, bo wykładniki wchodzą też w skład wielomianu. Wartość <c>out</c>
 * nie jest zmieniana w przypadku nieudanego parsowania. Jako, iż liczba ta zawsze ma być dodatnia, niedozwolone
 * jest poprzedzanie znakiem (nawet znakiem '+').
 * @param p parser źródłowy
 * @param out miejsce do zapisania wyniku
 * @param invalid_digit tu zostanie zapisany offset zanku w buforze, który spowodował błąd parsowania
 * Może być <c>NULL</c>, jeśli pozycja nas nie interesuje
 * <c>Lexer.startColumn + *overflow_digit</c>
 * @return <c>false</c> jeżeli wystapił overflow; <c>true</c> w.p.p.
 */
static bool ParseExponent(Parser *p, poly_exp_t *out, uint32_t *invalid_digit)
{
    if (p->lexer.tokenType != TOKEN_NUMBER) {
        if (invalid_digit != NULL)
            *invalid_digit = 0;
        return false;
    }
    const char *digits = p->lexer.tokenBuffer;
    poly_exp_t value = 0;

    for (uint32_t next_digit = 0; digits[next_digit] != 0; ++next_digit) {
        poly_exp_t digit = digits[next_digit] - '0';
        poly_exp_t new_value;

        new_value = value * 10;
        if (new_value / 10 != value || (new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit;
            return false;
        }

        new_value = new_value + digit;
        if ((new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit;
            return false;
        }

        value = new_value;
    }

    *out = value;
    LexerReadNextToken(&p->lexer);
    return true;
}


/**
 * Patarsuje i ustawia na stosie operacji argument typu <c>unsigned int</c>.
 * @param p struktura parsera
 * @return feedback parsowania
 */
static bool ParseAndPushUIntParameter(Parser *p, const char *error_message)
{
    if (!LexerExpectChar(&p->lexer, ' ') || p->lexer.tokenType != TOKEN_NUMBER) {
        fprintf(stderr, "ERROR %u %s\n", (unsigned int)p->lexer.startLine, error_message);
        return false;
    }
    long unsigned int arg = strtoul(p->lexer.tokenBuffer, NULL, 10);
    LexerReadNextToken(&p->lexer);
    if (errno == ERANGE || arg > UINT_MAX || p->lexer.tokenBuffer[0] != '\n') {
        fprintf(stderr, "ERROR %u %s\n", (unsigned int)p->lexer.startLine, error_message);
        return false;
    }
    CSSetUIArg(&p->stack, (unsigned int)arg);
    return true;
}


/**
 * Parsuje i wykonuje polecenie.
 * W przypadku błędu wypisuje komunikat zgodny z treścią zadania. Po poleceniu powinien następować separator, jednak
 * nie jest on wliczany do jednoski składniowej polecenia (zostanie w buforze).
 * @param p parser źródłowy
 * @return informację o powodzeniu
 */
static bool ParseCommand(Parser *p)
{
    CSOperation op_code;
    if (p->lexer.tokenType == TOKEN_INVALID_OVERFLOW) {
        fprintf(stderr, "ERROR %u WRONG COMMAND\n", (unsigned int)p->lexer.startLine);
        return false;
    }

    op_code = CSOperationFromString(p->lexer.tokenBuffer);
    LexerReadNextToken(&p->lexer);

    if (op_code == OPERATION_AT) {
        poly_coeff_t arg = 0;
        if (!LexerExpectChar(&p->lexer, ' ') || !ParseCoefficient(p, &arg, NULL) || p->lexer.tokenBuffer[0] != '\n') {
            fprintf(stderr, "ERROR %u WRONG VALUE\n", (unsigned int)p->lexer.startLine);
            return false;
        }
        CSSetPCArg(&p->stack, arg);
    } else if (op_code == OPERATION_DEG_BY || op_code == OPERATION_COMPOSE) {
        if (!ParseAndPushUIntParameter(p, op_code == OPERATION_DEG_BY ? "WRONG VARIABLE" : "WRONG COUNT"))
            return false;
    } else {
        if (op_code == OPERATION_INVALID || p->lexer.tokenBuffer[0] != '\n') {
            fprintf(stderr, "ERROR %u WRONG COMMAND\n", (unsigned int)p->lexer.startLine);
            return false;
        }
    }

    if (!CSCanExecute(&p->stack, op_code)) {
        fprintf(stderr, "ERROR %u STACK UNDERFLOW\n", (unsigned int)p->lexer.startLine);
        return false;
    }
    CSExecute(&p->stack, op_code, p->output);
    return true;
}


/**
 * Parsuje jednomian, czyli wyrażenie w postaci "(<Wielomian>,<Liczba bez znaku>)". W przypadku niepowodzenia, wypisuje
 * komunikat błędowy zgodny z treścią zadania.
 * @param p parser źródłowy
 * @param out miejsce w pamięci do zapisania sparsowanego jednomianu
 * @return feedback parsowania
 */
static bool ParseMonomial(Parser *p, Mono *out)
{
    if (!LexerExpectChar(&p->lexer, '(')) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
        return false;
    }

    Poly poly;
    if (!ParsePolynomial(p, &poly))
        return false;

    if (!LexerExpectChar(&p->lexer, ',')) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
        PolyDestroy(&poly);
        return false;
    }

    poly_exp_t exponent;
    uint32_t error_location;
    if (!ParseExponent(p, &exponent, &error_location)) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn + error_location);
        PolyDestroy(&poly);
        return false;
    }

    if (!LexerExpectChar(&p->lexer, ')')) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
        PolyDestroy(&poly);
        return false;
    }

    *out = MonoFromPoly(&poly, exponent);
    return true;
}


static bool ParsePolynomial(Parser *p, Poly *out)
{
    if (p->lexer.tokenBuffer[0] == '(') {
        uint32_t mono_array_size = 2;
        uint32_t mono_array_top_ptr = 0;
        Mono *mono_array = malloc(mono_array_size * sizeof(Mono));
        assert(mono_array != NULL);
        Mono next;
        bool status = true;

        do {
            if (!ParseMonomial(p, &next)) {
                status = false;
                break;
            }
            if (mono_array_top_ptr == mono_array_size) {
                Mono *temp = realloc(mono_array, sizeof(Mono) * mono_array_size * 2);
                assert(temp != NULL);
                mono_array_size *= 2;
                mono_array = temp;
            }
            mono_array[mono_array_top_ptr++] = next;
        } while (LexerExpectChar(&p->lexer, '+'));

        if (status) {
            *out = PolyAddMonos(mono_array_top_ptr, mono_array);
        } else {
            for (uint32_t i = 0; i < mono_array_top_ptr; ++i)
                MonoDestroy(mono_array + i);
        }
        free(mono_array);
        return status;

    } else {
        uint32_t error_offset = 0;
        poly_coeff_t coef;
        if (!ParseCoefficient(p, &coef, &error_offset)) {
            fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn + error_offset);
            return false;
        }
        *out = PolyFromCoeff(coef);
        return true;
    }
}


/**
 * Parsuje kolejną linię pliku.
 * Jeżeli po sparsowaniu oczekiwanej zawartości linii (wielomian lub polecenie) nie znajduje się w niej koniec wiersza,
 * zostanie wypluty komunikat: <c>ERROR r EOL EXPECTED</c>.
 * @param p parser źródłowy
 * @return informacje o wyniku parsowania
 */
static bool ParseNextLine(Parser *p)
{
    int feedback;
    if (isalpha(p->lexer.tokenBuffer[0])) {
        feedback = ParseCommand(p);
    } else {
        Poly poly;
        feedback = ParsePolynomial(p, &poly);
        if (feedback && p->lexer.tokenBuffer[0] != '\n') {
            fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
            PolyDestroy(&poly);
            return false;
        }
        if (feedback)
            CSPushPolynomial(&p->stack, poly);
    }

    if (!feedback)
        return false;
    LexerSkipEOL(&p->lexer);
    return true;
}


void ParserPrepare(Parser *parser, FILE *source, FILE *output)
{
    parser->output = output;
    LexerInit(&parser->lexer, source);
}


bool ParserExecuteAll(Parser *parser, bool error_resume_next) {
    LexerReadNextToken(&parser->lexer);
    while (parser->lexer.tokenType != TOKEN_EOF) {
        int feedback = ParseNextLine(parser);
        if (!feedback) {
            if (!error_resume_next)
                return false;
            LexerSkipEOL(&parser->lexer);
        }
    }
    return true;
}


Parser ParserInit(void) {
    Parser result;
    result.stack = CSInit();
    return result;
}


void ParserDestroy(Parser *parser) {
    CSDestroy(&parser->stack);
}
