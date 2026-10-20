#pragma once

#include <cstdio> // std::FILE
#include <memory>
#include <string>
#include <system_error> // std::error_code
#include <vector>

namespace e2e {

struct file_closer {
    void operator()(std::FILE* f) const noexcept {
        if (f)
            std::fclose(f);
    }
};

using file_ptr = std::unique_ptr<std::FILE, file_closer>;

// nullptr + errno при ошибке; нужен для подачи каталога в stdin
file_ptr open_input(const std::string& path);

// данные, которые нужно снять при запуске ребёнка
struct run_result {
    std::error_code ec; // ошибка запуска/ожидания - статус от boostproc
    int code = -1;      // child::exit_code() - обобщённый код типа 0 - успех, 1 - ошибка, и тд
    int native = -1;    // child::native_exit_code() - код падения на уровне ОС, сырой, даёт причину падения
    std::string out;    // нужно понять, является ли exit_code сырым native_exit_code или нормализуется
    std::string err;
};

// нормалидованный текст - без кареток и без последнего \n
std::string normalize(std::string s);

// обычный файл: вход открываем сами, только на чтение
run_result run(const std::string& exe1, const std::vector<std::string>& args1, const std::string& in_path,
               const std::string& out_path, const std::string& err_path);

// чтобы подать на вход каталог
// stdin передаётся уже открытым FILE* — boost не открывает путь сам
// уже открытый stdin (каталог)
run_result run_with_stdin(std::FILE* in, const std::string& exe1, const std::vector<std::string>& args1,
                          const std::string& out_path, const std::string& err_path);
} // namespace e2e
