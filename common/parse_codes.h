#ifndef IRINA_PARSE_CODES_H
#define IRINA_PARSE_CODES_H

// единый набор категорий ошибки разбора числа.
// листовые (общие): not_number, out_of_range, not_finite
// специфичные:     too_few/too_much (точка), empty/not_positive (радиус)

enum {
    NUMBER_OK = 0,
    NUMBER_NOT_NUMBER = 1,
    NUMBER_OUT_OF_RANGE = 2,
    NUMBER_NOT_FINITE = 3,
    NUMBER_TOO_FEW = 4,
    NUMBER_TOO_MUCH = 5,
    NUMBER_EMPTY = 6,
    NUMBER_NOT_POSITIVE = 7,
};

#endif