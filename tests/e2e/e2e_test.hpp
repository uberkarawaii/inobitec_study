#pragma once

#include <gtest/gtest.h>

#include <cerrno>
#include <format>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>

#include "e2e_case.hpp"
#include "e2e_runner.hpp"

namespace e2e {

// тип параметра для всех матричных suite: сценарий + реализация (C/C++)
using E2EParam = std::tuple<ExeKind, Case>;
using ParamInfo = ::testing::TestParamInfo<E2EParam>;

// единое правило имени: t2_norm -> t2_c_norm / t2_cpp_norm;
// общие сценарии (version, arg_unknown) -> t2_c_version / t2_c_arg_unknown
inline std::string test_name(std::string_view prefix, ExeKind kind, std::string_view case_name) {
    std::string id;
    id.reserve(prefix.size() + case_name.size() + 8);
    id.append(prefix);
    id += '_';
    id += (kind == ExeKind::c ? "c" : "cpp");
    id += '_';

    if (case_name.starts_with(prefix) && case_name.size() > prefix.size() && case_name[prefix.size()] == '_')
        id.append(case_name.substr(prefix.size() + 1));
    else
        id.append(case_name);
    return id;
}

// адаптер для INSTANTIATE_TEST_SUITE_P: стабильные и читаемые имена
inline std::string param_name(std::string_view prefix, const ParamInfo& info) {
    const auto& [kind, c] = info.param;
    return test_name(prefix, kind, c.name);
}

// одно тело проверки для всех tN_e2e.cpp:
// запуск -> код -> основной канал -> пустота обратного канала.
// ASSERT_* допустимы: функция void, return из неё завершает и TEST_P.
inline void expect_case_matches(std::string_view prefix, ExeKind kind, const Case& c, std::string_view exe_c,
                                std::string_view exe_cpp, std::string_view input_dir, std::string_view results_dir) {
    const std::string id = test_name(prefix, kind, c.name);
    const std::string exe = (kind == ExeKind::c) ? std::string(exe_c) : std::string(exe_cpp);
    const std::string out_path = std::format("{}/{}.out", results_dir, id);
    const std::string err_path = std::format("{}/{}.err", results_dir, id);

    run_result r;
    if (c.input_is_dir) {
        // stdin-каталог (io-fail) открываем сами: boost не должен открывать его заново
        file_ptr in = open_input(std::string(input_dir));
        ASSERT_TRUE(in) << "cannot open stdin directory: " << std::error_code(errno, std::generic_category()).message();
        r = run_with_stdin(in.get(), exe, c.args, out_path, err_path);
    } else {
        const std::string in_path = std::format("{}/{}", input_dir, c.input);
        r = run(exe, c.args, in_path, out_path, err_path);
    }

    ASSERT_FALSE(r.ec) << r.ec.message();

    EXPECT_EQ(r.code, c.expected_code);

    const std::string got = normalize(std::string(c.primary == Stream::out ? r.out : r.err));
    const std::string want = normalize(std::string(c.expected));
    EXPECT_EQ(got, want);

    const std::string& other = (c.primary == Stream::out) ? r.err : r.out;
    EXPECT_TRUE(other.empty()) << "reverse channel (" << (c.primary == Stream::out ? "stderr" : "stdout")
                               << ") is not empty: " << other.size() << " byte(s)";
}

} // namespace e2e
