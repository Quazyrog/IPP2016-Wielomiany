/** @file lexer.h
 * Plik nagłówkowy dla struktury Lexer.
 * Celem struktury jest ułatwienie czytania plików wejściowych, przez grupowanie czytanych znaków w tokeny, oraz
 * śledzenie bierzącej pozycji w pliku.
 */
#ifndef WIELOMIANY_LEXICALSCANER_H
#define WIELOMIANY_LEXICALSCANER_H

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

/**
 * Rozmiar bufrou na przechowywanie tokenu w lekserze.
 * Najdłuższe tokeny to liczby i polecenia, a te z pewnością powinny sie mieścić w takim buforze.
 */
#define LEXER_TOKEN_BUFFER_SIZE 256


/**
 * Rodzaje tokenów rozróżnaniane przez lekser.
 * (C cały czas mnie zaskakuje. Nie wiedziałem, ze ma <c>enum</c>.)
 */
typedef enum
{
    /** Token złożony w całości z cyfr */
    TOKEN_NUMBER,

    /** Token złożony w całości z liczb oraz podkreślników */
    TOKEN_COMMAND,

    /** Nawiasy, przecinek, plus i minus; zawsze tylko jeden znak. */
    TOKEN_SPECIAL_CHAR,

    /** Spacja lub złamanie linii; zawsze tylko jeden znak. */
    TOKEN_SEPARATOR,

    /** Koniec pliku; token jest pusty */
    TOKEN_EOF,

    /** Zawsze tylko jeden znak, który nie podpada pod żaden z powyższych typów tokenów. */
    TOKEN_INVALID,

    /** Ustawiane, kiedy token wczytany na wejściu nie mieści się w buforze; bufor jest wtedy zerowany. */
    TOKEN_INVALID_OVERFLOW
} LexerTokenType;

/**
 * Struktura przechowująca stan leksera.
 *
 * Przygotowanie do pracy z każdym kolejnym plikiem odbywa się przez wywołanie funkcji LexerInit(). Poza tym nie wymaga
 * żadnej dodatkowej specjalnej inicjalizacji, ani niszczenia. Jedna instancja może być używana wielokrotnie,
 * z różnymi plikami.
 *
 * Nie należy ręcznie modyfikować wartości w tej strukturze.
 */
typedef struct
{
    /// Następny znak w pliku.
    /// Ponieważ struktura dopiero po odczytaniu znaku wie, czy powinien on rozpocząć następny token, musi on zostać
    /// w taki sposób przechowany. Zaraz po rozpoczęciu pracy z plikiem ustawiony na 0)
    char nextChar;

    ///Linia, z której został przeczytany ostatni znak (ten przechowywany w <c>nextChar</c>)
    uint32_t line;
    ///Kolumna, z której został przeczytany ostatni znak (ten przechowywany w <c>nextChar</c>)
    uint32_t column;

    ///Linia, z której został przeczytany pierwszy znak ostatnio przeczytanego tokenu
    uint32_t startLine;
    ///Kolumna, z której został przeczytany pierwszy znak ostatnio przeczytanego tokenu
    uint32_t startColumn;

    ///Typ ostatnio przeczytanego tokenu
    LexerTokenType tokenType;

    ///Wskaźnik do pliku ze źródłem dla leksera
    FILE *input;

    ///Bufor do przechowywania tokenu
    char tokenBuffer[LEXER_TOKEN_BUFFER_SIZE];
} Lexer;


/**
 * Resetuje stan leksera, by przygotować go do pracy z kolejnym plikiem.
 * Żadna operacja nie jest wykonywana na starym pliku (w szczególności nie jest zamykany, ani nie sprawdzamy dostępności
 * dalszych danych).
 * @param scanner struktura do zainicjowania
 * @param file plik, z którego lekser będzie wczytywać dane
 */
void LexerInit(Lexer *scanner, FILE *file);

/**
 * Wczytuje do bufora leksera kolejny token z pliku wejściowego.
 * Kiedy lekser napotkał juz koniec pliku, nie jest modyfikowany i funkcja kończy działanie. W przypadku, gdy token
 * okaże się zbyt długi do umieszczenia w buforze, bufor zostanie wyzerowany (pierwszy znak), a typ odczytanego tokenu
 * ustawiony na <c>TOKEN_INVALID_OVERFLOW</c>.
 * @param scanner struktura przechowująca stan leksera
 */
void LexerReadNextToken(Lexer *scanner);

#endif //WIELOMIANY_LEXICALSCANER_H
