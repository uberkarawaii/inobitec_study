#include "coverage_union.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace cov {
namespace {

bool is_xml_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

bool is_name_char(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' ||
           c == '.' || c == ':';
}

ParseError make_error(ParseReason reason, std::size_t offset, std::string what) {
    return ParseError{reason, offset, std::move(what)};
}

// '>' внутри кавычек не считается концом тега
std::size_t find_tag_end(std::string_view s, std::size_t start) {
    char quote = '\0';
    for (std::size_t i = start; i < s.size(); ++i) {
        const char c = s[i];
        // quote == '\0' -> мы вне кавычек
        // quote != '\0' -> мы в кавычках
        if (quote != '\0') {
            // если при этом текущий символ - также кавычка, то она закрывающая. выходим из кавычек
            if (c == quote)
                quote = '\0';
        }
        // quote == '\0'. если сейчас кавычка, мы в неё зашли. запоминаем это в quote
        else if (c == '"' || c == '\'') {
            quote = c;
        }
        // quote == '\0', > реально закрывает тэг
        else if (c == '>') {
            return i;
        }
    }
    return std::string_view::npos;
}

struct Tag {
    std::string_view name{}; // "class", "line", "package", ...
    // контракт - Tag::text это весь тег, включая <...>
    std::string_view text{};   // "<class ...>", "</class>", "<line ... />"
    std::size_t begin = 0;     // индекс '<'
    bool closing = false;      // </...>
    bool self_closing = false; // .../>

    // только часть после имени тега: " filename=..." / ">" / "/>"
    std::string_view attributes() const {
        const std::size_t attributes_begin = (closing ? 2 : 1) + name.size();
        return text.substr(attributes_begin);
    }
};

// обособленный поиск тега - не считаем комментарии и строки, которые xml-парсер игнорирует
class TagReader {
public:
    explicit TagReader(std::string_view xml) : xml_(xml) {}

