#pragma once

// единый набор категорий ошибки разбора числа.
// листовые (общие): not_number, out_of_range, not_finite
// специфичные:     too_few/too_much (точка), empty/not_positive (радиус)
enum class number_error {
    not_number,
    out_of_range,
    not_finite,
    too_few,
    too_much,
    empty,
    not_positive,
};