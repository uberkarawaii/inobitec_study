#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

// coverage_union_lib имеет PUBLIC include dir на tools/coverage_union
// т.е. папка с этим hpp будет среди путей поиска, в относит.пути папки не нужны
#include "coverage_union.hpp"

namespace {

// ---------- общее для успешных проверок парсера ----------

struct ExpectedFile {
    std::string key;          // нормализованный под ОС ключ (как в Coverage::files)
    std::string display_path; // путь как в XML
    std::map<int, std::uint64_t> hits;
};

// проверка Coverage по ожидаемым файлам
// сейчас вывзывается только из TEST_P, если же появится вызов из разных мест, заменить ASSERT_NE на EXPECT_*
// чтобы не оборачивать все места в проверку того, не упал ли expect_coverage
void expect_coverage(const cov::Coverage& got, const std::vector<ExpectedFile>& want) {
    ASSERT_EQ(got.files.size(), want.size());

    for (const auto& w : want) {
        const auto it = got.files.find(w.key);
        // NE=not equal - ожидаем, что найденный it != концу,т.е. существует. иначе - failure и return из этой ф-ции
        ASSERT_NE(it, got.files.end()) << "no key: " << w.key;
        // для данного файла ожидаем равенство путей и замеров по покрытию строк
        EXPECT_EQ(it->second.display_path, w.display_path);
        EXPECT_EQ(it->second.hits, w.hits);
    }
}

// для summarize/missing_expected: формирование объекта Coverage
cov::Coverage make_coverage(const std::vector<ExpectedFile>& files) {
    cov::Coverage cov;
    for (const auto& f : files)
        cov.files[f.key] = cov::FileCoverage{f.display_path, f.hits};
    return cov;
}

// ---------- парсер: успешные кейсы ----------

struct ParseOkCase {
    std::string_view xml;
    bool fold_case = false; // true -> ключ в lower-case; false -> как есть
    std::vector<ExpectedFile> want;
};

// читаемый вывод упавшего кейса
std::ostream& operator<<(std::ostream& os, const ParseOkCase& c) {
    os << "xml=\"" << c.xml << "\", fold_case=" << c.fold_case;
    return os;
}

// шаблонный класс-suite для TEST_P
class ParseCoberturaOk : public ::testing::TestWithParam<ParseOkCase> {};

// параметризованный тест; внутри GetParam() возвращает текущий ParseOkCase
TEST_P(ParseCoberturaOk, Parses) {
    const ParseOkCase& c = GetParam();
    // проверяем, что парс cobertura.xml произошёл без ошибок
    const auto got = cov::parse_cobertura(c.xml, c.fold_case);
    ASSERT_TRUE(got.has_value()) << "reason=" << static_cast<int>(got.error().reason) << " what=" << got.error().what;
    // другой ф-цией проверяем, что результат равен ожиданию
    expect_coverage(*got, c.want);
    // внутри expect_coverage - ASSERT_NE, потому если после его вызова появятся другие проверки,
    // если вероятность проверять данные, на которых expect_coverage упал и сделал return
}

// === строковые константы входных данных для успешных кейсов ===
constexpr std::string_view kXmlMinimal = R"xml(<?xml version="1.0" encoding="utf-8"?>
<coverage>
  <packages>
    <package name="p">
      <classes>
        <class filename="C:\src\a.c">
          <methods>
            <method name="m">
              <lines>
                <line number="1" hits="1" />
              </lines>
            </method>
          </methods>
        </class>
      </classes>
    </package>
  </packages>
</coverage>)xml";

constexpr std::string_view kXmlTwoClasses = R"xml(<coverage>
  <class filename="C:\src\a.c">
    <lines>
      <line number="1" hits="0" />
      <line number="2" hits="3" />
    </lines>
  </class>
  <class filename="C:\src\a.c">
    <lines>
      <line number="1" hits="5" />
      <line number="2" hits="1" />
      <line number="3" hits="0" />
    </lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlComments = R"xml(<!-- <class filename="fake.c"><line number="99" hits="9" /></class> -->
<coverage>
  <!-- <class filename="fake2.c" /> -->
  <class filename="C:\src\real.c">
    <lines>
      <!-- <line number="77" hits="7" /> -->
      <line number="4" hits="1" />
      <!-- <line number="5" hits="1" /> -->
    </lines>
  </class>
</coverage>
<!-- <class filename="fake3.c" /> -->)xml";

