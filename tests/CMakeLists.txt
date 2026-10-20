# find_package с gtest именно тут т.к. корневому CMakeLists gtest не нужен
# find_package(GTest - найди и прочитай описание пакета GTest
# в описании сказано, как библиотека называется для CMake, где бинарники, headers
# CONFIG - читай именно описание от автора библиотеки (GTestConfig.cmake), не "догадываться"
# REQUIRED - если пошло некорректно - падаем. не идём тихо с ошибкой
find_package(GTest CONFIG REQUIRED)
# благодаря vcpkg GTestConfig.cmake найдётся. т.к. vcpkg.cmake Добавил build/<cfg>/vcpkg_installed/.../share
# в список каталогов по которому CMake ведёт поиск

# проверка версии gtest на соответствие tested-версии
check_component(GTest "${GTest_VERSION}" "${LIB_GTEST_TESTED}" "0")

# find_package(GTest...) - сама библиотека; include(GoogleTest) - хелперы для gtest 
# грузит GoogleTest.cmake - будут доступны gtest_add_tests и gtest_discover_tests (для целей, залинкованных с GTest)
include(GoogleTest)

# ф-ция для копирования библиотеч. dll руками, т.к. vcpkg applocal отключён во избежание конфликтов при -j
function(deploy_vcpkg_dlls target)
# после линковки цели, переместить к ней те dll от которых она зависит в т.ч. транзитивно,
# если их нет в папке у цели или если они изменились
# COMMAND_EXPAND_LISTS - развернуть dll из единого через ; в отдельные аргументы
# такие проблемы только на винде, под линуксом такого не возникнет
if(WIN32)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${target}>
            $<TARGET_FILE_DIR:${target}>
    COMMAND_EXPAND_LISTS)
endif()
endfunction()

# юнит-тесты
add_subdirectory(unit)

# е2е-тесты
add_subdirectory(e2e)
