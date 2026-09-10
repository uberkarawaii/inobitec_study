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

  При тестировании будут создаваться папки со следующей иерархией:
    ```
    build/
    ├── debug/
    │   └── test/
    │   │   └── t1_cpp_norm.ok
    │   │   ├── t1_cpp_abc.ok
    │   │   └── ...
    │   ├── t1_dist_matrix_cpp/
    │   ├── ...
    │   ├── t4_filter_c/
    │   ├── common/
    │   │   └── geometry_cpp.obj
    │   │   ├── ...
    │   │   └── string_utils_c.pdb
    │   └── tools/
    │       ├── run_case.obj
    │       ├── run_case.pdb
    │       └── run_case.exe
    │
    └── release/
        └── test/
        ├── tools/
        ├── common/
        ├── t1_dist_matrix_cpp/
        ├── ...
        └── t4_filter_c/
    ```

### Как собрать и прогнать тесты через CMake
#### сборка
**Временная мера от рассогласования /showIncludes на MSVC: `chcp 1251` перед любыми cmake-операциями**

как сконфигурировать и сгенерировать служебные файлы: 
```
cmake -B build/cmake_debug -G [generator] -DCMAKE_BUILD_TYPE=[Debug/Release]
```

рабочий пример с Ninja+Debug: `cmake -B build/cmake_debug -G Ninja -DCMAKE_BUILD_TYPE=Debug`

как собрать: 
```
cmake --build build/cmake_debug [--target t1_c/t1_cpp]
```
с --target и именем будет отдельная цель, а не all

как отформатировать:
```
cmake --build build/cmake_debug --target format
```

как почистить: 
```
cmake --build build/cmake_debug --target clean
```

#### тесты
тесты на коды и выходные значения:
```
ctest --test-dir build/cmake_debug
```

тест на формат файлов:
```
cmake --build build/cmake_debug --target format-check
```