    // true  - тег прочитан
    // false - конец документа
    // unexpected - malformed
    std::expected<bool, ParseError> next(Tag& out);

private:
    std::string_view xml_;
    std::size_t pos_ = 0;
};

// поиск следующего тега, пропуск комментариев и деклараций. падение на CDATA, DOCTYPE -
// не поддерживаем этот синтаксис. 1) там внутри мб < или > 2) .cobertura.xml не имеет такого синтаксиса
// next() не проверяет, есть ли у того же <class> закрывающий </class>. парный закрывающий ищет автомат парсера
std::expected<bool, ParseError> TagReader::next(Tag& out) {
    while (pos_ < xml_.size()) {
        std::size_t tag_open_angle = xml_.find("<", pos_);
        // если начала открывающего тега больше нет, теги закончились - это конец документа
        if (tag_open_angle == std::string_view::npos)
            return false;
        // сдвигаемся на <
        pos_ = tag_open_angle;

        // что именно находится за < ? comment/declaration -> пропускаем
        if (xml_.substr(pos_, 4) == "<!--") {
            // ищем закрытие комментария. если его нет - "плохой формат"
            const auto comment_end = xml_.find("-->", pos_ + std::string_view("<!--").size());
            if (comment_end == std::string_view::npos)
                return std::unexpected(make_error(ParseReason::malformed, pos_, "unterminated comment"));
            // если закрывающая часть есть, передвигаемся за неё
            pos_ = comment_end + std::string_view("-->").size();
            continue;
        }

        if (xml_.substr(pos_, 2) == "<?") {
            // ищем закрытие декларации. если его нет - "плохой формат"
            const auto declaration_end = xml_.find("?>", pos_ + std::string_view("<?").size());
            if (declaration_end == std::string_view::npos)
                return std::unexpected(make_error(ParseReason::malformed, pos_, "unterminated declaration"));
            // если закрывающая часть есть, передвигаемся за неё
            pos_ = declaration_end + std::string_view("?>").size();
            continue;
        }
        // если <!DOCTYPE.../<![CDATA... - падаем. т.к. не поддерживаем
        if (xml_.substr(pos_, 2) == "<!")
            return std::unexpected(make_error(ParseReason::malformed, pos_, "unsupported markup"));

        // найти конец тега
        const auto tag_end = find_tag_end(xml_, pos_);
        if (tag_end == std::string_view::npos)
            return std::unexpected(make_error(ParseReason::malformed, pos_, "unterminated tag"));

        // если дошли до сюда, то тег "нормальный", надо найти его имя и запомнить в out
        out.begin = pos_;
        out.text = xml_.substr(pos_, tag_end - pos_ + 1);
        out.self_closing = xml_[tag_end - 1] == '/';

        // нахождение имени тега
        std::size_t name_start = pos_ + 1;
        // это закрывающий тег?
        bool closing = false;
        if (name_start < tag_end && xml_[name_start] == '/') {
            // да, закрывающий. и для нахождения имени сдвигаемся ещё на один
            closing = true;
            ++name_start;
        }
        out.closing = closing;

        // идём от начала имени по одному символу, пока это не терминатор (space-symbol, >, etc.)
        std::size_t name_end = name_start;
        while (name_end < tag_end && is_name_char(xml_[name_end]))
            ++name_end;

        // <> или < class>
        if (name_end == name_start)
            return std::unexpected(make_error(ParseReason::malformed, pos_, "missing tag name or wrong name format"));

        // проверка что не </tag/> и не </tag attr="x">
        if (closing) {
            for (std::size_t i = name_end; i < tag_end; ++i) {
                if (!is_xml_space(xml_[i]))
                    return std::unexpected(
                        make_error(ParseReason::malformed, pos_, "unexpected characters in closing tag"));
            }
        }

        // на что заканчивается имя тега
        const char delim = (name_end < tag_end) ? xml_[name_end] : '>';
        // если имя заканчивается не на пробельный символ, не на / и не на > то это "плохой формат"
        if (!(is_xml_space(delim) || delim == '/' || delim == '>'))
            return std::unexpected(make_error(ParseReason::malformed, pos_, "improper end of tag name"));

        out.name = xml_.substr(name_start, name_end - name_start);

        // тег валидный, прыгаем за него и выходим из цикла - т.к. next tag найден, его out заполнен
        pos_ = tag_end + 1;
        return true;
    }
    return false;
}

// порядок атрибутов не важен; кавычки одинарные/двойные
// optional - value that may or may not be present
// find_attr смотрит на атрибуты, не учитывая название тега. т.к. оно и не является атрибутом
std::optional<std::string_view> find_attr(std::string_view attributes, std::string_view name) {
    std::size_t i = 0;
    while (i < attributes.size()) {
        while (i < attributes.size() && is_xml_space(attributes[i]))
            ++i;
        if (i >= attributes.size() || attributes[i] == '>' || attributes[i] == '/')
            break;

        const std::size_t ns = i;
        while (i < attributes.size() && !is_xml_space(attributes[i]) && attributes[i] != '=' && attributes[i] != '>' &&
               attributes[i] != '/')
            ++i;
        const std::string_view aname = attributes.substr(ns, i - ns);

        while (i < attributes.size() && is_xml_space(attributes[i]))
            ++i;
        if (i >= attributes.size() || attributes[i] != '=')
            continue;
        ++i;
        while (i < attributes.size() && is_xml_space(attributes[i]))
            ++i;
        if (i >= attributes.size() || (attributes[i] != '"' && attributes[i] != '\''))
            return std::nullopt;

        const char quote = attributes[i++];
        const std::size_t vs = i;
        while (i < attributes.size() && attributes[i] != quote)
            ++i;
        if (i >= attributes.size())
            return std::nullopt;

        const std::string_view value = attributes.substr(vs, i - vs);
        ++i;
        if (aname == name)
            return value;
    }
    return std::nullopt;
}

std::string decode_entities(std::string_view raw) {
    std::string out;
    out.reserve(raw.size());
    // в xml спецсимволы лежат как &имя; (& будет &amp; < &lt; и т.д.)
    // чтобы это не испортило пути, спецсимв. -> обычные символы
    for (std::size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] != '&') {
            out += raw[i];
        } else if (raw.substr(i, 5) == "&amp;") {
            out += '&';
            i += 4;
        } else if (raw.substr(i, 4) == "&lt;") {
            out += '<';
            i += 3;
        } else if (raw.substr(i, 4) == "&gt;") {
            out += '>';
            i += 3;
        } else if (raw.substr(i, 6) == "&quot;") {
            out += '"';
            i += 5;
        } else if (raw.substr(i, 6) == "&apos;") {
            out += '\'';
            i += 5;
        } else {
            out += raw[i];
        }
    }
    return out;
}

