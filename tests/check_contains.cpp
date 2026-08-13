#include <expected>
#include <fstream>
#include <iterator>
#include <print>
#include <string>

std::expected<std::string, int> file_to_bytes_string(const char* path) {
    // поток (бинарный и дл€ чтени€), источник байтов - path
    std::fstream stream(path, std::ios::binary | std::ios::in);
    // проверка что файл открылс€ удачно
    if (!stream.is_open()) {
        return std::unexpected(1);
    }
    // первый итератор опред. дл€ stream - побайтово передаЄт в s.
    // его значение каждую итерацию сравниваетс€ со вторым итератором (итер.-сентинел, опред. как null, end-of-stream
    // iterator) первый итератор при достижении EOF занул€етс€, и становитс€ равным второму(по значению). тогда чтение
    // из потока и запись в строку прекращаетс€
    std::istreambuf_iterator<char> start{stream}, end;
    std::string s{start, end};
    return s;
}

int main(int argc, char* argv[]) {
    // проверка что аргумента ровно 2
    if (argc != 3) {
        std::print(stderr, "ќжидалось 2 аргумента (<file_result> <file_expected>). ѕолучено аргументов: {}", argc - 1);
        return 1;
    }

    // чтение байтов из файлов (пути до файлов: argc[1] - result, argc[2] - expected)
    // и возвр. байтов в виде строки
    auto r1 = file_to_bytes_string(argv[1]);
    if (!r1) {
        std::print(stderr, "Ќе удалось открыть файл {}", argv[1]);
        return r1.error();
    }
    std::string result = *r1;

    auto r2 = file_to_bytes_string(argv[2]);
    if (!r2) {
        std::print(stderr, "Ќе удалось открыть файл {}", argv[2]);
        return r2.error();
    }
    std::string expected = *r2;

    // случай пустого expected (выноситс€, т.к. без него любой result + empty expected --> return 0, везде есть пуста€
    // подстрока)
    if (expected.empty()) {
        // если и результат также пуст - ќ 
        if (result.empty()) {
            return 0;
        }
        // иначе - файлы не сход€тс€
        else {
            std::print(stderr, "ќжидалс€ пустой выходной файл");
            return 1;
        }
    } else {
        // нахождение подстроки expected в result
        std::string::size_type n = result.find(expected);
        if (n == std::string::npos) {
            std::print(stderr, "¬ыходной файл содержит некорректные данные");
            return 1;
        }
    }

    return 0;
}