/** @file calc_compose.h
 * Testy całego programu pod kątem funkcji PolyCompose()
 */
#ifndef WIELOMIANY_CALC_COMPOSE_H
#define WIELOMIANY_CALC_COMPOSE_H

/**
 * <c>COMPOSE</c> bez parametru.
 */
void TestCalcComposeNoParam(void **state);

/**
 * <c>COMPOSE 0</c>
 */
void TestCalcComposeZero(void **state);

/**
 * <c>COMPOSE UINT_MAX</c>
 */
void TestCalcComposeUMax(void **state);

/**
 * <c>COMPOSE -1</c>
 */
void TestCalcComposeNegative(void **state);

/**
 * <c>COMPOSE UINT_MAX + 1</c>
 */
void TestCalcComposeOverflow(void **state);

/**
 * <c>COMPOSE UINT_MAX + dużo</c>
 */
void TestCalcComposeOooveeerflooow(void **state);

/**
 * <c>COMPOSE kapibara</c>
 */
void TestCalcComposeCapybara(void **state);

/**
 * <c>COMPOSE 44kapibary</c>
 */
void TestCalcCompose44Capybaras(void **state);

/**
 * Testy compose z przykładu
 */
void TestCalcComposeExample(void **state);

#endif //WIELOMIANY_CALC_COMPOSE_H
