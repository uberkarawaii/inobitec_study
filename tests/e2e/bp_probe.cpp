#include <boost/process/v1.hpp>
#include <charconv>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <system_error>
#include <vector>

// сокращение namespace
namespace bp = boost::process::v1;

// все, кто после этого namespace, увидят имена внутри него. те, кто до, - не увидят (именно здесь таких нет, все после)
namespace {
// у bp_probe в target_compile_definitions E2E_ARGV_ECHO прописан как argv_echo - т.е. путь до ребёнка
const std::string exe = E2E_ARGV_ECHO;
// директория с in/err/out, создана CMake-ом
// важно чтобы существовала, т.к. err и out нужно писать в существующий каталог
const std::string dir = E2E_PROBE_DIR;
// probe.in и /probe созданы CMake-ом, .out .err будут созданы от перенаправления bp::std_out >..., bp::std_err >...
const std::string in_file = std::format("{}/probe.in", dir);
const std::string out_file = std::format("{}/probe.out", dir);
const std::string err_file = std::format("{}/probe.err", dir);

std::vector<std::string> args = {"2 3", "2\" 3", "10\\", "++10", ""};
} // namespace

// чтение файла в виде байтов
std::string read_file_binary(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
}

// нормалидованный текст - без кареток и без последнего \n
std::string normalize(std::string s) {
    std::erase(s, '\r');
    while (!s.empty() && s.back() == '\n')
        s.pop_back();
    return s;
}

// данные, которые нужно снять при запуске ребёнка
struct run_result {
    std::error_code ec; // ошибка запуска/ожидания - статус от boostproc
    int code = -1;      // child::exit_code() - обобщённый код типа 0 - успех, 1 - ошибка, и тд
    int native = -1;    // child::native_exit_code() - код падения на уровне ОС, сырой, даёт причину падения
    std::string out;    // нужно понять, является ли exit_code сырым native_exit_code или нормализуется
    std::string err;
};

// запуск ребёнка
run_result run(const std::string& exe1, const std::vector<std::string>& args1, const std::string& in_path,
               const std::string& out_path, const std::string& err_path) {
    run_result r;
    bp::child c(exe1, bp::args = args1, bp::std_in < in_path, bp::std_out > out_path, bp::std_err > err_path, r.ec);
    if (r.ec)
        return r;
    c.wait(r.ec);
    if (r.ec)
        return r;
    r.code = c.exit_code();
    r.native = c.native_exit_code();
    r.out = read_file_binary(out_path);
    r.err = read_file_binary(err_path);
    return r;
}

// проверка выходного кода - что возвращается не сырой статус
// процесс ребёнка возьмёт строку, распознает число из неё и вернёт его как код завершения
// если не сырой код, то это и будет это же число
bool check_exit_code() {
    auto r = run(E2E_EXIT_WITH, {"65"}, in_file, dir + "/probe_exit.out", dir + "/probe_exit.err");
    if (r.ec) {
        std::println("exit_code: run failed: {}", r.ec.message());
        return false;
    }
    const bool ok = (r.code == 65);
    std::println("exit_code: code={}, native={}, want 65 -> {}", r.code, r.native, ok ? "PASS" : "FAIL");
    return ok;
}

// что будет под линуксом при подаче каталога на вход. ожидается io fail, excode=74
bool check_dir_stdin() {
#ifdef _WIN32
    std::println("dir stdin: SKIP (UNIX only)");
    return true;
#else
    auto r = run(E2E_T3_C, {}, E2E_INPUT_DIR, // input — каталог
                 dir + "/probe_dir.out", dir + "/probe_dir.err");
    // если у boostproc возникла проблема с тем, чтобы открыть каталог и он не смог подать его на вход процессу
    if (r.ec) {
        std::println("dir stdin: spawn/redirect failed: {}", r.ec.message());
        return false;
    }
    const bool ok = (r.code == 74);
    std::println("dir stdin: code={}, want 74 -> {}", r.code, ok ? "PASS" : "FAIL");
    return ok;
#endif
}

// проверка полноты вывода от big_out.cpp
bool check_big_output() {
    const std::string out_path = dir + "/probe_big.out";
    auto r = run(E2E_BIG_OUT, {}, in_file, out_path, dir + "/probe_big.err");
    if (r.ec) {
        std::println("big output: run failed: {}", r.ec.message());
        return false;
    }
    std::error_code fs_ec;
    const auto size = std::filesystem::file_size(out_path, fs_ec);
    const bool ok = !fs_ec && r.code == 0 && size == 1024 * 1024;
    std::println("big output: code={}, size={} -> {}", r.code, size, ok ? "PASS" : "FAIL");
    return ok;
}