constexpr std::string_view kXmlSelfClosing = R"xml(<coverage>
  <class filename="C:\src\empty.c" />
  <class filename="C:\src\real.c">
    <lines>
      <line number="2" hits="1" />
    </lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlPathForms = R"xml(<coverage>
  <class filename="C:\a\b.c">
    <lines><line number="1" hits="1" /></lines>
  </class>
  <class filename="C:/a/b.c">
    <lines><line number="2" hits="0" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlFoldCase = R"xml(<coverage>
  <class filename="C:\SRC\A.C">
    <lines><line number="1" hits="1" /></lines>
  </class>
  <class filename="c:/src/a.c">
    <lines><line number="2" hits="1" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlEntity = R"xml(<coverage>
  <class filename="C:\src\a&amp;b.c">
    <lines><line number="5" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlBOM = "\xEF\xBB\xBF"
                                     R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="2" hits="5" /></lines>
  </class>
</coverage>)xml";

// === удачные кейсы, и их инстанциация ===
INSTANTIATE_TEST_SUITE_P(OkCases, ParseCoberturaOk,
                         ::testing::Values(
                             // 1. Минимальный отчёт: один файл, одна строка.
                             ParseOkCase{
                                 kXmlMinimal,
                                 false,
                                 {
                                     {"C:/src/a.c", R"(C:\src\a.c)", {{1, 1}}},
                                 },
                             },
                             // 2. Два <class> одного файла: hits объединяются по max.
                             ParseOkCase{kXmlTwoClasses,
                                         false,
                                         {
                                             {"C:/src/a.c", R"(C:\src\a.c)", {{1, 5}, {2, 3}, {3, 0}}},
                                         }},
                             // 3. Комментарии до/после/внутри класса; фейковые class/line в комментариях.
                             ParseOkCase{kXmlComments,
                                         false,
                                         {
                                             {"C:/src/real.c", R"(C:\src\real.c)", {{4, 1}}},
                                         }},
                             // 4. Self-closing <class ... /> с filename рядом с реальным классом.
                             ParseOkCase{kXmlSelfClosing,
                                         false,
                                         {
                                             {"C:/src/empty.c", R"(C:\src\empty.c)", {}},
                                             {"C:/src/real.c", R"(C:\src\real.c)", {{2, 1}}},
                                         }},
                             // 5. C:\a\b.c и C:/a/b.c дают один нормализованный ключ.
                             ParseOkCase{kXmlPathForms,
                                         false,
                                         {
                                             {"C:/a/b.c", R"(C:\a\b.c)", {{1, 1}, {2, 0}}},
                                         }},
                             // 6. fold_case=true: пути, отличающиеся регистром, сливаются.
                             ParseOkCase{kXmlFoldCase,
                                         true,
                                         {
                                             {"c:/src/a.c", R"(C:\SRC\A.C)", {{1, 1}, {2, 1}}},
                                         }},
                             // 7. XML-сущность & в filename; в C++-ожидании уже настоящий '&'.
                             ParseOkCase{kXmlEntity,
                                         false,
                                         {
                                             {"C:/src/a&b.c", R"(C:\src\a&b.c)", {{5, 2}}},
                                         }},
                             // 8. BOM-кодировка в начале
                             ParseOkCase{kXmlBOM,
                                         false,
                                         {
                                             {"C:/src/a.c", R"(C:\src\a.c)", {{2, 5}}},
                                         }}));

// ---------- парсер: ошибки ----------

struct ParseErrCase {
    std::string_view xml;
    cov::ParseReason want;
};

std::ostream& operator<<(std::ostream& os, const ParseErrCase& c) {
    os << "xml=\"" << c.xml << "\", want=" << static_cast<int>(c.want);
    return os;
}

class ParseCoberturaErr : public ::testing::TestWithParam<ParseErrCase> {};

TEST_P(ParseCoberturaErr, Rejects) {
    const ParseErrCase& c = GetParam();

    const auto got = cov::parse_cobertura(c.xml, false);
    ASSERT_FALSE(got.has_value());
    EXPECT_EQ(got.error().reason, c.want);
    // проверка, что текст ошибки непустой
    EXPECT_FALSE(got.error().what.empty());
    // и что указатель ошибки - в пределах
    EXPECT_LE(got.error().offset, c.xml.size());
}

// === строковые литералы для ошибочных кейсов ====
constexpr std::string_view kXmlNoClasses = R"xml(<coverage><packages/></coverage>)xml";

