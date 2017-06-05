/** @file units_tests_poly_compose.h
 * Testy jednostkowe dla funkcji PolyCompose
 */
#ifndef WIELOMIANY_UNIT_TESTS_POLY_COMPOSE_H
#define WIELOMIANY_UNIT_TESTS_POLY_COMPOSE_H

/**
 * <c>p</c> wielomian zerowy, <c>count</c> równe <c>0</c>
 */
void TestPolyComposeZeroZero(void **state);

/**
 * <c>p</c> wielomian zerowy, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian stały
 */
void TestPolyComposeZeroConst(void **state);

/**
 * <c>p</c> wielomian stały, <c>count</c> równe <c>0</c>
 */
void TestPolyComposeConstZero(void **state);

/**
 * <c>p</c> wielomian stały, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian stały różny od <c>p</c>
 */
void TestPolyComposeConstConst(void **state);

/**
 * <c>p</c> wielomian liniowy, <c>count</c> równe <c>0</c>
 */
void TestPolyComposeLinZero(void **state);

/**
 * <c>p</c> wielomian liniowy, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian stały
 */
void TestPolyComposeLinConst(void **state);

/**
 * <c>p</c> wielomian liniowy, <c>count</c> równe <c>1</c>, <c>x[0]</c> wielomian linoowy
 */
void TestPolyComposeLinLin(void **state);

#endif //WIELOMIANY_UNIT_TESTS_POLY_COMPOSE_H
