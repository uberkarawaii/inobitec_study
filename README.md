Описание: проект посвящён C / C++, процессу компиляции и другим сопутствующим моментам

Структура
В корне
- README.md - мета-информация о проекте
- файлы (AGENTS.md, 00-temy.md, 01-teoriya.md, 02-praktika.md, 03-ocenka.md) - план обучения + память
- .clang-format - форматирование файлов на С / С++
- .editorconfig - правила чтения файлов для редакторов
- .gitignore - список типов файлов, которые не будут включаться в коммиты

Прочее: 
- /hello каталог с начальной задачей. выводит hello, world
- /t1_dist_matrix задача 1 из параграфа 6
- /dialog_logs: выгрузка диалогов с DeepSeek через агента OpenCode
- /common - папка с файлами обшего назначения

Как собрать: 
для hello/hello.cpp
    cd hello
    cl /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address hello.cpp

для /t1_dist_matrix/main.cpp:
    находимся в корне
    chcp 1251
    cl /c /Fo:t1_dist_matrix\objname.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\main.cpp
    link /DEBUG /OUT:t1_dist_matrix\main.exe t1_dist_matrix\main.obj
    t1_dist_matrix\main.exe


Как прогнать тесты:
1. для t1_dist_matrix\main.exe
  одной командой
  build_and_test.bat

  руками
  # тест на корректных данных
  echo 5 | t1_dist_matrix\main.exe > main_out.txt
  echo 5 | t1_dist_matrix\ref.exe > ref_out.txt
  fc main_out.txt ref_out.txt
  # ожидаемый вывод: сравнение файлов... FC: различия не найдены

  # тест с некорректными данными 
  echo abc | t1_dist_matrix\main.exe
  echo %ERRORLEVEL%
  # ожидаемый вывод: 65

  # тест с пустой строкой
  echo; | t1_dist_matrix\main.exe
  echo %ERRORLEVEL%
  # ожидаемый вывод: 66

  # тест с данными за диапазоном
  echo 2 | t1_dist_matrix\main.exe
  echo %ERRORLEVEL%
  echo 21 | t1_dist_matrix\main.exe
  echo %ERRORLEVEL%
  # ожидамые выводы: 64

