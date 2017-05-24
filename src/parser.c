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
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "parser.h"
#include "poly.h"
#include "lexer.h"


/**
 * Parsuje wielomian.
 * W skład wielomianun wchodzi liczba ze znakiem, lub sekwencja jednomianów oddzielonych znakiem '+'.
 * @param p parser źródłowy
 * @param out miejsce w pamięci, do którego zostanie zapisany wynik
 * @return informacje o powodzeniu parsowania
 */
static int ParsePolynomial(Parser *p, Poly *out);


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
 * @return <c>PARSE_FAILURE</c> jeżeli wystapił overflow; <c>PARSE_SUCCESS</c> w.p.p.
 */
static int ParseCoefficient(Parser *p, poly_coeff_t *out, uint32_t *invalid_digit)
{
    poly_coeff_t sgn = 1;
    if (p->lexer.tokenType == TOKEN_SPECIAL_CHAR) {
        if (p->lexer.tokenBuffer[0] == '-')
            sgn = -1;
        else if (p->lexer.tokenBuffer[0] != '+')
            return PARSE_FAILURE;
        LexerReadNextToken(&p->lexer);
    }

    if (p->lexer.tokenType != TOKEN_NUMBER) {
        if (invalid_digit != NULL)
            *invalid_digit = 0;
        return PARSE_FAILURE;
    }
    const char *digits = p->lexer.tokenBuffer;
    poly_coeff_t value = 0;

    for (uint32_t next_digit = 0; digits[next_digit] != 0; ++next_digit) {
        poly_coeff_t digit = digits[next_digit] - '0';
        poly_coeff_t new_value;

        new_value = value * 10;
        if (new_value / 10 != value || (new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit;
            return PARSE_FAILURE;
        }

        new_value = new_value + sgn * digit;
        if ((new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit;
            return PARSE_FAILURE;
        }

        value = new_value;
    }

    *out = value;
    LexerReadNextToken(&p->lexer);
    return PARSE_SUCCESS;
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
 * @return <c>PARSE_FAILURE</c> jeżeli wystapił overflow; <c>PARSE_SUCCESS</c> w.p.p.
 */
static int ParseExponent(Parser *p, poly_exp_t *out, uint32_t *invalid_digit)
{
    if (p->lexer.tokenType != TOKEN_NUMBER) {
        if (invalid_digit != NULL)
            *invalid_digit = 0;
        return PARSE_FAILURE;
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
            return PARSE_FAILURE;
        }

        new_value = new_value + digit;
        if ((new_value < 0 && value > 0) || (new_value > 0 && value < 0)) {
            if (invalid_digit != NULL)
                *invalid_digit = next_digit;
            return PARSE_FAILURE;
        }

        value = new_value;
    }

    *out = value;
    LexerReadNextToken(&p->lexer);
    return PARSE_SUCCESS;
}


/**
 * Parsuje i wykonuje polecenie.
 * W przypadku błędu wypisuje komunikat zgodny z treścią zadania. Po poleceniu powinien następować separator, jednak
 * nie jest on wliczany do jednoski składniowej polecenia (zostanie w buforze).
 * @param p parser źródłowy
 * @return informację o powodzeniu
 */
static int ParseCommand(Parser *p)
{
    if (strcmp(p->lexer.tokenBuffer, "AT") == 0) {
        poly_coeff_t arg;
        LexerReadNextToken(&p->lexer);
        if (!LexerExpectChar(&p->lexer, ' ') || !ParseCoefficient(p, &arg, NULL) || p->lexer.tokenType != TOKEN_SEPARATOR) {
            fprintf(stderr, "ERROR %u WRONG VALUE\n", (unsigned int)p->lexer.startLine);
            return PARSE_FAILURE;
        }
        CSSetPCArg(&p->stack, arg);
        CSExecute(&p->stack, OPERATION_AT, p->output);

    } else if (strcmp(p->lexer.tokenBuffer, "DEG_BY") == 0) {
        poly_exp_t arg;
        LexerReadNextToken(&p->lexer);
        if (!LexerExpectChar(&p->lexer, ' ') || !ParseExponent(p, &arg, NULL) || p->lexer.tokenType != TOKEN_SEPARATOR) {
            fprintf(stderr, "ERROR %u WRONG VALUE\n", (unsigned int)p->lexer.startLine);
            return PARSE_FAILURE;
        }
        CSSetPEArg(&p->stack, arg);
        CSExecute(&p->stack, OPERATION_DEG_BY, p->output);

    } else {
        CSOperation op_code = CSOperationFromString(p->lexer.tokenBuffer);
        CSExecute(&p->stack, op_code, p->output);
        LexerReadNextToken(&p->lexer);
    }

    return PARSE_SUCCESS;
}


/**
 * Parsuje jednomian, czyli wyrażenie w postaci "(<Wielomian>,<Liczba bez znaku>)". W przypadku niepowodzenia, wypisuje
 * komunikat błędowy zgodny z treścią zadania.
 * @param p parser źródłowy
 * @param out miejsce w pamięci do zapisania sparsowanego jednomianu
 * @return feedback parsowania
 */
static int ParseMonomial(Parser *p, Mono *out)
{
    if (!LexerExpectChar(&p->lexer, '(')) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
        return PARSE_FAILURE;
    }

    Poly poly;
    if (!ParsePolynomial(p, &poly))
        return PARSE_FAILURE;

    if (!LexerExpectChar(&p->lexer, ',')) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
        PolyDestroy(&poly);
        return PARSE_FAILURE;
    }

    poly_exp_t exponent;
    uint32_t error_location;
    if (!ParseExponent(p, &exponent, &error_location)) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn + error_location);
        PolyDestroy(&poly);
        return PARSE_FAILURE;
    }

    if (!LexerExpectChar(&p->lexer, ')')) {
        fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn);
        PolyDestroy(&poly);
        return PARSE_FAILURE;
    }

    *out = MonoFromPoly(&poly, exponent);
    return PARSE_SUCCESS;
}


