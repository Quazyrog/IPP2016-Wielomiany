/** @file parser.h
 * Nagłówek parsera plików z instrukcjami kalkulatora
 */
#ifndef WIELOMIANY_PARSER_H
#define WIELOMIANY_PARSER_H

#include <stdio.h>
#include "lexer.h"
#include "calculator_stack.h"


///Zwracane przez funkcje parsujące po udanym sparsowaniu. Kod: <c>if (PARSE_SUCCESS) { ... }</c> zostanie wykonany
///(innymi słowy <c>PARSE_SUCCESS</c> to taki alias do <c>true</c>).
#define PARSE_SUCCESS 1

///Zwracane przez funkcje parsujące po nieudanym sparsowaniu. Kod: <c>if (PARSE_FAILURE) { ... }</c> nie zostanie
///wykonany (innymi słowy <c>PARSE_FAILURE</c> to taki alias do <c>false</c>).
#define PARSE_FAILURE 0


/**
 * Reprezentacja bierzącego stanu parsera.
 * Instancja powinna być utworzona za pomocą <c>ParserInit()</c> i usunięta za pomocą <c>ParserDestroy</c> (konwencja
 * zarządzania pamięcią taka jak w bibliotece do wielomianów).
 *
 * Może być używana do parsowania wielu plików. Wtedy kolejny parsowany plik na wejściu dostanie na stosie to, co
 * zostało tam po poprzednich parsowaniach.
 *
 * Podobnie jak w przypadku leksera, nie należy ręcznie modyfikować wartości w tej strukturze.
 * @warning Parser nie jest gotowy
 */
typedef struct {
    ///Strumień, do którego polecenie PRINT wypisuje dane
    FILE *output;

    ///Lekser używany przez parsera
    Lexer lexer;

    ///Stos wykonujący parsowane polecenia
    CalculatorStack stack;
} Parser;


/**
 * Inicjalizuje nową instancje struktury <c>Parser</c>.
 * @return Zainicjowaną strukturę
 */
Parser ParserInit(void);

/**
 * Niszczy podaną strukturę wraz z podstrukturami i zwalnia jej pamięć.
 * A przynajmniej jest tu poto, żeby to zrobić, jak będzie taka potrzeba. Wielokrotne wywołanie na jednej instancji
 * nie powinno sprawiać problemów.
 * @param parser struktura do usunięcia
 */
void ParserDestroy(Parser *parser);


/**
 * Przygotowuje parser do pracy z kolejnym plikiem.
 * Jeden parser może pracowac z wieloma plikami, ale nie jednocześnie. Parser nie zamyka plików podanych tutaj jako
 * parametry.
 * @param parser parser do pracy na pliku
 * @param source plik źródłowy z poleceniami
 * @param output strumień wyjściowy dla polecenia <c>PRINT</c>
 */
void ParserPrepare(Parser *parser, FILE *source, FILE *output);

/**
 * Wykonuje wszystkie polecenia zawarte w skojarzonym pliku.
 * Oczywiscie należy to wywołać po uprzednim wywołaniu <c>ParserPrepare()</c>. Nizastosowanie się do tego nie musi się
 * dobrze skończyć.
 * @param parser parser wykonując polecenia
 * @return <c>PARSE_SUCCESS</c> kiedy cały plik został pomyślnie sparsowany i wykonany; <c>PARSE_FAILURE</c> w.p.p.
 */
int ParserExecuteAll(Parser *parser);


#endif //WIELOMIANY_PARSER_H