constexpr std::string_view kXmlNoLines = R"xml(<coverage>
  <class filename="C:\src\a.c"></class>
</coverage>)xml";

constexpr std::string_view kXmlNestedClass = R"xml(<coverage>
  <class filename="C:\src\a.c">
    <class filename="C:\src\a.c">
      <lines><line number="5" hits="2" /></lines>
    </class>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlUnclosedClass = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="5" hits="2" /></lines>
</coverage>)xml";

constexpr std::string_view kXmlNotOpenedClass = R"xml(<coverage>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineOutsideClass = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="5" hits="2" /></lines>
  </class>
  <lines><line number="5" hits="2" /></lines>
</coverage>)xml";

constexpr std::string_view kXmlUnclosedComment = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <!-- an unclosed comment
      <lines><line number="5" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlDoctype = R"xml(<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE>
<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="5" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlCdata = R"xml(<?xml version="1.0" encoding="utf-8"?>
<![CDATA[ info ]]>
<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="5" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlNoFilename = R"xml(<coverage>
  <class>
      <lines><line number="5" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlEmptyFilename = R"xml(<coverage>
  <class filename="">
      <lines><line number="5" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineWithoutNumber = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineWithoutHits = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineNumberNotDigit = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="uwu" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineHitsNotDigit = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="2" hits="uwu" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineNumberZero = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="0" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineNumberTooMuch = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="2147483648" hits="2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineHitsNegative = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="2" hits="-2" /></lines>
  </class>
</coverage>)xml";

constexpr std::string_view kXmlLineHitsTooMuch = R"xml(<coverage>
  <class filename="C:\src\a.c">
      <lines><line number="2" hits="18446744073709551616" /></lines>
  </class>
</coverage>)xml";

// === ошибочные кейсы и их инстанциация ===
INSTANTIATE_TEST_SUITE_P(ErrCases, ParseCoberturaErr,
                         ::testing::Values(
                             // нет ни <class>, ни <line>
                             ParseErrCase{kXmlNoClasses, cov::ParseReason::no_classes},
                             // нет строк, только <class> без <line>
                             ParseErrCase{kXmlNoLines, cov::ParseReason::no_lines},
                             // кейс для проверки поведения parse_cobertura на пустой строке
                             ParseErrCase{"", cov::ParseReason::no_classes},
                             // malformed: nested <class>
                             ParseErrCase{kXmlNestedClass, cov::ParseReason::malformed},
                             // malformed: unclosed <class>
                             ParseErrCase{kXmlUnclosedClass, cov::ParseReason::malformed},
                             // malformed: </class> без открытого
                             ParseErrCase{kXmlNotOpenedClass, cov::ParseReason::malformed},
                             // malformed: <line> вне <class>
                             ParseErrCase{kXmlLineOutsideClass, cov::ParseReason::malformed},
                             // malformed: незакрытый комментарий / <?...?> / тег
                             ParseErrCase{kXmlUnclosedComment, cov::ParseReason::malformed},
                             // malformed: DOCTYPE
                             ParseErrCase{kXmlDoctype, cov::ParseReason::malformed},
                             // malformed: CDATA
                             ParseErrCase{kXmlCdata, cov::ParseReason::malformed},
                             // malformed: <class> без filename
                             ParseErrCase{kXmlNoFilename, cov::ParseReason::malformed},
                             // malformed: <class> с filename=""
                             ParseErrCase{kXmlEmptyFilename, cov::ParseReason::malformed},
                             // malformed: line без number
                             ParseErrCase{kXmlLineWithoutNumber, cov::ParseReason::malformed},
                             // malformed: line без hits
                             ParseErrCase{kXmlLineWithoutHits, cov::ParseReason::malformed},
                             // malformed: line где number нечисло
                             ParseErrCase{kXmlLineNumberNotDigit, cov::ParseReason::malformed},
                             // malformed: line где hits нечисло
                             ParseErrCase{kXmlLineHitsNotDigit, cov::ParseReason::malformed},
                             // malformed: line с number=0
                             ParseErrCase{kXmlLineNumberZero, cov::ParseReason::malformed},
                             // malformed: line с number>INT_MAX
                             ParseErrCase{kXmlLineNumberTooMuch, cov::ParseReason::malformed},
                             // malformed: line с hits < 0
                             ParseErrCase{kXmlLineHitsNegative, cov::ParseReason::malformed},
                             // malformed: line с hits > UINT64_MAX
                             ParseErrCase{kXmlLineHitsTooMuch, cov::ParseReason::malformed}));

