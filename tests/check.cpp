#include <algorithm>
#include <cstddef>
#include <expected>
#include <fstream>
#include <iterator>
#include <print>
#include <string>
#include <string_view>
#include <utility>

// локальное пространсто имён для ф-ций данного хелпера
namespace {
std::expected<std::string, int> file_to_bytes_string(const char* path) {
    // поток (бинарный и для чтения), источник байтов - path
    std::ifstream stream(path, std::ios::binary | std::ios::in);
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
    // все вхождения \r удаляются
    std::erase(s, '\r');
    // конечные \n срезаются
    while (!s.empty() && s.back() == '\n') {
        s.pop_back();
    }
}

// для отображения байтов [from; limit+from) строки s как символов ASCII
// управляющие байты экранируются, байты непечатаемые (< 0x20) и не-ASCII (>= 0x7F) печатаются как числа, остальные -
// как есть. Если окно не покрывает всю строку, с соответствующей стороны добавляется "..."
std::string escape_window(std::string_view s, std::size_t from, std::size_t limit) {
    // проверка, что from не вылез за строку. он должен быть внутри неё
    if (from > s.size())
        from = s.size();
    // размер выводимого окна - минимум из эталонной ширины и реальной ширины (от from До конца)
    const std::size_t count = std::min(limit, s.size() - from);

    std::string out;
    // если from - не у начала, то к началу прибавить ... - что что-то ещё есть, но оно не отражается, тк не влезло
    if (from > 0)
        out += "...";
    // к выходной строке out
    for (std::size_t i = from; i < from + count; ++i) {
        // каст к unsigned чтобы байты 0x80-0xFF воспринимались как положит
        // (иначе если signed, то у 0x80-0xFF 1 будет стоять на 8 позиции,
        // что для signed это означает "число отрицательное"
        const unsigned char ch = static_cast<unsigned char>(s[i]);
        // экранирование для специсимволов - так они будут отражаться, как обычные символы
        switch (ch) {
        case '\\':
            out += "\\\\";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\t':
            out += "\\t";
            break;
        case '\v':
            out += "\\v";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\0':
            out += "\\0";
            break;
        case '"':
            out += "\\\"";
            break;
        // ветка для не-служебных символов
        default:
            // непечатаемые (< 0x20) и не-ASCII (>= 0x7F). отражаются как hex байты
            if (ch < 0x20 || ch >= 0x7F) {
                // набор чисел 16сс и префикс в начале
                constexpr char hex[] = "0123456789ABCDEF";
                out += "\\x";
                // к out прибавляется сначала старший байт, потом младший
                // (ch >> 4) - сдвиг 4 старших байтов на место младших и AND с маской 1111
                out += hex[(ch >> 4) & 0xF];
                // уже без сдвига - младший байт AND маска 1111
                out += hex[ch & 0xF];
                // в ином случае это символы ASCII с ними и так не будет проблем
            } else {
                out += static_cast<char>(ch);
            }
            break;
        }
    }
    // аналогично - как было у начала, так и у конца ... если что-то не влезло и конец окна != конец строки
    if (from + count < s.size())
        out += "...";
    return out;
}

// общая шапка ошибки для режимов contains и equal.
// показывает режим, что сравнивается, что была нормализация, где расхождение
void print_header(std::string_view mode, std::string_view result_path, std::string_view expected_path,
                  std::size_t result_size, std::size_t expected_size) {
    std::println(stderr, "check failed: {}", mode);
    std::println(stderr, "  result   : {} ({} bytes, normalized)", result_path, result_size);
    std::println(stderr, "  expected : {} ({} bytes, normalized)", expected_path, expected_size);
}

// константы для вывода того, что не сошлось
// сколько всего байт выводим (2-3 строки)
constexpr std::size_t kWindowBytes = 96;
// сколько байт до первой разницы выведем
constexpr std::size_t kContextBefore = 16;

} // namespace

int main(int argc, char* argv[]) {
    // проверка что аргумента ровно 3
    if (argc != 4) {
        std::println(stderr, "3 arguments expected (--flag <file_result> <file_expected>). Received {} arguments",
                     argc - 1);
        return 1;
    }
    // проверка флага - если это ни тот и ни другой флаг, то это ошибка
    std::string_view flag{argv[1]};
    if (flag != "--contains" && flag != "--equal") {
        std::println(stderr, "Unknown flag {}. Expected --contains or --equal", argv[1]);
        return 1;
    }

    // чтение байтов из файлов (пути до файлов: argc[2] - result, argc[3] - expected)
    // и возвр. байтов в виде строки
    auto r1 = file_to_bytes_string(argv[2]);
    if (!r1) {
        std::println(stderr, "Can not open {}", argv[2]);
        return r1.error();
    }
    // r1 (как и r2 далее) не будет больше использоваться. потому её ссылка опустошается и отдаётся result
    std::string result = std::move(*r1);

    auto r2 = file_to_bytes_string(argv[3]);
    if (!r2) {
        std::println(stderr, "Can not open {}", argv[3]);
        return r2.error();
    }
    std::string expected = std::move(*r2);

    // нормализация по CRLF в любом случае. оба режима лояльны к /n и /r
    rm_crlf_symbols(expected);
    rm_crlf_symbols(result);

    // след. действие определяется флагом из cmd
    if (flag == "--contains") {
        // если подстрока не была найдена - вывод шапки и тела ошибки для check
        if (result.find(expected) == std::string::npos) {
            print_header("--contains", argv[2], argv[3], result.size(), expected.size());
            std::println(stderr, "  needle not found");
            std::println(stderr, "    needle (first {} B): \"{}\"", kWindowBytes,
                         escape_window(expected, 0, kWindowBytes));
            return 1;
        }

    } else {
        // проход по файлам итераторами, сравнение по элементам. возвращает для каждого первое место расхождения
        auto [rit, eit] = std::mismatch(result.begin(), result.end(), expected.begin(), expected.end());
        // если хотя бы для одного не дошли до конца - значит есть расхождене. вычисл. размеры и
        if (rit != result.end() || eit != expected.end()) {
            // начало расхождения результата и ожидания. беззнак(size_t)offset = начало_result - позиция расхождения -
            const std::size_t k = static_cast<std::size_t>(rit - result.begin());
            // до расхождения хотм показать kContextBefore-байт или (если k < 16) с самого начала
            const std::size_t from = k > kContextBefore ? k - kContextBefore : 0;
            print_header("--equal", argv[2], argv[3], result.size(), expected.size());
            std::println(stderr, "  first difference at byte {} (normalization: CR removed, trailing LFs stripped)", k);
            std::println(stderr, "    expected: \"{}\"", escape_window(expected, from, kWindowBytes));
            std::println(stderr, "    result  : \"{}\"", escape_window(result, from, kWindowBytes));
            return 1;
        }
    }

    return 0;
}