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

/**
 * Czy mają być robione testy pamięci z użyciem CMocka?
 */
#define DO_MEMORY_TESTS
#ifdef DO_MEMORY_TESTS

#define malloc(size) _test_malloc(size, __FILE__, __LINE__)
#define realloc(ptr, size) _test_realloc(ptr, size, __FILE__, __LINE__)
#define calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)

extern void* _test_malloc(const size_t size, const char* file, const int line);
extern void* _test_realloc(void *ptr, const size_t size, const char* file, const int line);
extern void* _test_calloc(const size_t num, const size_t size, const char* file, const int line);
extern void* _test_free(void *ptr, const char* file, const int line);
#endif

#define main tested_main
#define fprintf mock_fprintf
#define printf(...) mock_fprintf(stdout, __VA_ARGS__)
#define fputc mock_fputc
#define scanf(format, ...) mock_scanf(format"%n", ##__VA_ARGS__, &read_char_count)
#define exit(code) fail_msg("Exited with code %i", code)
#define fgetc mock_fgetc

extern int ReadCharCount;
extern void fail_msg(const char *msg, ...);
int mock_scanf(const char *format, ...);
int mock_fprintf(FILE *stream, const char *format, ...);
int mock_fgetc(FILE *stream);
int mock_fputc(int c, FILE *stream);

#endif //WIELOMIANY_MOCK_TESTS_TRICKS_H
#endif //UNIT_TESTING
