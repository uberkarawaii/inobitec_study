# === T1 СБОРКА СТАТИЧ. БИБЛИОТЕКИ ===
# <name> - логическое имя цели
# [<type>] - static
# [EXCLUDE_FROM_ALL] - не установлено, деолтно false, то есть target включится в all
# <sources> = geometry.c / geometry.cpp; header-ы cmake должен подтянуть сам 
add_library(common_cpp_static STATIC geometry.cpp)
add_library(common_c_static STATIC geometry.c)

# target_compile_definitions - именно для директив препроцессора
# здесь - для добавления флага COMMON_STATIC
# PUBLIC сделает так, что COMMON_STATIC будет и при компиляции либы, и при компиляции тех, кто к ней линкуется
# t1../main.. линкуется к ней - транзитивно получит флаг COMMON_STATIC  
target_compile_definitions(common_cpp_static PUBLIC COMMON_STATIC)
target_compile_definitions(common_c_static PUBLIC COMMON_STATIC)

# из верхнего CMakeLists.txt - построит команду для сборки
# т.к. common_..._static - static library -> из configure_target для неё произойдёт только cl, не link
# и т.к. static, то ещё будет lib (CMake видит тип таргета)  
configure_target(common_cpp_static ON)
configure_target(common_c_static ON)