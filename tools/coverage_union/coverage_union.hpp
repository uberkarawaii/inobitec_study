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

// парсер узкий, и не проверяет "валидность xml вообще", только валидность конкретных моментов
// (теги/атрибуты - внутри class должен быть filename; должен/должны быть class, внутри - line)

// причина ошибки:
// malformed - не получилось разобрать тег или атрибут
// no_classes - не найдено ни одного <class>
// no_lines - не найдено ни одной <line>
// no_modules - не найдено ни одного <module ...> в native xml
enum class ParseReason { malformed, no_classes, no_lines, no_modules };
// содержание ошибки для вывода (причина, смещение, текст ошибки)
struct ParseError {
    ParseReason reason;
    std::size_t offset;
    std::string what;
};

// агрегация по всем копиям файла
// FileCoverage::hits - line -> max(hits)
struct FileCoverage {
    std::string display_path;
    std::map<int, std::uint64_t> hits;
};

// тип объекта - результата метода parse_cobertura
// все покрытые файлы в отчёте в структуре ключ-значение
// чтобы получить покрытие для опред.файла, на вход надо подать его нормализованный путь (std::string path)
struct Coverage {
    std::map<std::string, FileCoverage> files;
};

// разбор отчёта покрытия. на вход xml - .cobertura.xml;
// fold_case: true - всё в lower-case, регистр не значим (винда) / false - регистр значим (линукс)
// возвращает мапу Coverage, где одноимённые файлы объединены, а строки определяются max(hits)
// или ParseError (детали ошибки)
std::expected<Coverage, ParseError> parse_cobertura(std::string_view xml, bool fold_case);

// статистика для одного файла. valid - сколько всего строк
struct FileStat {
    std::string path;
    int covered;
    int valid;
};
// статистика непокрытых для одного файла; lines - номера строк, в которых max(hits) == 0
struct Uncovered {
    std::string path;
    std::vector<int> lines;
};

// Summary::files - все файлы, имеющиеся в Coverage. на каждый файл - FileStat
// Summary::uncovered - файлы, где есть непокрытое
// сред. покрытие по проекту = covered/valid, где covered=summ(FileStat::covered), valid=summ(FileStat::valid)
// valid - всё, что хоть раз было инструментировано. если ни разу не было в отчёте - не попадёт в valid
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
// если для файла valid==0, то он не был инструментирован
std::vector<std::string> missing_expected(const Coverage&, std::span<const std::string> expected, bool fold_case);

// секция отчёта со списком ожидаемых источников, которых нет в отчёте; пустая строка, если missing пуст
std::string format_missing_sources(std::span<const std::string> missing);

// ===== Block-метрика =====
// имя модуля и две метрики по нему из xml
struct ModuleStat {
    std::string name;
    std::uint64_t blocks_covered;
    std::uint64_t blocks_not_covered;
};

// все модули в алфавитном порядке из .xml
// для форматированного вывода и проверки полноты
struct NativeCoverage {
    std::vector<ModuleStat> modules;
};

// из native xml читает все строки вида  <module id / name / path / block_coverage ...>, отдаёт вектор с метриками
std::expected<NativeCoverage, ParseError> parse_native_xml(std::string_view xml, bool fold_case);

// проверяет по манифесту - есть ли модули без покрытия, которые указаны в манифесте
std::vector<std::string> missing_modules(const NativeCoverage&, std::span<const std::string> expected, bool fold_case);

// вернуть только модули из expected, в порядке expected
NativeCoverage select_modules(const NativeCoverage&, std::span<const std::string> expected, bool fold_case);

// печать таблицы с block-метрикой по всем модулям
std::string format_block_summary(const NativeCoverage&);
// если есть непокрытые продкутовые модули, печатает их
std::string format_missing_modules(std::span<const std::string> missing);

} // namespace cov
