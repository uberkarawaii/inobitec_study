Описание: проект посвящён C / C++, процессу компиляции и другим сопутствующим моментам

### Структура
В корне
- README.md - мета-информация о проекте
- файлы (AGENTS.md, 00-temy.md, 01-teoriya.md, 02-praktika.md, 03-ocenka.md) - план обучения + память
- .clang-format - форматирование файлов на С / С++
- .editorconfig - правила чтения файлов для редакторов
- .gitignore - список типов файлов, которые не будут включаться в коммиты
- CMakeLists.txt - корневой файл CMake. включает настроки проекта, менеджит дефолтные флаги `CMAKE_..._FLAGS_DEBUG`,
  включает ф-цию для построения команд компиляции и линка в зависимости от платформы и собираемой цели (задача VS обвязка),
  и ф-цию добавления задач add_task, через которую они далее и добавляются 
- common/CMakeLists.txt - построение библиотек (стат./динам.). по необходимости на линуксе привязывается libm.so; 
  добавляются константы времени компиляции COMMON_... с модификаторами PUBLIC/PRIVATE;
  включается в CMakeLists.txt перед добавлением задач.
- tests/CMakeLists.txt - для тестовой части. собираются run_case и check, включается в CMakeLists.txt перед добавлением задач. включает в себя tests/cases.cmake
- tests/cases.cmake - тест-кейсы к задачам 1-4 через add_case(...)
- cmake/format.cmake - файл с cmake в скриптовом режиме, используется для целей `format` и `format-check`,
  подключается к корневому файлу CMakeLists.txt

### Прочее: 
- /hello каталог с начальной задачей. выводит hello, world
- /t1_dist_matrix_cpp задача 1 из параграфа 6 на с++
- /t1_dist_matrix_c задача 1 из параграфа 6, на языке Си
- /t2_passport_cpp задача 2 из параграфа 6 на c++
- /t2_passport_c задача 2 из параграфа 6, на языке Си
- /t3_bbox_cpp задача 3 из параграфа 6, на с++
- /t3_bbox_c задача 3 из параграфа 6, на Си
- /t4_filter_cpp задача 4 из параграфа 6, на с++
- /t4_filter_c задача 4 из параграфа 6, на Си
- /dialog_logs: выгрузка диалогов с DeepSeek через агента OpenCode
- /common - папка с файлами обшего назначения
- /notes - заметки по темам; внутренний dialog_logs - логи бесед с агентом по темам
- /tests - в корне run_case.cpp - выходной .exe сверяет итоговый код программы и подаваемый ему код на равенство. 
  и check.cpp, вызов через ` --flag file1 file2`; `--equal` - равенство файлов, `--contains` - содержание `file2` в `file1`.
- /tests/expect - ожидаемые выводы при тестах на нормальных данных
- /tests/input_data - входные данные для задач 


### Как собрать и прогнать тесты через CMake
#### сборка
**среда: открыть x64 Native Tools prompt или вызвать vcvars64.bat**
**windows-only мера для добавления папок с cmake/ninja в PATH. выполняется в начале каждой новой cmd:**
```
set "BT=D:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
set "PATH=%BT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%BT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
```

как сконфигурировать и сгенерировать служебные файлы: 
```
cmake -B build/debug -G [generator] -DCMAKE_BUILD_TYPE=[Debug/Release]
```

рабочий пример с Ninja+Debug: `cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug`

как собрать: 
```
cmake --build build/debug [--target t1_c/t1_cpp]
```
с --target и именем будет отдельная цель, а не all

как отформатировать:
```
cmake --build build/debug --target format
```

как почистить: 
```
cmake --build build/debug --target clean
```

#### тесты
тесты на коды и выходные значения:
```
ctest --test-dir build/debug
```

тест на формат файлов:
```
cmake --build build/debug --target format-check
```
