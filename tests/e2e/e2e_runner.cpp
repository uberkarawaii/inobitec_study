#include "e2e_runner.hpp"

#include <boost/process/v1.hpp>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <print>
#include <string>
#include <system_error>
#include <vector>

namespace e2e {
namespace {

// сокращение namespace
namespace bp = boost::process::v1;

struct file_closer {
    void operator()(std::FILE* f) const noexcept {
        if (f)
            std::fclose(f);
    }
};

using file_ptr = std::unique_ptr<std::FILE, file_closer>;

// открытие файла - под виндой с fopen_s т.к. fopen провоцирует warning, небезопасный
// под линуксом fopen_s не существует
#ifdef _MSC_VER
std::FILE* open_file(const char* path, const char* mode) {
    std::FILE* f = nullptr;
    const errno_t rc = ::fopen_s(&f, path, mode);
    if (rc != 0) {
        errno = rc;
        return nullptr;
    }
    return f;
}
#else
std::FILE* open_file(const char* path, const char* mode) { return std::fopen(path, mode); }
#endif

// чтение файла в виде байтов
std::string read_file_binary(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
}

// private, запуск ребЄнка
run_result run_impl(const std::string& exe1, const std::vector<std::string>& args1, std::FILE* in,
                    const std::string& out_path, const std::string& err_path) {
    run_result r;
    file_ptr out(open_file(out_path.c_str(), "wb"));
    file_ptr err(open_file(err_path.c_str(), "wb"));
    if (!out || !err) {
        r.ec = std::error_code(errno, std::generic_category());
        std::println(stderr, "cannot open output files: {}", r.ec.message());
        return r;
    }

    bp::child c(exe1, bp::args = args1, bp::std_in<in, bp::std_out> out.get(), bp::std_err > err.get(), r.ec);
    if (r.ec)
        return r;
    c.wait(r.ec);
    if (r.ec)
        return r;
    r.code = c.exit_code();
    r.native = c.native_exit_code();
    // во избежание проблем на винде, если какой-то из файлов ещЄ открыт (тогда его невозможно будет прочесть)
    out.reset();
    err.reset();
    r.out = read_file_binary(out_path);
    r.err = read_file_binary(err_path);
    return r;
}

} // namespace

// нормализованный текст - без кареток и без последнего \n
std::string normalize(std::string s) {
    std::erase(s, '\r');
    while (!s.empty() && s.back() == '\n')
        s.pop_back();
    return s;
}

// обычный файл: вход открываем сами, только на чтение
run_result run(const std::string& exe1, const std::vector<std::string>& args1, const std::string& in_path,
               const std::string& out_path, const std::string& err_path) {
    file_ptr in(open_file(in_path.c_str(), "rb"));
    if (!in) {
        run_result r;
        r.ec = std::error_code(errno, std::generic_category());
        std::println(stderr, "cannot open input {}: {}", in_path, r.ec.message());
        return r;
    }
    return run_impl(exe1, args1, in.get(), out_path, err_path);
}

// чтобы подать на вход каталог
// stdin передаЄтс€ уже открытым FILE* Ч boost не открывает путь сам
// уже открытый stdin (каталог)
run_result run_with_stdin(std::FILE* in, const std::string& exe1, const std::vector<std::string>& args1,
                          const std::string& out_path, const std::string& err_path) {
    return run_impl(exe1, args1, in, out_path, err_path);
}
} // namespace e2e
