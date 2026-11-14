# минимальная необходимая версия cmake + свежие политики (конкретные точки поведения) до 3.22 
# в старых CMake-ах выполнение остановится здесь, с требованием версии 3.22
# политика - как себя ведёт что-либо в CMake. если не указать версию, то default=OLD и будет warning, 
# т.к. старые политики - для совместимости, но не обещают корретности
cmake_minimum_required(VERSION 3.22)

# проверка наличия ninja, если задан именно этот генератор. до проверки компилятора в project() 
if(CMAKE_GENERATOR MATCHES "^Ninja")
  # find_program ищет в PATH и кэширует результат в CMakeCache.txt как NINJA_EXECUTABLE
  find_program(NINJA_EXECUTABLE NAMES ninja)
  if(NOT NINJA_EXECUTABLE)
    message(FATAL_ERROR
      "The '${CMAKE_GENERATOR}' generator was selected, but ninja was not found. "
      "Install ninja or add its directory to PATH")
  endif()
  # сообщение при конфигурации
  message(STATUS "ninja: ${NINJA_EXECUTABLE}")
# else отсутствует - пока что в проекте генератор - именно Ninja
endif()


# === ПРОВЕРКА BASELINE ===
# делается до project, иначе будет падение не от моей проверки с моим сообщением,
# а при vcpkg install - каскад падений

# надо понять: "вложена" ли требуемая версия в имеющийся инстанс vcpkg. для этого используем git
# проверка что он есть - иначе нет инструмента для проверки
find_program(GIT_EXECUTABLE NAMES git)
if(NOT GIT_EXECUTABLE)
  message(FATAL_ERROR "git was not found in PATH; it is required to check the vcpkg baseline")
endif()

# при поиске инстанса опираемся на переменную среды VCPKG_DEV_ROOT - если её нет, падаем
# иначе не сможем понять, какой именно инстанс
if(NOT DEFINED ENV{VCPKG_DEV_ROOT} OR "$ENV{VCPKG_DEV_ROOT}" STREQUAL "")
  message(FATAL_ERROR "VCPKG_DEV_ROOT is not set; it must point to the vcpkg clone used by the toolchain")
endif()

# чтение vcpkg.json из корня проекта, чтобы определить baseline
# _vcpkg_manifest - весь vcpkg.json; _m - builtin-baseline найденный по regex  
file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" _vcpkg_manifest)
string(REGEX MATCH "\"builtin-baseline\"[ \t]*:[ \t]*\"([0-9a-fA-F]+)\"" _m "${_vcpkg_manifest}")
# если builtin-baseline не нашёлся, то падение с ошибкой 
if(NOT _m)
  message(FATAL_ERROR "builtin-baseline was not found in vcpkg.json")
endif()
# CMAKE_MATCH_1 - то что попало в первую группу regex - т.е. ([0-9a-fA-F]+), сам sha
set(VCPKG_BASELINE "${CMAKE_MATCH_1}")

# запуск подпроцесса git из папки клона vcpkg
# cat-file -e X - есть ли объект Х в базе? результат: 0(успех) или не-нуль(нет в базе), уйдёт в _baseline_ok
# "${VCPKG_BASELINE}^{commit}" - объект-коммит выделенного sha. если такого коммита нет - падение с не-нуль  
execute_process(
  COMMAND "${GIT_EXECUTABLE}" -C "$ENV{VCPKG_DEV_ROOT}" cat-file -e "${VCPKG_BASELINE}^{commit}"
  RESULT_VARIABLE _baseline_ok
  OUTPUT_QUIET
  ERROR_QUIET)

# если такого объекта нет в базе - падение с сообщением. если есть - тоже оповещаем
if(NOT _baseline_ok EQUAL 0)
  message(FATAL_ERROR
    "builtin-baseline ${VCPKG_BASELINE} is not reachable in the vcpkg instance at "
    "$ENV{VCPKG_DEV_ROOT}. Run 'git fetch' in that clone and try again.")
endif()

message(STATUS "vcpkg baseline : ${VCPKG_BASELINE} (reachable in $ENV{VCPKG_DEV_ROOT})")

# выключение механизма vcpkg applocal, т.к. он вызывает конфликты (мн-во пересозданий себя в %TEMP%) при -j билде
set(VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "Deploy vcpkg runtime DLLs via CMake instead of vcpkg applocal" FORCE)

