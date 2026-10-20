#include <gtest/gtest.h>

#include <string>
#include <tuple>

#include "e2e_case.hpp"
#include "e2e_test.hpp"
#include "t1_cases.hpp"

namespace {

std::string t1_param_name(const e2e::ParamInfo& info) { return e2e::param_name("t1", info); }

// suite-класс, тело будет заполнено на при раскрытии TEST_P
class T1E2E : public ::testing::TestWithParam<std::tuple<e2e::ExeKind, e2e::Case>> {};

// заполнение suit-класса данными
// это шаблонное для t1-t4 действие, потому оно находится в e2e_test.hpp inline expect_case_matches
TEST_P(T1E2E, Matches) {
    const auto& [kind, c] = GetParam();
    e2e::expect_case_matches("t1", kind, c, E2E_EXE_C, E2E_EXE_CPP, E2E_INPUT_DIR, E2E_RESULTS_DIR);
}

// сделать много инстансов класса T1E2E, наполнены они будут данными из t1_cases.hpp
INSTANTIATE_TEST_SUITE_P(All, T1E2E,
                         ::testing::Combine(::testing::Values(e2e::ExeKind::c, e2e::ExeKind::cpp),
                                            ::testing::ValuesIn(e2e::t1_cases)),
                         t1_param_name);
} // namespace
