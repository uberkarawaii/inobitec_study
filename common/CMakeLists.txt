# === T1 СБОРКА СТАТИЧ. БИБЛИОТЕКИ ===
# <name> - логическое имя цели
# [<type>] - static
# [EXCLUDE_FROM_ALL] - не установлено, деолтно false, то есть target включится в all
# <sources> = geometry.c / geometry.cpp; header-ы cmake должен подтянуть сам 
add_library(geometry_cpp_static STATIC geometry.cpp)
add_library(geometry_c_static STATIC geometry.c)

# target_compile_definitions - именно для директив препроцессора
# здесь - для добавления флага COMMON_STATIC
# PUBLIC сделает так, что COMMON_STATIC будет и при компиляции либы, и при компиляции тех, кто к ней линкуется
# t1../main.. линкуется к ней - транзитивно получит флаг COMMON_STATIC  
target_compile_definitions(geometry_cpp_static PUBLIC COMMON_STATIC)
target_compile_definitions(geometry_c_static PUBLIC COMMON_STATIC)

# libm.so для линукса. с ней линкуется библиотека и транзитивно - t1_c
if(UNIX)
  target_link_libraries(geometry_c_static PUBLIC m)
endif()

# из верхнего CMakeLists.txt - построит команду для сборки
# т.к. common_..._static - static library -> из configure_target для неё произойдёт только cl, не link
# и т.к. static, то ещё будет сделана сборка в lib-у (CMake видит тип таргета)  
configure_target(geometry_cpp_static ON)
configure_target(geometry_c_static ON)


# === T2 СБОРКА СТАТИЧ. БИБЛИОТЕКИ
add_library(string_utils_cpp_static STATIC string_utils.cpp)
add_library(string_utils_c_static STATIC string_utils.c)

target_compile_definitions(string_utils_cpp_static PUBLIC COMMON_STATIC) 
target_compile_definitions(string_utils_c_static PUBLIC COMMON_STATIC)

configure_target(string_utils_cpp_static ON)
configure_target(string_utils_c_static ON)


# === T3-T4 СБОРКА ДИНАМИЧЕСКОЙ БИБЛИОТЕКИ ===
add_library(common_cpp_shared SHARED geometry.cpp string_utils.cpp)
add_library(common_c_shared SHARED geometry.c string_utils.c)

# тут PRIVATE, т.к. main t3-t4 не должны иметь этот флаг, чтобы у них внутри был dllimport
target_compile_definitions(common_cpp_shared PRIVATE COMMON_EXPORTS)
target_compile_definitions(common_c_shared PRIVATE COMMON_EXPORTS)

if(UNIX)
  target_link_libraries(common_c_shared PUBLIC m)
endif()

configure_target(common_cpp_shared ON)
configure_target(common_c_shared ON)