bool parse_u64(std::string_view s, std::uint64_t& out) {
    if (s.empty())
        return false;
    const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out);
    return ec == std::errc{} && ptr == s.data() + s.size();
}

// \ -> /, схлопнуть //, на Windows lower-case
std::string normalize_key(std::string_view path, bool fold_case) {
    std::string out;
    out.reserve(path.size());
    bool prev_slash = false;
    for (char c : path) {
        if (c == '\\')
            c = '/';
        if (c == '/' && prev_slash)
            continue;
        prev_slash = (c == '/');
        if (fold_case && c >= 'A' && c <= 'Z')
            c = static_cast<char>(c - 'A' + 'a');
        out += c;
    }
    return out;
}

// приводит путь к каноническому виду для сравнения:
// - относительный путь склеивается с source_root;
// - lexically_normal убирает "./" и "..";
// - normalize_key приводит слеши и регистр.
std::string canonical_key(std::string_view path, std::string_view source_root, bool fold_case) {
    if (source_root.empty())
        return normalize_key(path, fold_case);

    std::filesystem::path p{path};
    if (p.is_relative())
        p = std::filesystem::path{source_root} / p;
    return normalize_key(p.lexically_normal().generic_string(), fold_case);
}

std::string relative_path(std::string_view path, std::string_view source_root) {
    if (source_root.empty())
        return std::string(path);
    const std::filesystem::path p{path};
    const std::filesystem::path root{source_root};
    const std::string rel = p.lexically_relative(root).generic_string();
    if (rel.empty() || rel.starts_with(".."))
        return std::string(path);
    return rel;
}

// общий хелпер для печати метрик по строкам и по блокам
std::string format_counts(std::string_view path, std::uint64_t covered, std::uint64_t valid, std::size_t width) {
    const double pct = valid == 0 ? 0.0 : 100.0 * static_cast<double>(covered) / static_cast<double>(valid);
    return std::format("{:<{}} {:>5}/{:<5} {:>7.2f}%\n", path, width, covered, valid, pct);
}

// форматный вывод статистики для отдельного файла
std::string format_line(std::string_view path, int covered, int valid, std::size_t width) {
    return format_counts(path, static_cast<std::uint64_t>(covered), static_cast<std::uint64_t>(valid), width);
}

// общий вывод списка missing для sources/modules
// вызывается из format_missing_sources и format_missing_modules
std::string format_missing_list(std::span<const std::string> missing, std::string_view title) {
    if (missing.empty())
        return {};
    std::string out = std::format("\n{}:\n", title);
    for (const auto& p : missing)
        out += std::format("  {}\n", p);
    return out;
}
} // namespace