# === PROJECT === 
# имя проекта сохранится в CMAKE_PROJECT_NAME
# определяются яызки проекта: с с++; определится компилятор; 
# появятся перменные (типа CMAKE_C_FLAGS_DEBUG) заполненные дефолтными значениями
project(inobitec_stud VERSION 0.3.0 LANGUAGES C CXX)

# === ТИП СБОРКИ ===
# определение debug / release / coverage; делается через -DCMAKE_BUILD_TYPE=...
# Debug будет поставлен, если ничего не передали через флаг
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release/Coverage)" FORCE)
endif()
# переменная CMAKE_BUILD_TYPE уйдёт в кэш 
# кэш: для CMake, хранить инф. м-у выполнениями, и пользователь не повторяет ввод каждый раз
# FORCE вызовет перезапись параметра


# === ПРОВЕРКА ВЕРСИЙ КОМПОНЕНТОВ ===
# файл с ф-цией проверки и константами версий компонентов
include(${CMAKE_SOURCE_DIR}/cmake/toolchain_versions.cmake)
# добавляем к списку зависимостей ещё одну: при изменении toolchain_versions.cmake произойдёт реконфиг
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/cmake/toolchain_versions.cmake")

# CMake: нормализуем "4.3.1" -> "4.3" и сверяем с MIN/TESTED
to_major_minor(_cmake_ver "${CMAKE_VERSION}")
check_component(CMake "${_cmake_ver}" "${TOOLCHAIN_CMAKE_TESTED}" "${TOOLCHAIN_CMAKE_MIN}")

