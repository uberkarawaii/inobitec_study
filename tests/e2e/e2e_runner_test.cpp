#include <gtest/gtest.h>

#include <format>
#include <string>
#include <system_error>

#include "e2e_runner.hpp"

TEST(E2ERunner, MissingInput) {
    auto r = e2e::run("dummy", {}, std::format("{}/no_such_input", E2E_INPUT_DIR),
                      std::format("{}/a.out", E2E_RESULTS_DIR), std::format("{}/a.err", E2E_RESULTS_DIR));
    EXPECT_EQ(r.ec, std::make_error_code(std::errc::no_such_file_or_directory));
    EXPECT_EQ(r.code, -1);
    EXPECT_TRUE(r.out.empty());
    EXPECT_TRUE(r.err.empty());
}

TEST(E2ERunner, MissingOutputDir) {
    auto r = e2e::run("dummy", {}, std::format("{}/t1_nul", E2E_INPUT_DIR),
                      std::format("{}/no_such_dir/a.out", E2E_RESULTS_DIR), std::format("{}/a.err", E2E_RESULTS_DIR));
    EXPECT_EQ(r.ec, std::make_error_code(std::errc::no_such_file_or_directory));
    EXPECT_EQ(r.code, -1);
}

TEST(E2ERunner, MissingErrorDir) {
    auto r = e2e::run("dummy", {}, std::format("{}/t1_nul", E2E_INPUT_DIR), std::format("{}/a.out", E2E_RESULTS_DIR),
                      std::format("{}/no_such_dir/a.err", E2E_RESULTS_DIR));
    EXPECT_EQ(r.ec, std::make_error_code(std::errc::no_such_file_or_directory));
    EXPECT_EQ(r.code, -1);
}
