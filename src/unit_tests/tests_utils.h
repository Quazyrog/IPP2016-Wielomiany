/** @file tests_utils.h
 * Funkcje potrzebne do testowania (na ogół obsługa atrap) + CMocka i inne potrzebne nagłówki.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <setjmp.h>
#include <cmocka.h>

#ifndef WIELOMIANY_TESTS_UTILS_H
#define WIELOMIANY_TESTS_UTILS_H

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
int mock_fprintf(FILE *stream, const char *format, ...);

/**
 * Atrapuje <c>fputc</c>
 */
int mock_fputc(int c, FILE *stream);

/**
 * Atrapuje <c>fgetc</c>
 */
int mock_fgetc(FILE *stream);

/**
 * Przygotowuje atrapę dla podanego strumienia, która spodziewa się podanych danych wyjściowych.
 * @param stream strumień atrapy
 * @param expect oczekiwane dane
 */
void UnitTestingExpectOutput(FILE *stream, const char *expect);

/**
 * Przygotowuje atrapę strumnienia wejściowego.
 * @param stream strumień wejściowy
 * @param input dane wejściowe
 * @param length długość danych wejściowych
 */
void UnitTestingFeedInput(FILE *stream, const char *input, unsigned int length);

/**
 * Sprawdza, czy wszystkie atrapy strumieni wyjściowych dostały poprawne oczekiwane dane.
 * @param ignore_other czy ignorować dane wypisane na strumienie bez atrap (jeżeli <c>false</c> to znaczy, że atrapami
 * są wszystkie używane strumienie)
 * @return czy wyjście jest zgodne z oczekiwanym
 */
bool UnitTestingCheckOutput(bool ignore_other);

#endif //WIELOMIANY_TESTS_UTILS_H
