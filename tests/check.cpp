#include <expected>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <string_view>

std::expected<std::string, int> file_to_bytes_string(const char* path) {
    // поток (бинарный и для чтения), источник байтов - path
    std::fstream stream(path, std::ios::binary | std::ios::in);
    // проверка что файл открылся удачно
    if (!stream.is_open()) {
        return std::unexpected(1);
    }
    // первый итератор опред. для stream - побайтово передаёт в s.
    // его значение каждую итерацию сравнивается со вторым итератором (итер.-сентинел, опред. как null, end-of-stream
    // iterator) первый итератор при достижении EOF зануляется, и становится равным второму(по значению). тогда чтение
    // из потока и запись в строку прекращается
    std::istreambuf_iterator<char> start{stream}, end;
    std::string s{start, end};
    return s;
}

// убирает каретки и заключительные переносы строк
void rm_crlf_symbols(std::string& s) {
    // все вхождения /r удаляются
    std::erase(s, '\r');
    // конечные \n срезаются
    while (!s.empty() && s.back() == '\n') {
        s.pop_back();
    }
}

int main(int argc, char* argv[]) {
    // проверка что аргумента ровно 3
    if (argc != 4) {
        std::print(stderr, "Ожидалось 3 аргумента (--flag <file_result> <file_expected>). Получено аргументов: {}",
                   argc - 1);
        return 1;
    }
    // проверка флага - если это ни тот и ни другой флаг, то это ошибка
    std::string_view flag{argv[1]};
    if (flag != "--contains" && flag != "--equal") {
        std::print(stderr, "Недоступный флаг {}. Ожидается --contains или --equal", argv[1]);
        return 1;
    }

    // чтение байтов из файлов (пути до файлов: argc[2] - result, argc[3] - expected)
    // и возвр. байтов в виде строки
    auto r1 = file_to_bytes_string(argv[2]);
    if (!r1) {
        std::print(stderr, "Не удалось открыть файл {}", argv[2]);
        return r1.error();
    }
    std::string result = *r1;

    auto r2 = file_to_bytes_string(argv[3]);
    if (!r2) {
        std::print(stderr, "Не удалось открыть файл {}", argv[3]);
        return r2.error();
    }
    std::string expected = *r2;

    // след. действие определяется флагом из cmd
    if (flag == "--contains") {
        // нахождение подстроки expected в result
        std::string::size_type n = result.find(expected);
        if (n == std::string::npos) {
            std::print(stderr, "Выходной файл содержит некорректные данные");
            return 1;
        }
    } else {
        // нормализация по CRLF
        rm_crlf_symbols(expected);
        rm_crlf_symbols(result);
        // проверка на равенство файлов
        if (result != expected) {
            std::print(stderr, "Файлы {} {} не равны после нормализации по CR и заключительным LF", argv[2], argv[3]);
            return 1;
        }
    }

    return 0;
}