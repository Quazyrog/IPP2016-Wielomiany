/** @file lexer.c
 * Implementacja leksera.
 */
#include "lexer.h"
#include <ctype.h>
#include <memory.h>


/**
 * Zwraca rodzaj tokenu do jakiego może należeć dany znak.
 * Sytuacja jest o tyle prosta, że według specyfikacji każdy znak może odpowiadać tylko jednemu tokenowi.
 * @param c odpytywany znak
 * @return rodzaj tokenu, w którego skład może wejść <c>c</c>
 */
static LexerTokenType LSCharacterTokenType(char c)
{

    if (isdigit(c))
        return TOKEN_NUMBER;
    if (isalnum(c) || c == '_')
        return TOKEN_COMMAND;
    if (c == ' ' || c == '\n')
        return TOKEN_SEPARATOR;
    if (c == EOF)
        return TOKEN_EOF;
    if (c == '(' || c == ')' || c == '+' || c == '-' || c == ',')
        return TOKEN_SPECIAL_CHAR;
    return TOKEN_INVALID;
}

/**
 * Zwraca <code>true</code>, gdy tokeny podanego typu powinny się składać zawsze z (co najwyżej) jednego znaku.
 * @param t odpytywany rodzaj tokenu
 * @return <c>true</c>, kiedy token typu <c>t</c> powinien składac się z co najwyżej jednego znaku; <c>false</c> w.p.p.
 */
static inline int LSIsMonocharToken(LexerTokenType t)
{
    return t != TOKEN_NUMBER && t != TOKEN_COMMAND;
}


void LexerInit(Lexer *scanner, FILE *file)
{
    scanner->line = 1;
    scanner->column = 1;

    scanner->input = file;
    scanner->tokenType = TOKEN_SEPARATOR;
    scanner->nextChar = 0;
    memset(scanner->tokenBuffer, 0, sizeof(scanner->tokenBuffer));
}


void LexerReadNextToken(Lexer *scanner)
{
    if (scanner->tokenType == TOKEN_EOF)
        return;

    scanner->startLine = scanner->line;
    scanner->startColumn = scanner->column;
    scanner->tokenBuffer[0] = 0;
    if (scanner->nextChar == 0)
        scanner->nextChar = fgetc(scanner->input);
    scanner->tokenType = LSCharacterTokenType(scanner->nextChar);

    int length = 0;
    do {
        scanner->tokenBuffer[length] = scanner->nextChar;
        ++length;
        if (length == LEXER_TOKEN_BUFFER_SIZE) {
            scanner->tokenType = TOKEN_INVALID_OVERFLOW;
            scanner->tokenBuffer[0] = 0;
            return;
        }
        ++scanner->column;

        if (scanner->nextChar == '\n') {
            ++scanner->line;
            scanner->column = 1;
        }
        scanner->nextChar = fgetc(scanner->input);

        if (LSIsMonocharToken(scanner->tokenType))
            break;
    } while (LSCharacterTokenType(scanner->nextChar) == scanner->tokenType);

    scanner->tokenBuffer[length] = 0;
}


