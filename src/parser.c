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


/**
 * Parsuje liczbę całkowitą typu <c>long long int</c>.
 * W przypadku niepowodzenia nie wypisuje żadanych komunikatów błędowych, a jedynie zwraca <c>PARSE_FAILURE</c>. Nie ma
 * gwarancji, że wówczas zawartość <c>out</c> nie ulegnie zmianie.
 * @param p parser źródłowy
 * @param out miejsce w pamięci, do którego zapisze wynik
 * @return informację o powodzeniu parsowania
 */
static int ParseInteger(Parser *p, long long int *out)
{
    char buf[LEXER_TOKEN_BUFFER_SIZE + 1];
    char *load_ptr = buf;

    if (p->lexer.tokenType == TOKEN_SPECIAL_CHAR) {
        if (p->lexer.tokenBuffer[0] == '-') {
            *load_ptr = '-';
            ++load_ptr;
        } else if (p->lexer.tokenBuffer[0] != '+') {
            return PARSE_FAILURE;
        }
        LexerReadNextToken(&p->lexer);
    }
    if (p->lexer.tokenType != TOKEN_NUMBER)
        return PARSE_FAILURE;
    memcpy(load_ptr, p->lexer.tokenBuffer, LEXER_TOKEN_BUFFER_SIZE);

    *out = strtoll(buf, NULL, 10);
    if (errno == ERANGE)
        return PARSE_FAILURE;

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
        long long int arg;
        LexerReadNextToken(&p->lexer);
        if (!LexerExpect(&p->lexer, " ") || !ParseInteger(p, &arg) || p->lexer.tokenType != TOKEN_SEPARATOR) {
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
