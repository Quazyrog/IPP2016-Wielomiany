cmake_minimum_required(VERSION 3.4)
project(wielomiany C)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

# Nie chcemy oglądać komunikatów o procentowym postępie kompilacji.
set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

# Chcemy zobaczyć polecenia wywołane przez make.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Ustawiamy wspólne opcje kompilowania dla wszystkich wariantów projektu.
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra")
# Domyślne opcje dla wariantów Release i Debug są sensowne.
# Jeśli to konieczne, ustawiamy tu inne.
# set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
# set(CMAKE_C_FLAGS_DEBUG "-g")

# Wskazujemy pliki źródłowe.
set(SOURCE_FILES_COMMON
        src/poly.c
        src/poly.h)

set(SOURCE_FILES_POLY_TEST_ONLY
        src/test_poly.c
        src/test_poly_core.h
        src/test_poly_core.c
        src/test_poly.c
        src/const_arr.h)

set(SOURCE_FILES_CALC_ONLY
        src/lexer.h
        src/lexer.c
        src/parser.h
        src/parser.c
        src/calculator_stack.h
        src/calculator_stack.c
        src/main.c)

# Wskazujemy plik wykonywalny.
add_executable(calc_poly ${SOURCE_FILES_COMMON} ${SOURCE_FILES_CALC_ONLY})
add_executable(test_poly ${SOURCE_FILES_COMMON} ${SOURCE_FILES_POLY_TEST_ONLY})

# Dodajemy obsługę Doxygena: sprawdzamy, czy jest zainstalowany i jeśli tak to:
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # Wskazujemy lokalizacją pliku konfiguracyjnego i podajemy jego docelową lokalizację w folderze, gdzie następuje kompilacja.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    # Dodajemy cel doc: użycie make doc spowoduje wywołanie doxygena z odpowiednim plikiem konfiguracyjnym w folderze kompilacji.
    # Na wyjście zostanie wypisany odpowiedni komentarz.
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif (DOXYGEN_FOUND)

#Testy w CMocka
enable_testing()
find_library(CMOCKA_FOUND cmocka)
if (CMOCKA_FOUND)
    message("CMocka so found: " ${CMOCKA_FOUND})
    get_filename_component(CMOCKA_LIBRARY_DIR ${CMOCKA_FOUND} DIRECTORY)
    set(SOURCE_FILES_UNIT_TESTS
            src/unit_tests_poly.c
            src/mock_tricks.h
            )

    add_executable(unit_tests_poly ${SOURCE_FILES_COMMON} ${SOURCE_FILES_CALC_ONLY} ${SOURCE_FILES_UNIT_TESTS})
    target_compile_definitions(unit_tests_poly PUBLIC -DUNIT_TESTING)
    target_link_libraries(unit_tests_poly -L${CMOCKA_LIBRARY_DIR} -lcmocka)
    add_test(NAME CMockaPolyUnitTests COMMAND unit_tests_poly)
else()
    message("Cannot find CMocka shared object file")
endif (CMOCKA_FOUND)
