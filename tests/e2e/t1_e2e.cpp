#include <gtest/gtest.h>

#include <format>
#include <string>
#include <string_view>
#include <tuple>

#include "e2e_case.hpp"
#include "e2e_runner.hpp"
#include "t1_cases.hpp"

namespace {

// Единое имя: t1_norm -> t1_c_norm / t1_cpp_norm;
// common (version, arg_unknown) -> t1_c_version / t1_cpp_version.
std::string t1_test_name(e2e::ExeKind kind, std::string_view case_name) {
    const std::string lang = (kind == e2e::ExeKind::c) ? "c" : "cpp";
    if (case_name.starts_with("t1_")) {
        return "t1_" + lang + "_" + std::string(case_name.substr(3));
    }
    return "t1_" + lang + "_" + std::string(case_name);
}

// полное имя теста - с с/с++ и смысловым именем
std::string t1_param_name(const ::testing::TestParamInfo<std::tuple<e2e::ExeKind, e2e::Case>>& info) {
    const auto& [kind, c] = info.param;
    return t1_test_name(kind, c.name);
}

// suite-класс, тело будет заполнено на при раскрытии TEST_P
class T1E2E : public ::testing::TestWithParam<std::tuple<e2e::ExeKind, e2e::Case>> {};

// заполнение suit-класса данными
TEST_P(T1E2E, Matches) {
    // вид теста и структура Case на него
    const auto& [kind, c] = GetParam();

    const std::string id = t1_test_name(kind, c.name);
    const std::string exe = (kind == e2e::ExeKind::c) ? E2E_T1_C : E2E_T1_CPP;
    // запуск дочернего процесса exe
    const auto r =
        e2e::run(exe, c.args, std::format("{}/{}", E2E_INPUT_DIR, c.input),
                 std::format("{}/{}.out", E2E_RESULTS_DIR, id), std::format("{}/{}.err", E2E_RESULTS_DIR, id));
    // состояние обвязки
    ASSERT_FALSE(r.ec) << r.ec.message();

    // код процесса
    EXPECT_EQ(r.code, c.expected_code);

    // сравнение выходных данных - равны ли эталону?
    const std::string got = e2e::normalize(c.primary == e2e::Stream::out ? r.out : r.err);
    const std::string want = e2e::normalize(std::string(c.expected));
    EXPECT_EQ(got, want);

    // проверка, что ненормализованный обратный канал пуст
    EXPECT_TRUE((c.primary == e2e::Stream::out ? r.err : r.out).empty());
}

// сделать много инстансов класса T1E2E, наполнены они будут данными из t1_cases.hpp
INSTANTIATE_TEST_SUITE_P(All, T1E2E,
                         ::testing::Combine(::testing::Values(e2e::ExeKind::c, e2e::ExeKind::cpp),
                                            ::testing::ValuesIn(e2e::t1_cases)),
                         t1_param_name);

} // namespace
