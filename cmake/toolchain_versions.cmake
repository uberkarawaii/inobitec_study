# версии компонентов - минимальные (если есть) и на которых тестировали
set(TOOLCHAIN_MSVC_MIN "19.37")
set(TOOLCHAIN_MSVC_TESTED "19.51")
set(TOOLCHAIN_GCC_MIN "14")
set(TOOLCHAIN_GCC_TESTED "15.2")
set(TOOLCHAIN_CMAKE_MIN "3.22")
set(TOOLCHAIN_CMAKE_TESTED "4.3")
set(TOOLCHAIN_NINJA_TESTED "1.13")
set(TOOLCHAIN_GIT_MIN "2.0")
set(TOOLCHAIN_GIT_TESTED "2.54")
set(TOOLCHAIN_VCPKG_TOOL_TESTED "2026-07-27")
set(LIB_GTEST_TESTED "1.18.0")

# ф-ция cреза всего после major_minor: "19.51.36247.0" -> "19.51"; "4.3.1-msvc1" -> "4.3"; "1.13.2" -> "1.13"
function(to_major_minor out in)
  string(REGEX MATCH "^[0-9]+\\.[0-9]+" matched "${in}")
  set(${out} "${matched}" PARENT_SCOPE)
endfunction()

# версия компонента проверяется и происходит падение/предупреждение/сообщение в зависимости от версии
function(check_component name actual tested min)
  if(NOT min STREQUAL "0" AND actual VERSION_LESS min)
    message(FATAL_ERROR "${name} ${actual} is older than minimal ${min}. upgrade the version")

  elseif(NOT actual VERSION_EQUAL tested)
    message(WARNING "${name} ${actual} is different from tested ${tested}")

  # когда actual == tested
  else()
    message(STATUS "${name} ${actual} is same with tested ${tested}")
  endif()
endfunction()