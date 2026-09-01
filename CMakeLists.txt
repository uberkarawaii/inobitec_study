# минимальная необходимая версия cmake + свежие политики (конкретные точки поведения) до 3.22 
# в старых CMake-ах выполнение остановится здесь, с требованием версии 3.22
# политика - как себя ведёт что-либо в CMake. если не указать версию, то default=OLD и будет warning, 
# т.к. старые политики - для совместимости, но не обещают корретности
cmake_minimum_required(VERSION 3.22)
# имя проекта сохранится в CMAKE_PROJECT_NAME
# определяются яызки проекта: с с++; определится компилятор; 
# появятся перменные (типа CMAKE_C_FLAGS_DEBUG) заполненные дефолтными значениями
project(inobitec_stud LANGUAGES C CXX)

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
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type: Debug / Release" FORCE)
endif()
# переменная CMAKE_BUILD_TYPE будет сохранена в кэше с с пояснит. строкой
# кэш: для самого CMake (хранить инф. м-у выполнениями) и чтобы человек каждый раз не вводил эти вещи
# FORCE вызовет перезапись параметра

# === НЕ /RTC1, А ASan ===
# CMAke дефолтно положит /RTC1 в debug-ветку. /RTC1 ловит ошибки в runtime, для этого лезет в стек и размечает его
# ASan - та же ф-ция. если оба будут менять стек, они помешают друг другу -> несовместимы. /RTC1 убирается 
# /incremental также несовместим с asan (именно по документации. так работало и когда оба вместе)
if(MSVC)
  string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  string(REPLACE "/INCREMENTAL" "" CMAKE_EXE_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
endif()

# === ПОСТРОИТЬ КОМАНДЫ КОМПИЛЯЦИИ И ЛИНКА ===
# тут /Zi, /EHsc, /DEBUG  будут дублироваться, но прописываются для ясности
function(configure_target trgt DBG)
if(MSVC)
  # target_compile_options - общие флаги компилятора 
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

# === ДОБАВЛЕНИЕ ЗАДАЧИ === 
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

  # команда для cl+link. флаг DEBUG = ON - т.е. пойдём по ветке if (DBG) с флагами для программ-задач
  configure_target(${trgt} ON)
 
endfunction()

# переход в /common к CMakeLists.txt, там сделаются common_...lib, с которыми main надо слинковаться  
add_subdirectory(common)

# arg1 - папка бинарей и прочих продуктов (build/cfg/...)
# arg2 - логическое имя цели
# arg3 - исходник, чтобы сделать файл цели
# arg4 - библиотека с которой линковаться
add_task(t1_dist_matrix_cpp t1_cpp t1_dist_matrix_cpp/main.cpp common_cpp_static)
add_task(t1_dist_matrix_c t1_c t1_dist_matrix_c/main.c common_c_static)