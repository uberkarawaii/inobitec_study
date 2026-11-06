#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <format>
#include <fstream>
#include <ios>
#include <optional>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "common/exit_codes.hpp"
#include "coverage_union.hpp"

namespace {

// константа для печати help
constexpr std::string_view kUsage = "usage: coverage_union --cobertura <file> [--native <file>] [--modules <file>] "
                                    "[--source-root <dir>] [--expected <file>] [--output <file>]";

// регистр путей в зависимости от ОС - на виндоус приводим к lower-case, под линуксом оставляем в изначальном виде
#if defined(_WIN32)
constexpr bool kFoldCase = true;
#else
constexpr bool kFoldCase = false;
#endif

// generic_category - текстовое описание ошибок со стандартными кодами
std::string system_error_text(int code) { return std::generic_category().message(code); }

// чтение файла в бинарном режиме (для cobertura.xml и манифеста с путями файлов для покрытия)
// при ошибке открытия или чтения - std::nullopt - объект не содержит никакого значения
// пустой файл - не ошибка. в т.ч. поэтому не проверяем failbit, чтобы не упасть на пустом файле
// поэтому " если чтение упадёт с failbit, но без badbit, код этого не заметит". это ок для локальных файлов
std::optional<std::string> read_file(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) {
        std::println(stderr, "cannot open '{}': {} (errno {})", path, system_error_text(errno), errno);
        return std::nullopt;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (in.bad()) {
        std::println(stderr, "cannot read '{}': {} (errno {})", path, system_error_text(errno), errno);
        return std::nullopt;
    }
    return buffer.str();
}

// для манифеста с путями файлов для покрытия. из одной строки с манифестом делает мн-во строк.
// разбиение по \n, срезает хвостовой \r
std::vector<std::string> split_lines(std::string_view text) {
    std::vector<std::string> lines;
    std::size_t pos = 0;
    while (pos < text.size()) {
        const std::size_t end = text.find('\n', pos);
        std::string_view line = text.substr(pos, end == std::string_view::npos ? text.size() - pos : end - pos);
        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);
        if (!line.empty())
            lines.emplace_back(line);
        if (end == std::string_view::npos)
            break;
        pos = end + 1;
    }
    return lines;
}

// пишет отчёт в файл в binary-режиме
// если упадёт во время записи, отчёт будет частичным
bool write_file(const std::string& path, std::string_view text) {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) {
        std::println(stderr, "cannot open '{}' for writing: {} (errno {})", path, system_error_text(errno), errno);
        return false;
    }
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!out) {
        std::println(stderr, "cannot write '{}': {} (errno {})", path, system_error_text(errno), errno);
        return false;
    }
    return true;
}

// cobertura - путь к cobertura.xml
// source_root - для путей в отчёте. если пустой, пути абсолютные
// expected - путь к манифесту с файлами, которые нужны в покрытии. если пустой, presence check не выполнится
// output - куда выводить отчёт. если пустой - только в stdout, если непустой - то ещё и в указанный файл
// native - файл native xml
// modules - манифест с модулями, которые должны быть покрыты
struct Options {
    std::string cobertura;
    std::string native;
    std::string modules;
    std::string source_root;
    std::string expected;
    std::string output;
};

// обрабатывает вызов с флагами, а вызов мы делаем сами из coverage.cmake
// возвратит nullopt если есть: повторение флагов; отсутствие --cobertura; аргумент без флага
// пустой аргумент; отсутствующий аргумент
// не умеет обрабатывать след. моменты (считаем это ок, т.к. инструмент внутренний)
// 1.  --flag=value, только  --flag value
// 2. -- end-of-options
// 3. не различает причин неизвестный флаг / нет значения / повтор, на всё вернёт nullopt
// 4. вызов вида --cobertura --output будет воспринят, будто --output - значение --cobertura. --output учтён не будет
// но т.к. мы сами делаем все вызовы из coverage.cmake, и это "не для пользователя", то ок
std::optional<Options> parse_options(int argc, char* argv[]) {
    Options opt;
    bool seen_cobertura = false;
    bool seen_source_root = false;
    bool seen_expected = false;
    bool seen_output = false;
    bool seen_native = false;
    bool seen_modules = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        auto take = [&](std::string& dst, bool& seen) {
            if (seen || i + 1 >= argc)
                return false;
            const std::string_view value = argv[i + 1];
            if (value.empty())
                return false;
            seen = true;
            dst = std::string(value);
            ++i;
            return true;
        };

        if (arg == "--cobertura") {
            if (!take(opt.cobertura, seen_cobertura))
                return std::nullopt;
        } else if (arg == "--source-root") {
            if (!take(opt.source_root, seen_source_root))
                return std::nullopt;
        } else if (arg == "--expected") {
            if (!take(opt.expected, seen_expected))
                return std::nullopt;
        } else if (arg == "--output") {
            if (!take(opt.output, seen_output))
                return std::nullopt;
        } else if (arg == "--native") {
            if (!take(opt.native, seen_native))
                return std::nullopt;
        } else if (arg == "--modules") {
            if (!take(opt.modules, seen_modules))
                return std::nullopt;
        } else {
            return std::nullopt;
        }
    }

    if (!seen_cobertura)
        return std::nullopt;
    // --modules имеет смысл только вместе с --native
    // --native без --modules разрешён: тогда block-таблица печатается без фильтра и без проверки полноты
    if (seen_modules && !seen_native)
        return std::nullopt;
    return opt;
}

} // namespace