# компилятор: MSVC или GCC; сравнение по major.minor
if(MSVC)
  to_major_minor(_cc_ver "${CMAKE_CXX_COMPILER_VERSION}")   # "19.51.36247.0" -> "19.51"
  check_component(MSVC "${_cc_ver}" "${TOOLCHAIN_MSVC_TESTED}" "${TOOLCHAIN_MSVC_MIN}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  to_major_minor(_cc_ver "${CMAKE_CXX_COMPILER_VERSION}")   # "15.2.0" -> "15.2"
  check_component(GCC "${_cc_ver}" "${TOOLCHAIN_GCC_TESTED}" "${TOOLCHAIN_GCC_MIN}")
else()
  message(WARNING "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'; version not checked")
endif()

# Ninja: версия через --version; NINJA_EXECUTABLE найден до project()
if(NINJA_EXECUTABLE)
  execute_process(
  COMMAND "${NINJA_EXECUTABLE}" --version
  OUTPUT_VARIABLE _ninja_raw
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE _ninja_rc)                         
  # проверка на нормальный код завершения дочернего процесса               
  if(_ninja_rc STREQUAL "0")
    to_major_minor(_ninja_ver "${_ninja_raw}")       # "1.13.2" -> "1.13"
    check_component(Ninja "${_ninja_ver}" "${TOOLCHAIN_NINJA_TESTED}" "0")
  else()
    message(WARNING "could not determine Ninja version (result: ${_ninja_rc}); version check skipped")
  endif()
endif()

# git: строка вида "git version 2.54.0.windows.1"; GIT_EXECUTABLE найден до project()
# без to_major_minor т.к. у гита строка версии начинается с "git version" - потому простым regex-ом
execute_process(
  COMMAND "${GIT_EXECUTABLE}" --version
  OUTPUT_VARIABLE _git_raw
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE _git_rc)
if(_git_rc STREQUAL "0")
  string(REGEX MATCH "[0-9]+\\.[0-9]+" _git_ver "${_git_raw}")   # -> "2.54"
  check_component(git "${_git_ver}" "${TOOLCHAIN_GIT_TESTED}" "${TOOLCHAIN_GIT_MIN}")
else()
    message(WARNING "could not determine Git version (result: ${_git_rc}); version check skipped")
endif()

# vcpkg-tool: версия из метаданных клона; сравнение точное (дата релиза)
# CMAKE_MATCH_1 - результат последнего string(REGEX MATCH), содержит группу ([0-9-]+)
file(READ "$ENV{VCPKG_DEV_ROOT}/scripts/vcpkg-tool-metadata.txt" _vcpkg_meta)
string(REGEX MATCH "VCPKG_TOOL_RELEASE_TAG=([0-9-]+)" _vm "${_vcpkg_meta}")
check_component(vcpkg-tool "${CMAKE_MATCH_1}" "${TOOLCHAIN_VCPKG_TOOL_TESTED}" "0")

# какой инстанс vcpkg реально подключён - защита от тихой подмены инструмента
# CMAKE_TOOLCHAIN_FILE путь к скрипт vcpkg.cmake
# Z_VCPKG_ROOT_DIR корень vcpkg, вычисляется им самим. т.е. из какого инстанса идёт инструмент
# триплет: профиль сборки. у меня сейчас x64-windows - x64, винда, динамич. линковка
if(VCPKG_TARGET_TRIPLET)
  message(STATUS "vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
  if(DEFINED Z_VCPKG_ROOT_DIR)
    message(STATUS "vcpkg instance : ${Z_VCPKG_ROOT_DIR}")
  endif()
  message(STATUS "vcpkg triplet  : ${VCPKG_TARGET_TRIPLET}")
endif()

# Microsoft.CodeCoverage.Console (Windows) / gcovr (GNU/Linux) при coverage-сборке 
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  # ======= Windows =======
  if(WIN32)
    if(NOT MSVC) 
      message(FATAL_ERROR "coverage build requires MSVC") 
    endif()

    find_program(MICROSOFT_COVERAGE NAMES Microsoft.CodeCoverage.Console)
    if(NOT MICROSOFT_COVERAGE OR NOT EXISTS "${MICROSOFT_COVERAGE}")
    message(FATAL_ERROR "Coverage build was selected but Microsoft.CodeCoverage.Console was not found. "
    "Build from the x64 Native Tools Command Prompt, or add the tool directory to PATH. "
    "If the tool is not installed, add the Build Tools component "
    "'Microsoft.VisualStudio.Component.TestTools.BuildTools'.")
    endif()

    message(STATUS "Microsoft.CodeCoverage.Console was found: ${MICROSOFT_COVERAGE}")

    # существование проверено, теперь проверка версии
    execute_process(
      COMMAND "${MICROSOFT_COVERAGE}" --version
      OUTPUT_VARIABLE _coverage_raw
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE _coverage_rc)
    if(_coverage_rc STREQUAL "0")
      to_major_minor(_coverage_ver "${_coverage_raw}")
      check_component(CodeCoverage "${_coverage_ver}"
        "${TOOLCHAIN_CODECOVERAGE_TESTED}" "${TOOLCHAIN_CODECOVERAGE_MIN}")
    else()
      message(WARNING "could not determine Microsoft.CodeCoverage.Console version "
        "(result: ${_coverage_rc}); version check skipped")
    endif()

  # ======= LINUX =======
  # оба компилятора должны быть gnu, чтобы сработал --coverage, не было mismatch gcov/.gcno
  # цепочка --coverage -> .gcno/.gcda -> gcov -> gcovr -> Cobertura - именно под GNU
  elseif(UNIX
         AND CMAKE_C_COMPILER_ID STREQUAL "GNU"
         AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # находим инструмент gcovr
  find_program(GCOVR_EXECUTABLE NAMES gcovr)
    if(NOT GCOVR_EXECUTABLE OR NOT EXISTS "${GCOVR_EXECUTABLE}")
      message(FATAL_ERROR
        "Coverage build was selected but gcovr was not found. "
        "Install gcovr 8.x and add it to PATH: "
        "sudo apt install pipx; pipx install gcovr==8.8; pipx ensurepath")
    endif()

    message(STATUS "gcovr was found: ${GCOVR_EXECUTABLE}")

    # gcvor - какая версия?
    execute_process(
      COMMAND "${GCOVR_EXECUTABLE}" --version
      OUTPUT_VARIABLE _gcovr_raw
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE _gcovr_rc)
    # _gcov_rc - код exec_proc gcovr --version. если нулевой, то парс версии 
    if(_gcovr_rc STREQUAL "0")
      string(REGEX MATCH "gcovr ([0-9]+\\.[0-9]+)" _gcovr_match "${_gcovr_raw}")
      if(_gcovr_match)
        set(_gcovr_ver "${CMAKE_MATCH_1}")
        check_component(gcovr "${_gcovr_ver}"
          "${TOOLCHAIN_GCOVR_TESTED}" "${TOOLCHAIN_GCOVR_MIN}")
      else()
        message(WARNING "could not parse gcovr version from '${_gcovr_raw}'; "
          "version check skipped")
      endif()
    # если ненулевой, то gcovr --version выполнился с ошибкой, установить версию нельзя
    else()
      message(WARNING "could not determine gcovr version "
        "(result: ${_gcovr_rc}); version check skipped")
    endif()

  # ======= niether WINDOWS nor LINUX =======
  else()
    message(FATAL_ERROR "coverage build requires MSVC on Windows or GCC on Linux")
  endif()
endif()

# === СТАНДАРТЫ ===
# стандарт языка Си
set(CMAKE_C_STANDARD 17)
# компилятор обязан поддерживать этот стандарт языка. иначе - ошибка во время конфигурации CMakeLists (прекомпил.)
set(CMAKE_C_STANDARD_REQUIRED ON)
# только стандартный компилятор языка, без расширений, типа gnu-расширений
set(CMAKE_C_EXTENSIONS OFF)
# то же для с++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
                               
# особые флаги для coverage-сборки. /Ob1 - инлайн только того, что явно inline, не всего, что захочет компилятор
# условный FORCE чтобы при пустых CMAKE_*_FLAGS_COVERAGE записть приосходила
if(MSVC)

  if(NOT CMAKE_C_FLAGS_COVERAGE)
    set(CMAKE_C_FLAGS_COVERAGE "/O2 /Ob1 /Zi /DNDEBUG"
      CACHE STRING "Flags used by the C compiler during Coverage builds." FORCE)
  endif()
  if(NOT CMAKE_CXX_FLAGS_COVERAGE)
    set(CMAKE_CXX_FLAGS_COVERAGE "/O2 /Ob1 /Zi /DNDEBUG"
      CACHE STRING "Flags used by the C++ compiler during Coverage builds." FORCE)
  endif()
  if(NOT CMAKE_EXE_LINKER_FLAGS_COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "/DEBUG"
      CACHE STRING "Linker flags for Coverage builds." FORCE)
  endif()
  if(NOT CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "/DEBUG"
      CACHE STRING "Shared linker flags for Coverage builds." FORCE)
  endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # -O0: без инлайна, чтобы строки и блоки не разъезжались
  # --coverage нужен и при компиляции, и при линковке (libgcov)
  if(NOT CMAKE_C_FLAGS_COVERAGE)
    set(CMAKE_C_FLAGS_COVERAGE "-O0 -g -DNDEBUG --coverage"
        CACHE STRING "Flags used by the C compiler during Coverage builds." FORCE)
  endif()
  if(NOT CMAKE_CXX_FLAGS_COVERAGE)
    set(CMAKE_CXX_FLAGS_COVERAGE "-O0 -g -DNDEBUG --coverage"
        CACHE STRING "Flags used by the C++ compiler during Coverage builds." FORCE)
  endif()
  if(NOT CMAKE_EXE_LINKER_FLAGS_COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage"
        CACHE STRING "Linker flags for Coverage builds." FORCE)
  endif()
  if(NOT CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "--coverage"
        CACHE STRING "Shared linker flags for Coverage builds." FORCE)
  endif()
endif()


# === ИСПРАВЛЕНИЕ DEFAULT-ФЛАГОВ ===
# CMAke дефолтно положит /RTC1 в debug-ветку. /RTC1 ловит ошибки в runtime, для этого лезет в стек и размечает его
# ASan - та же ф-ция. если оба будут менять стек, они помешают друг другу -> несовместимы. /RTC1 убирается 
# /incremental также несовместим с asan (именно по документации. так работало и когда оба вместе)
if(MSVC)
  string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  string(REPLACE "/INCREMENTAL" "" CMAKE_EXE_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
endif()


# === ПОСТРОИТЬ КОМАНДЫ КОМПИЛЯЦИИ И ЛИНКА ===
function(configure_target trgt PRODUCT_TARGET)
if(MSVC)
  # минимальная поддерживаемая ОС - win7 (и все дальнейшие)
  target_compile_definitions(${trgt} PRIVATE _WIN32_WINNT=0x0601)
  # в msvc /Zi, /EHsc, /DEBUG  будут дублироваться, но прописываются для ясности
  # target_compile_options - произвольные флаги компилятора 
  # PRIVATE даёт нетранзитивность: А (этот объект) и Б к нему сможет линковаться. а когда К линк. к Б, то К не будет линк. к А
  target_compile_options(${trgt} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/EHsc> /W4 /permissive-)

  # сюда будет заходить только для программ из задач, не для программ для тестирования (run_case, check) - им эти флаги не нужны
  if(PRODUCT_TARGET)
    # CMake проставит след. флаги автоматиечки: /Od (на release /O2 /Ob2); /MDd /MD; /DNDEBUG
    target_compile_options(${trgt} PRIVATE $<$<CONFIG:Debug>:/Zi /fsanitize=address> $<$<CONFIG:Release>:/Zi>)
    target_link_options(${trgt} PRIVATE $<$<CONFIG:Debug>:/DEBUG> $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
  endif()

else()
  target_compile_options(${trgt} PRIVATE -Wall -Wextra)
  # и в ветке продуктов, и в ветке тестовых программ оптимизация на релизе будет именно -O2, а не cmake-default -O3
  if (PRODUCT_TARGET) 
    # -O0 на дебаге будет подтянут CMake автоматически, а на релизе -DNDEBUG
    target_compile_options(${trgt} PRIVATE $<$<CONFIG:Debug>: -g -fsanitize=address> $<$<CONFIG:Release>: -g -O2>)
    target_link_options(${trgt} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address>)
  else()
    target_compile_options(${trgt} PRIVATE $<$<CONFIG:Release>: -O2>)
  endif()
endif()
endfunction()


# === Ф-ЦИЯ ДОБАВЛЕНИЯ ЗАДАЧИ === 
# сборки exe из main.c/main.cpp; уходит в каталог build/cfg/dir
function(add_task dir trgt src lib)
  # добавление исполняемного файла 
  # цель и её исходник
  # цель t1_c отдаёт файл t1_c.exe; уникальность имени нужна coverage-инструменту, иначе модули слипаются
  add_executable(${trgt} ${src})

  # в CMAKE_BINARY_DIR будет тот каталог, который был передан по -B
  set_target_properties(${trgt} PROPERTIES
  OUTPUT_NAME ${trgt}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${dir})

  # с кем линкуемся
  # тут private не имеет смысла (.exe конечен, к нему не линкуются), просто для единообразия
  target_link_libraries(${trgt} PRIVATE ${lib})

  # определение типа библиотеки т.к. на windows для .dll нужно доп. действие для связки с .exe
  get_target_property(lib_type ${lib} TYPE)
  # копирование dll в папку к exe даже если сам exe не перестраивался
  if(WIN32 AND lib_type STREQUAL "SHARED_LIBRARY")
    set(copied_dll "${CMAKE_BINARY_DIR}/${dir}/${lib}.dll")
    add_custom_command(OUTPUT "${copied_dll}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${lib}> "${CMAKE_BINARY_DIR}/${dir}"
      DEPENDS ${lib} 
      COMMENT "Copying DLL ${lib} next to ${trgt}"
      VERBATIM)
    add_custom_target(${trgt}_dll DEPENDS "${copied_dll}")
    add_dependencies(${trgt} ${trgt}_dll)
  endif()

  # команда для cl+link. флаг PRODUCT_TARGET = ON - т.е. пойдём по ветке if (PRODUCT_TARGET) с флагами для программ-задач
  configure_target(${trgt} ON)

  # в переменной-свойстве накапливаем то, для чего вызывался add_task - так получаем список продуктовых целей
  set_property(GLOBAL APPEND PROPERTY PRODUCT_TARGETS "${trgt}")
 
endfunction()

# генерация version.h из шаблона
# @PROJECT_VERSION@ подставится реальная версия из project(VERSION)
# @ONLY - подставляются только @VAR@, а не ${...} (в шаблоне только @PROJECT_VERSION@). для защиты
# /generated в каталоге бинарей будет создан configure_file-ом, если такого каталога ещё нет
configure_file(
  ${CMAKE_SOURCE_DIR}/common/version.h.in
  ${CMAKE_BINARY_DIR}/generated/version.h
  @ONLY)

# === ПОДПРОЕКТ ДЛЯ СОЗДАНИЯ БИБЛИОТЕК === 
# add_subdirectory(source_dir [binary_dir] ...)
# source_dir - где лежит вложенный CMakeLists.txt /common)
# binary_dir - где положить служеб. файлы CMake для подпроекта (если не указано, то в CMAKE_BINARY_DIR/source_dir)  
add_subdirectory(common)


# === ДОБАВЛЕНИЕ ЗАДАЧ ===
# arg1 - папка бинарей и прочих продуктов (build/cfg/...)
# arg2 - логическое имя цели
# arg3 - исходник, чтобы сделать файл цели
# arg4 - библиотека с которой линковаться

# TASK 1
add_task(t1_dist_matrix_cpp t1_cpp t1_dist_matrix_cpp/main.cpp common_cpp_static)
add_task(t1_dist_matrix_c t1_c t1_dist_matrix_c/main.c common_c_static)

# TASK 2
add_task(t2_passport_cpp t2_cpp t2_passport_cpp/main.cpp string_utils_cpp_static)
add_task(t2_passport_c t2_c t2_passport_c/main.c string_utils_c_static)

# TASK 3
add_task(t3_bbox_cpp t3_cpp t3_bbox_cpp/main.cpp common_cpp_shared)
add_task(t3_bbox_c t3_c t3_bbox_c/main.c common_c_shared)

# TASK 4
add_task(t4_filter_cpp t4_cpp t4_filter_cpp/main.cpp common_cpp_shared)
add_task(t4_filter_c t4_c t4_filter_c/main.c common_c_shared)

# guard чтобы у инструментированного были уникальные имена. иначе блок-метрика слипнется
# если когда-то имена модулей будут одинаковые, то падение
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  get_property(_product_targets GLOBAL PROPERTY PRODUCT_TARGETS)
  set(_output_names "")
  foreach(_t IN LISTS _product_targets)
    get_target_property(_out ${_t} OUTPUT_NAME)
    if(_out IN_LIST _output_names)
      message(FATAL_ERROR "duplicate OUTPUT_NAME '${_out}' among product targets (${_t}); coverage module names must be unique")
    endif()
    list(APPEND _output_names "${_out}")
  endforeach()
endif()

# === ФОРМАТИРОВАНИЕ ===

# cmake/format.cmake в скриптовом режиме. отделен он графа сборки проекта
set(FORMAT_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/format.cmake")

# исключаем весь каталог сборки, а не только текущую конфигурацию:
# конфигурации вложены (build/debug, build/release)
set(FORMAT_BUILD_ROOT "${CMAKE_BINARY_DIR}")
get_filename_component(FORMAT_BUILD_PARENT "${CMAKE_BINARY_DIR}" DIRECTORY)
if(NOT FORMAT_BUILD_PARENT STREQUAL CMAKE_SOURCE_DIR)
  set(FORMAT_BUILD_ROOT "${FORMAT_BUILD_PARENT}")
endif()

# один и тот же source-dir и build-dir, изменяется только check/apply - он добавляется в командах
# общая часть из source-dir и build-dir
set(FORMAT_COMMON -DSRC_DIR=${CMAKE_SOURCE_DIR} -DBUILD_DIR=${FORMAT_BUILD_ROOT} -P ${FORMAT_SCRIPT})

# отформатировать
add_custom_target(format
  COMMAND ${CMAKE_COMMAND} -DMODE=apply ${FORMAT_COMMON}
  COMMENT "Applying clang-format to all C/C++ files")

# проверить формат (в этом же виде вызывается из ctest)
add_custom_target(format-check
  COMMAND ${CMAKE_COMMAND} -DMODE=check ${FORMAT_COMMON}
  COMMENT "Checking clang-format of all C/C++ files")

# === TESTS ===
enable_testing()
# timeout-ограничение на тесты
set(TESTS_TIMEOUT 30)
# первый тест: проверка чистоты формата
# если есть clang+format - проверка строгая, не-падение - только при чистом формате
# иначе - warning и тесты
add_test(NAME format-check COMMAND ${CMAKE_COMMAND} -DMODE=check ${FORMAT_COMMON})
# на format-check также ставится таймаут, для единообразия с остальными тестами
set_tests_properties(format-check PROPERTIES TIMEOUT ${TESTS_TIMEOUT}) 

# при coverage-build должны быть именно релиз.версии внешних библиотек, для совместимости с бинарниками задач
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  set(CMAKE_MAP_IMPORTED_CONFIG_COVERAGE "Release")
endif()

# библиотека с парсером cobertura.xml, тестируется юнит-тестами
add_subdirectory(tools/coverage_union)
# тесты на знчения и коды
add_subdirectory(tests) 

# особая цель при сборке с покрытием
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  # в PRODUCT_TARGETS записаны все задачи
  get_property(_tasks GLOBAL PROPERTY PRODUCT_TARGETS)
  # инструментироваться будут и задачи, и dll/so-библиотеки
  set(_targets ${_tasks} common_c_shared common_cpp_shared)

  # --- манифест исходников: общий для обеих ОС ---
  set(_sources "")
  foreach(_t IN LISTS _targets)
    get_target_property(_dir ${_t} SOURCE_DIR)   # где цель объявлена
    get_target_property(_srcs ${_t} SOURCES)     # main.c / geometry.c / ...
    foreach(_s IN LISTS _srcs)
      if(NOT IS_ABSOLUTE "${_s}")
        set(_s "${_dir}/${_s}")
      endif()
      list(APPEND _sources "${_s}")
    endforeach()
  endforeach()

  # страховка: если позже добавим common_*_static, geometry.c придёт из двух целей
  list(REMOVE_DUPLICATES _sources)
  list(SORT _sources)
  string(JOIN "\n" _expected_text ${_sources})
  file(WRITE "${CMAKE_BINARY_DIR}/coverage_expected_sources.txt" "${_expected_text}\n")

  # --- платформенно-зависимая часть ---
  if(WIN32)
    # манифест модулей для block-метрики: только продуктовые цели
    # (8 задач + common_*_shared), unit-тесты сюда не входят
    set(_expected_modules "")
    foreach(_t IN LISTS _targets)
      get_target_property(_out ${_t} OUTPUT_NAME)
      if(NOT _out)
        set(_out "${_t}")
      endif()

      get_target_property(_type ${_t} TYPE)
      if(_type STREQUAL "EXECUTABLE")
        set(_suffix "${CMAKE_EXECUTABLE_SUFFIX}")
      elseif(_type STREQUAL "SHARED_LIBRARY")
        set(_suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        message(FATAL_ERROR "unexpected target type for ${_t}: ${_type}")
      endif()

      list(APPEND _expected_modules "${_out}${_suffix}")
    endforeach()

    # fail-fast на одинаковые имена модулей: native XML не должен получить дубли
    list(LENGTH _expected_modules _modules_before)
    list(REMOVE_DUPLICATES _expected_modules)
    list(LENGTH _expected_modules _modules_after)
    if(NOT _modules_before EQUAL _modules_after)
      message(FATAL_ERROR "duplicate module file names among coverage targets")
    endif()

    list(SORT _expected_modules)
    string(JOIN "\n" _expected_modules_text ${_expected_modules})
    file(WRITE "${CMAKE_BINARY_DIR}/coverage_expected_modules.txt" "${_expected_modules_text}\n")

    set(_coverage_backend "msvc")
    set(_coverage_tool_args
      "-DCOVERAGE_EXE=${MICROSOFT_COVERAGE}"
      "-DSETTINGS_FILE=${CMAKE_SOURCE_DIR}/cmake/coverage.msvc.settings.xml"
      "-DEXPECTED_MODULES=${CMAKE_BINARY_DIR}/coverage_expected_modules.txt")
  else()
    set(_coverage_backend "gcovr")
    set(_coverage_tool_args
      "-DGCOVR_EXE=${GCOVR_EXECUTABLE}")
  endif()

  add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND}
      "-DCOVERAGE_BACKEND=${_coverage_backend}"
      "-DCTEST_EXE=${CMAKE_CTEST_COMMAND}"
      "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
      "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
      "-DREPORT_DIR=${CMAKE_BINARY_DIR}/reports"
      "-DCOVERAGE_UNION_EXE=$<TARGET_FILE:coverage_union>"
      "-DEXPECTED_SOURCES=${CMAKE_BINARY_DIR}/coverage_expected_sources.txt"
      ${_coverage_tool_args}
      -P "${CMAKE_SOURCE_DIR}/cmake/coverage.cmake"
    DEPENDS all
    COMMENT "Collecting code coverage"
    VERBATIM)

  # coverage_union собран как EXCLUDE_FROM_ALL; DEPENDS all его не поднимет
  add_dependencies(coverage coverage_union)
endif()
