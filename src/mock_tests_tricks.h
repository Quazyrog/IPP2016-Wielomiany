/** @file mock_tests_tricks.h
 * Plik pomocniczy testów jednostkowych.
 * Zawiera makra potrzebne do przesłonięcia odpowiednich funkcji ich atrapami. Przesłania funkcje:
 * - <c>main</c>
 * - <c>fprintf</c>
 * - <c>printf</c>
 */
#ifdef UNIT_TESTING
#ifndef WIELOMIANY_MOCK_TESTS_TRICKS_H
#define WIELOMIANY_MOCK_TESTS_TRICKS_H

#define main tested_main
#define fprintf mock_fprintf
#define printf(...) mock_fprintf(stdout, __VA_ARGS__)

int mock_fprintf(FILE *stream, const char *format, ...);

#endif //WIELOMIANY_MOCK_TESTS_TRICKS_H
#endif //UNIT_TESTING