// ================================================================================================
// parse_cobertura - проход по сырому cobertura.xml
std::expected<Coverage, ParseError> parse_cobertura(std::string_view xml, bool fold_case) {
    Coverage cov;
    // string_view одной строкой всего .cobertura.xml
    TagReader reader(xml);
    // текущий тег, текущий класс, к-во классов и строк вообще для итог.отчёта
    Tag tag{};
    // если мы не внутри класса, current==nullptr
    FileCoverage* current = nullptr;
    std::size_t classes = 0;
    std::size_t lines = 0;

    while (true) {
        // следующий тег
        auto has = reader.next(tag);
        // has -> есть значение в *has
        // !has -> есть ParseError
        if (!has)
            return std::unexpected(has.error());
        // has и *has==false -> документ закончился, парсить больше нечего, выходим
        if (!*has)
            break;

        // has и *has==true - есть прочитанный тег
        // сейчас тег class и он не-закрывающий - <class>
        if (tag.name == "class" && !tag.closing) {
            // если current не null, то мы внутри класса. и встретили открывающий <class>
            // у cobertura влож.классов быть не должно -> ошибка
            if (current != nullptr)
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "nested <class>"));

            // иначе, класс ещё не был открыт. парсим атрибуты самого <class ...>
            // ищем атрибут filename после имени тега (т.е. сам class не учитываем, т.к. его и не нужно учитывать)
            auto raw = find_attr(tag.attributes(), "filename");
            // там обязательно должен быть filename, если нет - "плохой формат"
            if (!raw)
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "no filename"));

            // переделать спец.символы в обычные и в key, проверить что path непуст, сделать регистр соотв. ОС
            const std::string path = decode_entities(*raw);
            // filename считаем пустым, только если он "". все остальные вариации - валидный filename
            // не trim, чтобы не портить реальные пробелы в путях
            if (path.empty())
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "empty filename"));
            const std::string key = normalize_key(path, fold_case);
            // добавление FileCoverage в общий Coverage. если для данного path нет записи, то будет
            // создана новая запись и добавлен путь до файла
            FileCoverage& fc = cov.files[key];
            if (fc.display_path.empty())
                fc.display_path = path;

            ++classes;
            // если не <class/>, а <class> - то мы внутри класса и надо будет проверить сущ-вование </class>
            if (!tag.self_closing)
                current = &fc;
        } else if (tag.name == "class" && tag.closing) {
            // встретили </class>. если мы не внутри <class>, то </class> ни к чему не относится
            if (current == nullptr)
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "unexpected </class>"));
            // иначе мы были внутри класса и он закрылся
            current = nullptr;
        } else if (tag.name == "line" && !tag.closing) {
            // line, не относящийся ни к какому классу -> "плохой формат"
            if (current == nullptr)
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "<line> outside <class>"));

            // теги внутри line, если нет - "плохой формат"
            const std::string_view attrs = tag.attributes();
            auto raw_n = find_attr(attrs, "number");
            auto raw_h = find_attr(attrs, "hits");
            if (!raw_n || !raw_h)
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "line without number/hits"));

            // парс number/hits, если не удалось или n не в пределах нормы - "плохой формат"
            // hits в рамках xml - беззнаковый, а значит шире. number/hits сначала парс.в такой тип
            std::uint64_t n = 0, h = 0;
            // проверка hits (что не вылез за диапазон) осуществляется тут же. а для number - ещё одна далее
            if (!parse_u64(*raw_n, n) || !parse_u64(*raw_h, h))
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "bad number/hits"));
            // считаем, что в исходнике к-во строк не превышает INT_MAX
            if (n == 0 || n > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
                return std::unexpected(make_error(ParseReason::malformed, tag.begin,
                                                  "line number is out of supported range [1; INT_MAX]"));
            // текущая строка и нахождение максимальных hits для неё
            const int line_no = static_cast<int>(n);
            // === нахождение максимума hits на строку ===
            // -> означает разыменов.указателя и взять соотв.поле (то же, что и (*current).hits[line_no])
            // current->hits[line_no] - обращение к std::map<int, uint64_t> - мапа из номер_строки и max_hits
            // если строки ещё не было, то hits[line_no] вставляет её с hits=0 и возвр. ссылку на неё
            // в итоге выбирается максимум hits - из того, что уже было, и из того, что есть сейчас
            auto& slot = current->hits[line_no];
            slot = std::max(slot, h);
            ++lines;
        }
        // package/classes/methods/method/lines/source_file/... игнорируем
    } // здесь закрывается while - если закрылся, то next вернул false, теги закончились

    // тегов больше нет, но имеем открытый <class> - не встретили </class> - незакрытый класс
    if (current != nullptr)
        return std::unexpected(make_error(ParseReason::malformed, 0, "unclosed <class>"));
    // если в отчёте не было классов / строк - не должно так быть, ошибка
    if (classes == 0)
        return std::unexpected(make_error(ParseReason::no_classes, 0, "no <class> in report"));
    if (lines == 0)
        return std::unexpected(make_error(ParseReason::no_lines, 0, "no <line> in report"));
    return cov;
}

