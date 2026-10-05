#include <gtest/gtest.h>

#include <ostream>
#include <string_view>

#include "printers.hpp"
#include "../../common/geometry.hpp"
#include "../../common/parse_codes.hpp"


namespace {
// тип данных. вход в виде строки и ожидаема€ точка. нормальный случай
struct OkCase {
    std::string_view in;
    Point want;
};

// класс-фикстура (suite) - тк TEST_P хочет именно такое первым параметром
// публично наследуемс€ от TestWithParam<OkCase>, который задаЄт тип данных параметра - OkCase
// сам по себе ParsePointOk пуст
class ParsePointOk : public ::testing::TestWithParam<OkCase> {};

// TEST_* - макрос-генератор шаблонного кода (делает регистрацию в реестре gtest, и др. - не надо это писать руками)
// _P - с параметрами
// генераци€ публичного наследника от ParsePointOk - ParsePointOk_Parses_Test
// (в TestBody будет подставленно тело самого TEST_P)
// второй парам. (Parses) - им€ теста
// в gtest каждый TEST_P даст отдельный класс, наследуемый от класса-фикстуры
TEST_P(ParsePointOk, Parses) {
    // когда данные есть, берЄт каждое следующее из таблицы с данными. результ. - в got
    const OkCase& c = GetParam();
    auto got = parse_point(c.in);
    // без значени€ *got читать нельз€ (UB), поэтому ASSERT - тест прервЄтс€ если така€ ситуаци€
    ASSERT_TRUE(got.has_value());
    // при несовпадении пишет результат и идЄт дальше
    EXPECT_EQ(*got, c.want);
}

// регистраци€ данных Values под ключом ParsePointOk
// от него gtest при run создаст экземпл€ры класса ParsePointOk_Parses_Test
// ParsePointOk_Parses_Test имеет ключ ParsePointOk и эти данные, зарегистр. через INSTANTIATE - тоже
// поэтому у них будет join и будут сделаны инстансы ParsePointOk_Parses_Test именно с данными с ключом ParsePointOk
INSTANTIATE_TEST_SUITE_P(OkCases, ParsePointOk,
                         ::testing::Values(OkCase{"1 2 3", Point{1, 2, 3}}, OkCase{"1.5 2.5 3.5", Point{1.5, 2.5, 3.5}},
                                           OkCase{" 1.5 2.5 3.5 ", Point{1.5, 2.5, 3.5}},
                                           OkCase{"+1 2 3", Point{1, 2, 3}}, OkCase{"3e+2 0 0", Point{300, 0, 0}},
                                           OkCase{"1E2 0 0", Point{100, 0, 0}}, OkCase{"1e-2 0 0", Point{0.01, 0, 0}},
                                           OkCase{".5 2 3", Point{0.5, 2, 3}}, OkCase{"5. 2 3", Point{5, 2, 3}},
                                           OkCase{"0 0 0", Point{0, 0, 0}}, OkCase{"1e-320 0 0", Point{1e-320, 0, 0}}));

// вход в виде строки и ожидаемый код ошибки. ошибочный случай
struct ErrCase {
    std::string_view in;
    number_error want;
};

class ParsePointErr : public ::testing::TestWithParam<ErrCase> {};

TEST_P(ParsePointErr, Rejects) {
    const ErrCase& c = GetParam();
    auto got = parse_point(c.in);
    ASSERT_FALSE(got.has_value());
    EXPECT_EQ(got.error(), c.want);
}

INSTANTIATE_TEST_SUITE_P(
    ErrCases, ParsePointErr,
    ::testing::Values(
        // мало координат
        ErrCase{"1 2", number_error::too_few}, ErrCase{"1", number_error::too_few}, ErrCase{"", number_error::too_few},
        ErrCase{"   ", number_error::too_few},

        // слишком много координат
        ErrCase{"1 2 3 4", number_error::too_much}, ErrCase{"1 2 3 nan", number_error::too_much},

        // не число
        ErrCase{"1 2 x", number_error::not_number}, ErrCase{"abc 2 3", number_error::not_number},
        ErrCase{"0x10 2 3", number_error::not_number}, ErrCase{"0X10 2 3", number_error::not_number},
        ErrCase{"1,5 2 3", number_error::not_number}, ErrCase{"+-3 2 3", number_error::not_number},

        // вне диапазона double
        ErrCase{"1e400 0 0", number_error::out_of_range}, ErrCase{"1e-400 0 0", number_error::out_of_range},

        // не-конечное
        ErrCase{"inf 0 0", number_error::not_finite}, ErrCase{"nan 0 0", number_error::not_finite},
        ErrCase{"infinity 0 0", number_error::not_finite}, ErrCase{"NAN(2) 0 0", number_error::not_finite},

        // регресс: \v / \f перед hex-префиксом
        ErrCase{"\v0x10 2 3", number_error::not_number}, ErrCase{"\f0X10 2 3", number_error::not_number}));

// дл€ gtest - "как печатать объект типа OkCase"
std::ostream& operator<<(std::ostream& os, const OkCase& c) {
    os << "in=\"" << c.in << "\", want=" << ::testing::PrintToString(c.want);
    return os;
}

// дл€ gtest - "как печатать объект типа ErrCase"
std::ostream& operator<<(std::ostream& os, const ErrCase& c) {
    os << "in=\"" << c.in << "\", want=code " << static_cast<int>(c.want);
    return os;
}
} // namespace