int main(int argc, char* argv[]) {
    // вывод справки, если main вызвали с -h/--help
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::println("{}", kUsage);
            return exit_code::success;
        }
    }

    // если файлы для парса пустые, то выходим с ex_code=64
    const auto options = parse_options(argc, argv);
    if (!options) {
        std::println(stderr, "{}", kUsage);
        return exit_code::usage;
    }
    // чтение cobertura.xml
    const auto xml = read_file(options->cobertura);
    if (!xml)
        return exit_code::io_fail;
    // парс cobertura.xml, с передачей kFoldCase для приведения пути к файлу к виду, соотв. ОС
    const auto parsed = cov::parse_cobertura(*xml, kFoldCase);
    if (!parsed) {
        const cov::ParseError& err = parsed.error();
        std::println(stderr, "parse error at byte {}: {}", err.offset, err.what);
        return err.reason == cov::ParseReason::malformed ? exit_code::data : exit_code::no_in;
    }

    // парс native xml
    std::optional<cov::NativeCoverage> native;
    if (!options->native.empty()) {
        const auto native_xml = read_file(options->native);
        if (!native_xml)
            return exit_code::io_fail;

        auto parsed_native = cov::parse_native_xml(*native_xml, kFoldCase);
        if (!parsed_native) {
            const cov::ParseError& err = parsed_native.error();
            std::println(stderr, "parse error at byte {}: {}", err.offset, err.what);
            return err.reason == cov::ParseReason::malformed ? exit_code::data : exit_code::no_in;
        }
        native = *parsed_native;
    }

    // ===== проверка наличия expected / missing, относящихся к cobertura.xml =====
    std::vector<std::string> expected_sources;
    std::vector<std::string> missing_sources;

    // какие файлы должны быть инструментированы, согласно манифесту из --expected
    // если такого флага не передали - ок, не ошибка, просто ожиданий нет
    if (!options->expected.empty()) {
        const auto manifest = read_file(options->expected);
        if (!manifest)
            return exit_code::io_fail;

        expected_sources = split_lines(*manifest);
        // если в файле по --expected после разбиения нет ни одного пути, то выходим с bad-data.
        // т.к. либо без --expected вообще, либо с валидным манифестом ожиданий
        if (expected_sources.empty()) {
            std::println(stderr, "expected sources manifest '{}' is empty", options->expected);
            return exit_code::data;
        }
        // учитываем, какие файлы из expected не были инструментированы
        // missing_expected также считает отсутствующим файл, у которого 0 инструментированных строк
        missing_sources = cov::missing_expected(*parsed, expected_sources, kFoldCase);
    }

    // ===== проверка наличия expected / missing, относящихся к native xml =====
    std::vector<std::string> expected_modules;
    std::vector<std::string> missing_mods;

    if (native && !options->modules.empty()) {
        const auto manifest = read_file(options->modules);
        if (!manifest)
            return exit_code::io_fail;

        expected_modules = split_lines(*manifest);
        if (expected_modules.empty()) {
            std::println(stderr, "expected modules manifest '{}' is empty", options->modules);
            return exit_code::data;
        }
        missing_mods = cov::missing_modules(*native, expected_modules, kFoldCase);
    }

    // вывод сводки по проекту
    std::string report = cov::format_summary(cov::summarize(*parsed, options->source_root));
    // если есть отчёт по native xml
    if (native) {
        cov::NativeCoverage block = *native;
        if (!expected_modules.empty())
            block = cov::select_modules(*native, expected_modules, kFoldCase);
        report += cov::format_block_summary(block);
    }

    // если есть неинструментированное по cobertura, прибавляем это к отчёту
    report += cov::format_missing_sources(missing_sources);
    if (!options->expected.empty())
        report += std::format("files: {}/{} expected\n", expected_sources.size() - missing_sources.size(),
                              expected_sources.size());

    // если есть неинструментированное по native xml, также прибавляем к отчёту
    report += cov::format_missing_modules(missing_mods);
    if (!options->modules.empty())
        report += std::format("modules: {}/{} expected\n", expected_modules.size() - missing_mods.size(),
                              expected_modules.size());

    std::print("{}", report);

    // если задан --output, но почему-то туда не смогли записать отчёт
    if (!options->output.empty() && !write_file(options->output, report))
        return exit_code::io_fail;

    // если есть то, что должно было быть инструментировано, но оно инструментировано не было,
    // выводим и падаем с ошибкой
    if (!missing_sources.empty() || !missing_mods.empty())
        return exit_code::data;

    return exit_code::success;
}