// как отобразятся литералы chcp1251 при выводе ошибки?
// t1_c на вход подастся t1_abc, где abc вместо числа, далее - сравнение текста ошибки с chcp1251 raw-строкой
bool check_cp1251_literal() {
    // R - raw строковый литерал, разделитель - "E2E". т.е. всё в скобках м-у E2E - сырой строковый литерал
    constexpr std::string_view expected_literal = R"E2E(N должно быть целым числом. Получено: abc)E2E";

    const std::string from_file = normalize(read_file_binary(std::format("{}/t1_abc", E2E_EXPECT_DIR)));
    const std::string from_literal = normalize(std::string(expected_literal));

    auto r = run(E2E_T1_C, {}, std::format("{}/t1_abc", E2E_INPUT_DIR), dir + "/probe_cyr.out", dir + "/probe_cyr.err");
    if (r.ec) {
        std::println("cp1251: run failed: {}", r.ec.message());
        return false;
    }
    const std::string from_program = normalize(r.err);

    const bool lit_ok = (from_literal == from_file);
    const bool prog_ok = (from_program == from_file);
    std::println("cp1251: literal vs expect {}, program vs expect {}", lit_ok ? "PASS" : "FAIL",
                 prog_ok ? "PASS" : "FAIL");
    return lit_ok && prog_ok;
}

// проверка того, что exe-стиль работает ожидаемо (т.е. "2\" 3" -> 2" 3) и shell-стиль отсутствует
bool check_argv() {

    // статус boostproc-операции - создание процесса, его удачное завершение, открытие файлов для него. всё успешно -> 0
    std::error_code ec;
    // запуск подпроцесса exe (пришёл как путь в define от cmake)
    bp::child c(exe, bp::args = args, bp::std_in<in_file, bp::std_out> out_file, bp::std_err > err_file, ec);
    // удачно ли создался процесс?
    if (ec) {
        std::println(stderr, "spawn failed: {}", ec.message());
        return false;
    }
    // ждём завершения ребёнка и записываем его код в ec
    c.wait(ec);
    // удачно ли завершился процесс?
    if (ec) {
        std::println(stderr, "wait failed: {}", ec.message());
        return false;
    }
    // код завершения ребёнка
    // проверить, что exit_code() будет давать на POSIX (если не чистый ex_code, то wexitstatus)
    const int code = c.exit_code();
    std::println("exit code = {}", code);

    // чтение и разбор выходного файла. если нечитаемый - выход
    std::ifstream out_stream(out_file);
    if (!out_stream) {
        std::println(stderr, "cannot open {}", out_file);
        return false;
    }
    // запись выходного файла в вектор
    std::vector<std::string> lines;
    for (std::string line; std::getline(out_stream, line);)
        lines.push_back(line);

    bool ok = (code == 0) ? true : false;
    // argc: первая строка "argc = N" - проверка что у строки ожидаемый префикс и N - по кол-ву переданных арг.+1
    // куда уйдёт распознанное значение
    int got_argc = -1;
    if (lines.empty() || !lines[0].starts_with("argc = ")) {
        ok = false;
    } else {
        const char* first = lines[0].data() + 7;
        const char* last = lines[0].data() + lines[0].size();
        // если распознанное значение не ушло в got_argc - ошибка не дефолтная/пустая, то есть несовпадение
        if (std::from_chars(first, last, got_argc).ec != std::errc{})
            ok = false;
    }
    // совпадает ли кол-во аргументов ребёнка с тем, что должно быть на самом деле
    const int want_argc = static_cast<int>(args.size()) + 1; // + имя программы
    if (got_argc != want_argc) {
        ok = false;
        std::println(stderr, "argc: got {}, want {}", got_argc, want_argc);
    }

    // argv[1..N]: строки 2..N+1 (0 — argc, 1 — argv[0])
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::size_t idx = i + 2;
        // если idx больше реального вывода, то реальный вывод неполный. пометка в ok и выход, т.к. нечего проверять
        if (idx >= lines.size()) {
            ok = false;
            break;
        }
        // проверка на формат вывода argv[N] = X, при нарушении формата - пометка в ok
        const std::string& l = lines[idx];
        const auto pos = l.find("] = ");
        if (pos == std::string::npos) {
            ok = false;
            std::println(stderr, "bad line: {}", l);
            continue;
        }
        // проверка - что ребёнок напечатал после "] = " и совпадает ли это с соотв. арг из args = {"2 3",...}
        const std::string got = l.substr(pos + 4);
        if (got != args[i]) {
            ok = false;
            std::println(stderr, "argv[{}]: got \"{}\", want \"{}\"", i + 1, got, args[i]);
        }
    }
    // если ребёнок вывел лишнее
    if (lines.size() > args.size() + 2)
        ok = false;

    // stderr должен быть пуст
    std::ifstream err_stream(err_file);
    if (!err_stream) {
        std::println(stderr, "cannot open {}", err_file);
        return false;
    }
    if (err_stream.peek() != std::ifstream::traits_type::eof()) {
        ok = false;
        std::println(stderr, "stderr is not empty");
    }
    // было ли хоть одно несовпадение с эталоном?
    std::println("{}", ok ? "PASS" : "FAIL");
    return ok;
}

// аггрегация всех проверок
int main() {
    const bool ok_argv = check_argv();
    const bool ok_exit = check_exit_code();
    const bool ok_dir = check_dir_stdin();
    const bool ok_big = check_big_output();
    const bool ok_cyr = check_cp1251_literal();
    const bool all = ok_argv && ok_exit && ok_dir && ok_big && ok_cyr;
    std::println("{}", all ? "ALL PASS" : "SOME FAILED");
    return all ? 0 : 1;
}
