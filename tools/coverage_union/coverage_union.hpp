#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cov {

// парсер узкий, и не провер€ет "валидность xml вообще", только валидность конкретных моментов
// (теги/атрибуты - внутри class должен быть filename; должен/должны быть class, внутри - line)

// причина ошибки:
// malformed - не получилось разобрать тег или атрибут
// no_classes - не найдено ни одного <class>
// no_lines - не найдено ни одной <line>
enum class ParseReason { malformed, no_classes, no_lines };
// содержание ошибки дл€ вывода (причина, смещение, текст ошибки)
struct ParseError {
    ParseReason reason;
    std::size_t offset;
    std::string what;
};

// агрегаци€ по всем копи€м файла
// FileCoverage::hits - line -> max(hits)
struct FileCoverage {
    std::string display_path;
    std::map<int, std::uint64_t> hits;
};

// тип объекта - результата метода parse_cobertura
// все покрытые файлы в отчЄте в структуре ключ-значение
// чтобы получить покрытие дл€ опред.файла, на вход надо подать его нормализованный путь (std::string path)
struct Coverage {
    std::map<std::string, FileCoverage> files;
};

// разбор отчЄта покрыти€. на вход xml - .cobertura.xml;
// fold_case: true - всЄ в lower-case, регистр не значим (винда) / false - регистр значим (линукс)
// возвращает мапу Coverage, где одноимЄнные файлы объединены, а строки определ€ютс€ max(hits)
// или ParseError (детали ошибки)
std::expected<Coverage, ParseError> parse_cobertura(std::string_view xml, bool fold_case);

// статистика дл€ одного файла. valid - сколько всего строк
struct FileStat {
    std::string path;
    int covered;
    int valid;
};
// статистика непокрытых дл€ одного файла; lines - номера строк, в которых max(hits) == 0
struct Uncovered {
    std::string path;
    std::vector<int> lines;
};

// Summary::files - все файлы, имеющиес€ в Coverage. на каждый файл - FileStat
// Summary::uncovered - файлы, где есть непокрытое
// сред. покрытие по проекту = covered/valid, где covered=summ(FileStat::covered), valid=summ(FileStat::valid)
// valid - всЄ, что хоть раз было инструментировано. если ни разу не было в отчЄте - не попадЄт в valid
struct Summary {
    std::vector<FileStat> files;
    std::vector<Uncovered> uncovered;
    int covered;
    int valid;
};

// получение структуры Summary: покрытие на каждый файл и итог по проекту
Summary summarize(const Coverage&, std::string_view source_root);
// форматирование - из Summary-объекта делает таблицу
std::string format_summary(const Summary&);
// в Summary нет кол-ва ожидаемых файлов, потому к-во ожидаемых надо печатать отдельно

// проверка того, что все ожидаемые файлы были инструментированы (ищет в Coverage c)
// если дл€ файла valid==0, то он не был инструментирован
std::vector<std::string> missing_expected(const Coverage&, std::span<const std::string> expected, bool fold_case);

} // namespace cov