// ================================================================================================
// итоговая статистика по проекту
Summary summarize(const Coverage& cov, std::string_view source_root) {
    Summary out{};
    for (const auto& entry : cov.files) {
        const auto& fc = entry.second;
        // относит. путь, подсчёт - покрытые и какие именно непокрытые
        const std::string display = relative_path(fc.display_path, source_root);
        int covered = 0;
        std::vector<int> missing;
        for (const auto& [line, h] : fc.hits) {
            if (h > 0)
                ++covered;
            else
                missing.push_back(line);
        }
        // сколько всего инструментированных строк по данному файлу
        const int valid = static_cast<int>(fc.hits.size());
        // добавляем в итоговый Summary статистику по текущему файлу
        out.files.push_back(FileStat{display, covered, valid});
        // есть непокрытые строки? в uncovered curr-файла перемещаем missing, тк сам missing здесь более не нужен
        if (!missing.empty())
            out.uncovered.push_back(Uncovered{display, std::move(missing)});
        // вклад данного файла (всего и покрытые) в итоговую статистику
        out.covered += covered;
        out.valid += valid;
    }
    return out;
}
// ================================================================================================
// форматированный вывод статистики для всего проекта
std::string format_summary(const Summary& s) {
    // ширина этого столбца должна быть не меньше, чем строка "TOTAL", которая там точно будет
    std::size_t width = std::string_view("TOTAL").size();
    // в итоге выбираем width равный ширине самого широкого пути до файла
    for (const auto& f : s.files)
        width = std::max(width, f.path.size());

    // шапка и все записи по файлам. и итоговая запись с "всего" и "покрыто" по всему проекту
    std::string out = "coverage union (line-based, Cobertura)\n";
    for (const auto& f : s.files)
        out += format_line(f.path, f.covered, f.valid, width);
    out += format_line("TOTAL", s.covered, s.valid, width);

    // если есть непокрытые строки, вывод их с подробностями
    if (!s.uncovered.empty()) {
        out += "\nuncovered lines:\n";
        for (const auto& u : s.uncovered) {
            out += "  ";
            out += u.path;
            out += ": ";
            for (std::size_t i = 0; i < u.lines.size(); ++i) {
                if (i != 0)
                    out += ", ";
                out += std::to_string(u.lines[i]);
            }
            out += '\n';
        }
    }
    return out;
}
// ================================================================================================
// если есть файлы, которые должны были быть инструментированы, но не были, - вернуть их список
// expected - список путей из манифеста
// fold_case должен быть тот же, что и переданный в parse_cobertura
std::vector<std::string> missing_expected(const Coverage& cov, std::span<const std::string> expected, bool fold_case,
                                          std::string_view source_root) {
    // берём канонич. путь файла и смотрим, есть ли хотя бы одна инструментированная строка хотя бы в одной копии
    std::map<std::string, bool> present;
    for (const auto& [key, fc] : cov.files) {
        const std::string canonical = canonical_key(key, source_root, fold_case);
        // OR, а не присваивание: если один и тот же файл пришёл дважды,
        // достаточно, чтобы hits были в любой копии
        present[canonical] = present[canonical] || !fc.hits.empty();
    }

    std::vector<std::string> missing;
    for (const auto& e : expected) {
        const auto it = present.find(canonical_key(e, source_root, fold_case));
        // если итератор дошёл до конца, не обнаружив файла, или у файла 0 инструментированных строк
        if (it == present.end() || !it->second)
            missing.push_back(e);
    }
    return missing;
}

// секция отчёта со списком ожидаемых источников, которых нет в отчёте; пустая строка, если missing пуст
// существующий format_missing_sources теперь использует общий хелпер
std::string format_missing_sources(std::span<const std::string> missing) {
    return format_missing_list(missing, "missing product sources");
}