// ---------- summarize ----------

// параметры - сами литералы внутри TEST, извне параметры не приходят
TEST(Summarize, ComputesTotalsAndUncovered) {
    const cov::Coverage coverage = make_coverage({
        {"c:/a.c", R"(C:\a.c)", {{1, 1}, {2, 0}}},
        {"c:/b.c", R"(C:\b.c)", {{5, 3}}},
    });

    const cov::Summary s = cov::summarize(coverage, "");

    EXPECT_EQ(s.covered, 2);
    EXPECT_EQ(s.valid, 3);
    ASSERT_EQ(s.files.size(), 2u);

    // порядок files — по ключам map: c:/a.c, c:/b.c
    EXPECT_EQ(s.files[0].path, R"(C:\a.c)");
    EXPECT_EQ(s.files[0].covered, 1);
    EXPECT_EQ(s.files[0].valid, 2);

    EXPECT_EQ(s.files[1].path, R"(C:\b.c)");
    EXPECT_EQ(s.files[1].covered, 1);
    EXPECT_EQ(s.files[1].valid, 1);

    ASSERT_EQ(s.uncovered.size(), 1u);
    EXPECT_EQ(s.uncovered[0].path, R"(C:\a.c)");
    EXPECT_EQ(s.uncovered[0].lines, std::vector<int>{2});
}

// ---------- format_summary ----------

// проверка на корректных данных
TEST(FormatSummary, ContainsTotalAndUncovered) {
    cov::Summary s;
    s.files = {cov::FileStat{"a.c", 1, 2}};
    s.covered = 1;
    s.valid = 2;
    s.uncovered = {cov::Uncovered{"a.c", {2}}};

    const std::string text = cov::format_summary(s);

    EXPECT_NE(text.find("TOTAL"), std::string::npos);
    EXPECT_NE(text.find("1/2"), std::string::npos);
    EXPECT_NE(text.find("50.00%"), std::string::npos);
    EXPECT_NE(text.find("uncovered lines:"), std::string::npos);
    EXPECT_NE(text.find("a.c: 2"), std::string::npos);
}
// "краевые" данные, при valid == 0
// проверка, что format_summary не делит на ноль и не печатает пустую секцию uncovered

TEST(FormatSummary, ZeroValidAndNoUncovered) {
    cov::Summary s;
    s.files = {cov::FileStat{"a.c", 0, 0}};
    s.covered = 0;
    s.valid = 0;

    const std::string text = cov::format_summary(s);

    EXPECT_NE(text.find("0/0"), std::string::npos);
    EXPECT_NE(text.find("0.00%"), std::string::npos);
    EXPECT_EQ(text.find("uncovered lines:"), std::string::npos);
}

// ---------- missing_expected ----------

struct MissingCase {
    std::vector<std::string> expected;
    bool fold_case = false;
    std::vector<std::string> want_missing;
};

std::ostream& operator<<(std::ostream& os, const MissingCase& c) {
    os << "fold_case=" << c.fold_case;
    return os;
}

class MissingExpectedCases : public ::testing::TestWithParam<MissingCase> {};

TEST_P(MissingExpectedCases, Finds) {
    const MissingCase& c = GetParam();

    const cov::Coverage coverage = make_coverage({
        {"c:/a.c", R"(C:\a.c)", {{1, 1}}},       // есть и hits
        {"c:/empty.c", R"(C:\empty.c)", {}},     // valid == 0 -> missing
        {"c:/zero.c", R"(C:\zero.c)", {{2, 0}}}, // hits есть, но 0 -> НЕ missing
    });

    const auto got = cov::missing_expected(coverage, c.expected, c.fold_case);
    EXPECT_EQ(got, c.want_missing);
}

INSTANTIATE_TEST_SUITE_P(MissingCases, MissingExpectedCases,
                         ::testing::Values(MissingCase{{"c:/a.c"}, false, {}}, MissingCase{{"C:/A.C"}, true, {}},
                                           MissingCase{{"C:/A.C"}, false, {"C:/A.C"}},
                                           MissingCase{{"c:/empty.c"}, false, {"c:/empty.c"}},
                                           MissingCase{{"c:/zero.c"}, false, {}},
                                           MissingCase{{"c:/absent.c"}, false, {"c:/absent.c"}}));

} // namespace
