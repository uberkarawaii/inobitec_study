# применение clang-format ко всем C/C++ файлам репозитория
# проверка
# cmake -DMODE=check -DSRC_DIR=<корень> -DBUILD_DIR=<каталог сборки> -P cmake/format.cmake
# отформатировать
# cmake -DMODE=apply -DSRC_DIR=<корень> -DBUILD_DIR=<каталог сборки> -P cmake/format.cmake
# graceful decay - если на машине нет clang-format, об этом будет предупреждение, и тесты пройдут
# если есть но не отформатировано - будет падение

# проверка что режим передали
if(NOT DEFINED MODE OR (NOT "${MODE}" STREQUAL "check" AND NOT "${MODE}" STREQUAL "apply"))
    message(FATAL_ERROR "MODE must be 'check' or 'apply'; received: '${MODE}'")
endif()
# проверка что указали папку, в которой форматировать файлы
if(NOT DEFINED SRC_DIR OR "${SRC_DIR}" STREQUAL "")
    message(FATAL_ERROR "SRC_DIR is not set")
endif()
# по умолчанию каталог сборки - корень
if(NOT DEFINED BUILD_DIR)
    set(BUILD_DIR "")
endif()

# поиск clang-format-a
find_program(CLANG_FORMAT NAMES clang-format)

# если нет - предупреждаем и идём к тестам
if(NOT CLANG_FORMAT)
    message(WARNING "clang-format was not found, so the file format was not checked. Install it to enable this check: "
                    "winget install --id=LLVM.ClangFormat -e (Windows) / sudo apt install clang-format (Linux)")
    return()
endif()

# список проверяемых / форматируемых файлов
file(GLOB_RECURSE FORMAT_FILES
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.h"
    "${SRC_DIR}/*.hpp")

# исключить поддиректорию билда из ресурсов для фоматирования, т.к. там не программы, а их продукты 
# а также продукты генерации CMake - не надо форматировать этот код
if(NOT "${BUILD_DIR}" STREQUAL "")
    list(FILTER FORMAT_FILES EXCLUDE REGEX "^${BUILD_DIR}/")
endif()

# если нечего форматировать - кинуть предупреждение
if(FORMAT_FILES STREQUAL "")
    message(WARNING "No C/C++ source files found under ${SRC_DIR}")
    return()
endif()
# форматирование
if("${MODE}" STREQUAL "apply")
    execute_process(COMMAND "${CLANG_FORMAT}" -i ${FORMAT_FILES}
                    RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "clang-format failed to format the files (exit code ${result})")
    endif()
    message(STATUS "clang-format: files formatted")
# проверка формата
else()
    execute_process(COMMAND "${CLANG_FORMAT}" --dry-run -Werror ${FORMAT_FILES}
                    RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "clang-format found format violations. Run "
                            "'cmake --build <build-dir> --target format' to fix them, then re-run tests.")
    endif()
    message(STATUS "clang-format: file format is correct")
endif()