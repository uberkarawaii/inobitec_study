# библиотека с парсером для разбора cobertura.xml
add_library(coverage_union_lib STATIC coverage_union.cpp)
# добавление каталога tools/coverage_union в пути поисков заголовков
# PUBLIC - для всех, кто линкуется с coverage_union_lib
# нужно для юнит-теста, который будет линковаться с данной библиотекой и включать coverage_union.hpp
target_include_directories(coverage_union_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# ON чтобы был ASan, иначе потом на юнит-тестах err LNK2038, т.к. юнит-тест с асан, а либа - без 
configure_target(coverage_union_lib ON)

# сборка программы command line interface, для coverage-сборки
# применяет библиотечные методы coverage_union_lib для парса cobertura.xml, и манифеста с файлами покрытия, etc. 
# в итоге для получения человекочитаемой сводки по покрытию
# EXCLUDE_FROM_ALL: by-default coverage_union не собирается, только при явной команде / при зависимости др. target-а
add_executable(coverage_union EXCLUDE_FROM_ALL main.cpp)
target_link_libraries(coverage_union PRIVATE coverage_union_lib)
# включает common/exit_codes.hpp, потому именно для coverage_union нужно включить папку с этим hpp для поиска путей
target_include_directories(coverage_union PRIVATE "${CMAKE_SOURCE_DIR}")
# ASan тут по факту не нужен, но это защита от LNK2038, если кто-то на дебаге решит собрать coverage_union
configure_target(coverage_union ON)

