Описание: проект посвящён C / C++, процессу компиляции и другим сопутствующим моментам

### Структура
В корне
- README.md - мета-информация о проекте
- файлы (AGENTS.md, 00-temy.md, 01-teoriya.md, 02-praktika.md, 03-ocenka.md) - план обучения + память
- .clang-format - форматирование файлов на С / С++
- .editorconfig - правила чтения файлов для редакторов
- .gitignore - список типов файлов, которые не будут включаться в коммиты
- t3_bbox_ref.ps1 - сценарий на power shell для 3 задачи
- t1_build_and_test.bat ... и т.д. - batch файлы для тестов и сборки к каждой задаче

### Прочее: 
- /hello каталог с начальной задачей. выводит hello, world
- /t1_dist_matrix задача 1 из параграфа 6 на с++
- /t1_dist_matrix_c задача 1 из параграфа 6, на языке Си
- /t2_passport_cpp задача 2 из параграфа 6 на c++
- /t2_passport_c задача 2 из параграфа 6, на языке Си
- /t3_bbox_cpp задача 3 из параграфа 6, на с++
- /t3_bbox_c задача 3 из параграфа 6, на Си
- /t4_filter_cpp задача 4 из параграфа 6, на с++
- /dialog_logs: выгрузка диалогов с DeepSeek через агента OpenCode
- /common - папка с файлами обшего назначения
- /notes - заметки по темам; внутренний dialog_logs - логи бесед с агентом по темам

### Как собрать

**hello/hello.cpp**
```x64 Native Tools
cd hello
cl /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address hello.cpp
```

**/t1_dist_matrix/main.cpp и /t1_dist_matrix_c/main.c**
```
cl /c /Fo:t1_dist_matrix\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\main.cpp
link /DEBUG /OUT:t1_dist_matrix\main.exe t1_dist_matrix\main.obj

cl /c /Fo:t1_dist_matrix_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\main.c
link /DEBUG /OUT:t1_dist_matrix_c\main.exe t1_dist_matrix_c\main.obj
```

**/t2_passport_cpp/main.cpp и /t2_passport_c/main.c**
```
cl -c /Fo:t2_passport_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t2_passport_cpp\main.cpp
link /DEBUG /OUT:t2_passport_cpp\main.exe t2_passport_cpp\main.obj

cl -c /Fo:t2_passport_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t2_passport_c\main.c
link /DEBUG /OUT:t2_passport_c\main.exe t2_passport_c\main.obj
```

**t3_bbox_cpp\main.cpp и t3_bbox_c\main.с**
```
cl -c /Fo:t3_bbox_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t3_bbox_cpp\main.cpp
link /DEBUG /OUT:t3_bbox_cpp\main.exe t3_bbox_cpp\main.obj 

cl -c /Fo:t3_bbox_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t3_bbox_c\main.c
link /DEBUG /OUT:t3_bbox_c\main.exe t3_bbox_c\main.obj 
```

**t4_filter_cpp\main.cpp**
```
cl -c /Fo:t4_filter_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t4_filter_cpp\main.cpp
link /DEBUG /OUT:t4_filter_cpp\main.exe t4_filter_cpp\main.obj

```

### Как прогнать тесты
#### Через .bat файлы
**t1_dist_matrix\main.cpp и t1_dist_matrix_c\main.c**
```bash
t1_build_and_test.bat
```

**t2_passport_cpp\main.cpp и t1_passport_c\main.c**
```bash
t2_build_and_test.bat
```

**t3_bbox_cpp\main.cpp и t3_bbox_c\main.c**
```bash
t3_build_and_test.bat
```

**t4_filter_cpp\main.cpp**
```bash
t4_build_and_test.bat
```

При успешном проходе тестов в будет выводиться:
...
ALL CPP TESTS PASSED
...
ALL C TESTS PASSED

#### Руками (на примере первой задачи на с++)
тест на корректных данных
```bash
echo 5 | t1_dist_matrix\main.exe > t1_dist_matrix\main_out.txt
echo 5 | t1_dist_matrix\ref.exe > t1_dist_matrix\ref_out.txt
fc t1_dist_matrix\main_out.txt t1_dist_matrix\ref_out.txt
```
ожидаемый вывод: сравнение файлов... FC: различия не найдены

тест с некорректными данными 
```bash
echo abc | t1_dist_matrix\main.exe
echo %ERRORLEVEL%
```
ожидаемый вывод: 65

тест с пустой строкой
```bash
echo; | t1_dist_matrix\main.exe
echo %ERRORLEVEL%
```
ожидаемый вывод: 66

тест с данными за диапазоном
```bash
echo 2 | t1_dist_matrix\main.exe
echo %ERRORLEVEL%
echo 21 | t1_dist_matrix\main.exe
echo %ERRORLEVEL%
```
ожидамые выводы: 64
