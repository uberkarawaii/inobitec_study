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

# имя проекта сохранится в CMAKE_PROJECT_NAME
# определяются яызки проекта: с с++; определится компилятор; 
# появятся перменные (типа CMAKE_C_FLAGS_DEBUG) заполненные дефолтными значениями
project(inobitec_stud LANGUAGES C CXX)

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

                               
# === ТИП СБОРКИ ===
# определение debug / release; делается через -DCMAKE_BUILD_TYPE=...
# Debug будет поставлен, если ничего не передали через флаг
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug/Release)" FORCE)
endif()
# переменная CMAKE_BUILD_TYPE уйдёт в кэш 
# кэш: для CMake, хранить инф. м-у выполнениями, и пользователь не повторяет ввод каждый раз
# FORCE вызовет перезапись параметра


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
function(configure_target trgt DBG)
if(MSVC)
  # в msvc /Zi, /EHsc, /DEBUG  будут дублироваться, но прописываются для ясности
  # target_compile_options - произвольные флаги компилятора 
  # PRIVATE даёт нетранзитивность: А (этот объект) и Б к нему сможет линковаться. а когда К линк. к Б, то К не будет линк. к А
  target_compile_options(${trgt} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/EHsc> /W4 /permissive-)

  # сюда будет заходить только для программ из задач, не для программ для тестирования (run_case, check) - им эти флаги не нужны
  if(DBG)
    # CMake проставит след. флаги автоматиечки: /Od /O2; /MDd /MD; /DNDEBUG
    target_compile_options(${trgt} PRIVATE $<$<CONFIG:Debug>:/Zi /fsanitize=address> $<$<CONFIG:Release>:/Zi>)
    target_link_options(${trgt} PRIVATE $<$<CONFIG:Debug>:/DEBUG> $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
  endif()

else()
  target_compile_options(${trgt} PRIVATE -Wall -Wextra)
  # и в ветке продуктов, и в ветке тестовых программ оптимизация на релизе будет именно -O2, а не cmake-default -O3
  if (DBG) 
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
  # цель и её исходник. цель имеет одно имя (логическое, типа t1_c), а сделанный ею файл - другое (OUTPUT_NAME main)
  add_executable(${trgt} ${src})

  # в CMAKE_BINARY_DIR будет тот каталог, который был передан по -B
  set_target_properties(${trgt} PROPERTIES
  OUTPUT_NAME main
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

  # команда для cl+link. флаг DEBUG = ON - т.е. пойдём по ветке if (DBG) с флагами для программ-задач
  configure_target(${trgt} ON)
 
endfunction()

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
# первый тест: проверка чистоты формата
# если есть clang+format - проверка строгая, не-падение - только при чистом формате
# иначе - warning и тесты
add_test(NAME format-check COMMAND ${CMAKE_COMMAND} -DMODE=check ${FORMAT_COMMON}) 
# тесты на знчения и коды
add_subdirectory(tests) 