// ===== BLOCK-METRIC (native xml) ======
// ================================================================================================
// разбор нативного XML Microsoft.CodeCoverage.Console
std::expected<NativeCoverage, ParseError> parse_native_xml(std::string_view xml, bool fold_case) {
    // вектор с объектами name + blocks_covered + blocks_not_covered
    NativeCoverage cov;
    // ищем каждый след. тег в xml
    TagReader reader(xml);
    Tag tag{};
    // состояние - мы внутри модуля или снаружи
    bool in_module = false;

    while (true) {
        auto has = reader.next(tag);
        // имеем либо ParseErr - тогда его возвращаем; либо !flag - тогда теги закончились
        if (!has)
            return std::unexpected(has.error());
        if (!*has)
            break;

        // <skipped_module>, <modules>, <function> и пр. игнорируются
        if (tag.name != "module")
            continue;

        // </module>
        if (tag.closing) {
            if (!in_module)
                return std::unexpected(make_error(ParseReason::malformed, tag.begin, "unexpected </module>"));
            in_module = false;
            continue;
        }

        // <module> внутри <module>
        if (in_module)
            return std::unexpected(make_error(ParseReason::malformed, tag.begin, "nested <module>"));

        // нахождение всех тегов, нужных для объекта NativeCoverage. если хотя бы одного нет - ошибка
        const std::string_view attrs = tag.attributes();
        auto raw_name = find_attr(attrs, "name");
        auto raw_covered = find_attr(attrs, "blocks_covered");
        auto raw_not_covered = find_attr(attrs, "blocks_not_covered");
        if (!raw_name || !raw_covered || !raw_not_covered)
            return std::unexpected(
                make_error(ParseReason::malformed, tag.begin, "module without name/blocks_covered/blocks_not_covered"));

        // перевести спецсимволы в обычные
        const std::string name = decode_entities(*raw_name);
        // не обезаем имя, только если .empty() - то выходим. остальное считаеся за валидное имя
        if (name.empty())
            return std::unexpected(make_error(ParseReason::malformed, tag.begin, "empty module name"));

        // парс blocks_covered и blocks_not_covered
        std::uint64_t covered = 0;
        std::uint64_t not_covered = 0;
        if (!parse_u64(*raw_covered, covered) || !parse_u64(*raw_not_covered, not_covered))
            return std::unexpected(
                make_error(ParseReason::malformed, tag.begin, "bad blocks_covered/blocks_not_covered"));

        // name - имя текущего модуля. проходим по всем распознанным до этого cov.modules,
        // у модуля m берём имя и сравниваем его с именем текущего модуля, который сейчас обрабатывается
        const std::string key = normalize_key(name, fold_case);
        const auto duplicate = std::find_if(cov.modules.begin(), cov.modules.end(), [&](const ModuleStat& m) {
            return normalize_key(m.name, fold_case) == key;
        });
        // если итератор вернул не .end() ("прошли до конца, тк ничего не совпало"), а что-то совпало, то выходим
        // т.к. имена у всего продуктового уникальны
        if (duplicate != cov.modules.end())
            return std::unexpected(make_error(ParseReason::malformed, tag.begin, "duplicate module name"));

        // если с имененм всё в порядке, то добавляем модуль с метриками в вектор
        cov.modules.push_back(ModuleStat{name, covered, not_covered});
        if (!tag.self_closing)
            in_module = true;
    }
    // тегов не осталось, а состояние - "внутри module" -> незакрытый тег
    if (in_module)
        return std::unexpected(make_error(ParseReason::malformed, 0, "unclosed <module>"));
    if (cov.modules.empty())
        return std::unexpected(make_error(ParseReason::no_modules, 0, "no <module> in report"));

    std::ranges::sort(cov.modules, {}, &ModuleStat::name);
    return cov;
}

// ================================================================================================
// проход по манифесту и реальным блокам. если что-то из манифеста не находим в cov - оно идёт в missing
std::vector<std::string> missing_modules(const NativeCoverage& cov, std::span<const std::string> expected,
                                         bool fold_case) {
    std::vector<std::string> missing;
    for (const auto& e : expected) {
        // имя модуля из манифеста
        const std::string key = normalize_key(e, fold_case);
        // any_of - lambda, внутри - цикл
        // идём по всем cov-элементам, сравниваем имена каждого элем. cov с именем из манифеста
        // как только вернётся true, цикл в any_of
        // если true не было ни разу, дойдём до end() и вернётся false
        const bool found = std::any_of(cov.modules.begin(), cov.modules.end(),
                                       [&](const ModuleStat& m) { return normalize_key(m.name, fold_case) == key; });
        if (!found)
            missing.push_back(e);
    }
    return missing;
}

// ================================================================================================
// вернуть только модули из expected, в порядке expected
NativeCoverage select_modules(const NativeCoverage& cov, std::span<const std::string> expected, bool fold_case) {
    NativeCoverage out;
    for (const auto& e : expected) {
        const std::string key = normalize_key(e, fold_case);
        const auto it = std::find_if(cov.modules.begin(), cov.modules.end(),
                                     [&](const ModuleStat& m) { return normalize_key(m.name, fold_case) == key; });
        if (it != cov.modules.end())
            out.modules.push_back(*it);
    }
    return out;
}

// ================================================================================================
// вывод статистики покрытия по модулям
std::string format_block_summary(const NativeCoverage& cov) {
    if (cov.modules.empty())
        return {};

    std::size_t width = std::string_view("block coverage").size();
    for (const auto& m : cov.modules)
        width = std::max(width, m.name.size());

    std::string out = "\nblock coverage (native XML, per module; not a union):\n";
    for (const auto& m : cov.modules) {
        const std::uint64_t total = m.blocks_covered + m.blocks_not_covered;
        out += format_counts(m.name, m.blocks_covered, total, width);
    }
    return out;
}

std::string format_missing_modules(std::span<const std::string> missing) {
    return format_missing_list(missing, "missing product modules");
}
} // namespace cov
