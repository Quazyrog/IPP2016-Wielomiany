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
 * Parsuje współczynnik jednomianu.
 * To mogłoby byc po prostu ParseIntegere, ALE:
 *   - ParseInteger parsuje <c>long long int</c>, więc mogło by nie wykryć overflowu
 *   - Powyższe rzecz jasna nie byłoby problemem, bo można zrzucowac na <c>poly_coeff_t</c> i wtedy wykryć overflow.
 *     ALE musimy wiedzieć, która cyfra powoduje overflowa, co wszystko psuje: musimy mieć własne ewaluowanie liczb
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
        *invalid_digit = 0;
        return PARSE_FAILURE;
    }
    const char *digits = p->lexer.tokenBuffer;
    poly_coeff_t value = 0;

    for (uint32_t next_digit = 0; digits[next_digit] != 0; ++next_digit) {
        poly_coeff_t digit = digits[next_digit] - '0';
        poly_coeff_t new_value;

        new_value = value * 10;
        if (new_value / 10 != value) {
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
 * Parsuje liczbę całkowitą typu <c>long long unsigned int</c>.
 * W przypadku niepowodzenia nie wypisuje żadanych komunikatów błędowych, a jedynie zwraca <c>PARSE_FAILURE</c>. Nie ma
 * gwarancji, że wówczas zawartość <c>out</c> nie ulegnie zmianie.
 * @param p parser źródłowy
 * @param out miejsce w pamięci, do którego zapisze wynik
 * @return informację o powodzeniu parsowania
 */
static int ParseUnsigned(Parser *p, long long unsigned int *out)
{
    if (p->lexer.tokenType != TOKEN_NUMBER)
        return PARSE_FAILURE;

    *out = strtoull(p->lexer.tokenBuffer, NULL, 10);
    if (errno == ERANGE)
        return PARSE_FAILURE;

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
        if (!LexerExpect(&p->lexer, " ") || !ParseCoefficient(p, &arg, NULL) || p->lexer.tokenType != TOKEN_SEPARATOR) {
            fprintf(stderr, "ERROR %u WRONG VALUE\n", (unsigned int)p->lexer.startLine);
            return PARSE_FAILURE;
        }
        //TODO wykonaj AT
        printf("at(%lli)\n", arg);

    } else if (strcmp(p->lexer.tokenBuffer, "DEG_BY") == 0) {
        long long unsigned int arg;
        LexerReadNextToken(&p->lexer);
        if (!LexerExpect(&p->lexer, " ") || !ParseUnsigned(p, &arg) || p->lexer.tokenType != TOKEN_SEPARATOR) {
            fprintf(stderr, "ERROR %u WRONG VALUE\n", (unsigned int)p->lexer.startLine);
            return PARSE_FAILURE;
        }
        //TODO wykonaj DEG_BY
        printf("degby(%llu)\n", arg);

    } else {
        //TODO reszta poleceń
        printf("cmd(%s)\n", p->lexer.tokenBuffer);
        LexerReadNextToken(&p->lexer);
    }

    return PARSE_SUCCESS;
}


/**
 * Parsuje wielomian.
 * W skład wielomianun wchodzi liczba ze znakiem, lub sekwencja jednomianów oddzielonych znakiem '+'.
 * @param p parser źródłowy
 * @param out miejsce w pamięci, do którego zostanie zapisany wynik
 * @return informacje o powodzeniu parsowania
 */
static int ParsePolynomial(Parser *p, Poly *out)
{
    //TODO ...
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
    if (p->lexer.tokenType == TOKEN_COMMAND)
        feedback = ParseCommand(p);
    else
        feedback = ParsePolynomial(p, NULL); //FIXME nie NULL

    if (!feedback)
        return PARSE_FAILURE;
    if (!LexerExpect(&p->lexer, "\n")) {
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
    //TODO będzie być może mądrzejsza, kiedy parser będzie potrzebowac stosu
    Parser result;
    return result;
}


void ParserDestroy(Parser *parser) {
    //TODO będzie na pewno mądrzejsza, kiedy parser będzie potrzebowac stosu
}