static int ParsePolynomial(Parser *p, Poly *out)
{
    if (p->lexer.tokenBuffer[0] == '(') {
        uint32_t mono_array_size = 256;
        uint32_t mono_array_top_ptr = 0;
        Mono *mono_array = malloc(mono_array_size * sizeof(Mono));
        assert(mono_array != NULL);
        Mono next;
        int status = PARSE_SUCCESS;

        do {
            if (!ParseMonomial(p, &next)) {
                status = PARSE_FAILURE;
                break;
            }
            if (mono_array_top_ptr == mono_array_size) {
                Mono *temp = realloc(mono_array, mono_array_size * 2);
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
        uint32_t error_offset;
        poly_coeff_t coef;
        if (!ParseCoefficient(p, &coef, &error_offset)) {
            fprintf(stderr, "ERROR %u %u\n", p->lexer.startLine, p->lexer.startColumn + error_offset);
            return PARSE_FAILURE;
        }
        *out = PolyFromCoeff(coef);
        return PARSE_SUCCESS;
    }
}


/**
 * Parsuje kolejną linię pliku.
 * Jeżeli po sparsowaniu oczekiwanej zawartości linii (wielomian lub polecenie) nie znajduje się w niej koniec wiersza,
 * zostanie wypluty komunikat: <c>ERROR r EOL EXPECTED</c>.
 * @param p parser źródłowy
 * @return informacje o wyniku parsowania
 */
static int ParseNextLine(Parser *p)
{
    int feedback;
    if (p->lexer.tokenType == TOKEN_COMMAND) {
        feedback = ParseCommand(p);
    } else {
        Poly poly;
        feedback = ParsePolynomial(p, &poly);
        CSPushPolynomial(&p->stack, poly);
    }

    if (!feedback)
        return PARSE_FAILURE;
    if (!LexerExpectChar(&p->lexer, '\n')) {
        fprintf(stderr, "ERROR %u EOL EXPECTED\n", (unsigned int)p->lexer.startLine);
        return PARSE_FAILURE;
    }
    return PARSE_SUCCESS;
}


void ParserPrepare(Parser *parser, FILE *source, FILE *output)
{
    parser->output = output;
    LexerInit(&parser->lexer, source);
}


int ParserExecuteAll(Parser *parser) {
    LexerReadNextToken(&parser->lexer);
    while (parser->lexer.tokenType != TOKEN_EOF) {
        if (!ParseNextLine(parser))
            return PARSE_FAILURE;
    }
    return PARSE_SUCCESS;
}


Parser ParserInit(void) {
    Parser result;
    result.stack = CSInit();
    return result;
}


void ParserDestroy(Parser *parser) {
    CSDestroy(&parser->stack);
}
