# ---------- ПАПКИ В /build ---------- 
# папка с учётом режима сборки. если не было передано иного, будет /debug
CONFIG ?= debug
# директория для бинарных файлов. внутри неё будут и debug, и release
BINDIR := build\$(CONFIG)
# для бинарников тестовых программ. по умолчанию будет build/debug/test
TESTDIR := $(BINDIR)\test


# ---------- ПЛАТФОРМОЗАВИСИМОЕ: ИНСТРУМЕНТЫ ДЛЯ СБОРКИ И ФЛАГИ, ЗАВИСЯЩИЕ ОТ РЕЖИМА (для cl, link) ---------- 
ifeq ($(OS), Windows_NT)
  # версия для MSVC
  CC := cl
  CXX := cl
  LINK := link
  CSTD := /std:c17
  CXXSTD := /std:c++latest
  WARN := /W4 /permissive-
  GREP := findstr
  NULLDEV := nul
  MKDIR := mkdir
  TOUCH := type nul > 
  # /s - всё содержимое; /q - quietly
  RMDIR := rmdir /s /q
  ifeq ($(CONFIG), debug)
    CONFIG_FLAGS := /Od /Zi /MDd /fsanitize=address
    LINK_FLAGS := /DEBUG 
  else
    # /O2 - с оптимизацией кода
    # /Zi - также будет сбор инф. для дебага в pdb. но уже по оптимизированному коду
    # /DNDEBUG - define NDEBUG - макрос чтобы убрать любые assert-ы 
    # т.к. они могут остановить выполнение программы при неверном значении лог. выражения
    # /MD - многопотом. динами. crt - без d (debug)
    CONFIG_FLAGS := /O2 /Zi /DNDEBUG /MD
    # /DEBUG - для сбора инф. в общий .pdb из .obj - /Zi на cl уже сложило инф. по .obj
    # /OPT:REF - строится граф ссылок и линкер убирает ф-ции и данные, на которые нет ссылок
    # /OPT:ICF - identical comdat folding - куски кода, которые идентичны после cl, скледиваются в один
    # если /OPT:ICF то /OPT:REF подтянется автоматически. 
    # /OPT:... уменьшают итоговый .exe, но бин. код получается не 1:1 с изначальным кодом - неудобство при отладке  
    LINK_FLAGS := /DEBUG /OPT:REF /OPT:ICF  
  endif
  # итоговый набор флагов cl
  CXXFLAGS := $(CXXSTD) $(WARN) /EHsc $(CONFIG_FLAGS)  
  CFLAGS := $(CSTD) $(WARN) $(CONFIG_FLAGS)
else
  # флаги под линукс - пока нереализовано
  # TODO : написать флаги под линукс
endif


# ---------- ПЕРЕМЕННЫЕ ДЛЯ ЦЕЛЕЙ (ЭТО ОБЪЕКТНИКИ, ИСПОЛНЯЕМЫЕ ФАЙЛЫ, ТЕСТОВЫЕ ФАЙЛЫ) ---------- 
# исполняемые файлы прикладных программ
RUN_CASE := $(BINDIR)\tools\run_case.exe
FIND_SUBSTR := $(BINDIR)\tools\check_contains.exe

# объектники и исполняемые (цели и под-цели)
T1_CPP_OBJ := $(BINDIR)\t1_dist_matrix_cpp\main.obj $(BINDIR)\common\geometry_cpp.obj
T1_C_OBJ := $(BINDIR)\t1_dist_matrix_c\main.obj $(BINDIR)\common\geometry_c.obj
T1_CPP_EXE := $(BINDIR)\t1_dist_matrix_cpp\main.exe
T1_C_EXE := $(BINDIR)\t1_dist_matrix_c\main.exe

T2_CPP_OBJ := $(BINDIR)\t2_passport_cpp\main.obj $(BINDIR)\common\string_utils_cpp.obj
T2_C_OBJ := $(BINDIR)\t2_passport_c\main.obj $(BINDIR)\common\string_utils_c.obj
T2_CPP_EXE := $(BINDIR)\t2_passport_cpp\main.exe
T2_C_EXE := $(BINDIR)\t2_passport_c\main.exe

T3_CPP_EXE := $(BINDIR)\t3_bbox_cpp\main.exe
T3_C_EXE := $(BINDIR)\t3_bbox_c\main.exe

# target-specific переменные - для определения линковки на этапе компиляции - статическая / динамическая
$(T1_CPP_EXE) $(T1_C_EXE) $(T2_CPP_EXE) $(T2_C_EXE): COMMON_FLAG := /DCOMMON_STATIC
# для t3 - t4 этот флаг пустой - так функции будут помечаться, как импортируемые из dll
# и инструкция set PATH чтобы .exe находил .dll
$(T3_CPP_EXE) $(T3_C_EXE): COMMON_FLAG :=

# цели (тестовые)
# тут нельзя через wildcard: если так сделать, то при построении дерева make будет сверяться с файловой системой
# и не увидит там ни одного из .ok файлов - прогона тестов ещё не было. тогда ни для одного .ok файла рецепт для неё не выполнится
# поэтом с абсолютными путями
T1_CPP_TESTS := $(TESTDIR)\t1_cpp_abc.ok \
	    $(TESTDIR)\t1_cpp_float.ok \
	    $(TESTDIR)\t1_cpp_low.ok \
	    $(TESTDIR)\t1_cpp_high.ok \
	    $(TESTDIR)\t1_cpp_nul.ok \
            $(TESTDIR)\t1_cpp_norm.ok

T1_C_TESTS := $(TESTDIR)\t1_c_abc.ok \
	    $(TESTDIR)\t1_c_float.ok \
	    $(TESTDIR)\t1_c_low.ok \
	    $(TESTDIR)\t1_c_high.ok \
	    $(TESTDIR)\t1_c_nul.ok \
            $(TESTDIR)\t1_c_norm.ok

T2_CPP_TESTS := $(TESTDIR)\t2_cpp_norm.ok \
                $(TESTDIR)\t2_cpp_eof_name.ok \
                $(TESTDIR)\t2_cpp_empty_name.ok \
                $(TESTDIR)\t2_cpp_eof_vertexes.ok \
                $(TESTDIR)\t2_cpp_empty_vertexes.ok \
                $(TESTDIR)\t2_cpp_negative_vertexes.ok \
                $(TESTDIR)\t2_cpp_fractional_vertexes.ok \
                $(TESTDIR)\t2_cpp_nan_vertexes.ok

T2_C_TESTS := $(TESTDIR)\t2_c_norm.ok \
                $(TESTDIR)\t2_c_eof_name.ok \
                $(TESTDIR)\t2_c_empty_name.ok \
                $(TESTDIR)\t2_c_eof_vertexes.ok \
                $(TESTDIR)\t2_c_empty_vertexes.ok \
                $(TESTDIR)\t2_c_negative_vertexes.ok \
                $(TESTDIR)\t2_c_fractional_vertexes.ok \
                $(TESTDIR)\t2_c_nan_vertexes.ok

T3_CPP_TESTS := $(TESTDIR)\t3_cpp_empty_case.ok \
                $(TESTDIR)\t3_cpp_not_digit_case.ok \
                $(TESTDIR)\t3_cpp_too_few_args_case.ok \
                $(TESTDIR)\t3_cpp_test1.ok \
                $(TESTDIR)\t3_cpp_test2.ok \
                $(TESTDIR)\t3_cpp_test3.ok 

T3_C_TESTS := $(TESTDIR)\t3_c_empty_case.ok \
                $(TESTDIR)\t3_c_not_digit_case.ok \
                $(TESTDIR)\t3_c_too_few_args_case.ok \
                $(TESTDIR)\t3_c_test1.ok \
                $(TESTDIR)\t3_c_test2.ok \
                $(TESTDIR)\t3_c_test3.ok 

# exit-коды
USAGE := 64
DATA := 65
NO_INPUT := 66
IO_FAIL := 74

# отдельно вынесенные имена каталогов - визуально сократить order-only (абс. пути), т.к. % паттерн не м.б. в связке с order-only
CPP_DIRS := $(BINDIR)\t1_dist_matrix_cpp $(BINDIR)\t2_passport_cpp $(BINDIR)\t3_bbox_cpp
C_DIRS := $(BINDIR)\t1_dist_matrix_c $(BINDIR)\t2_passport_c $(BINDIR)\t3_bbox_c


# ---------- ПОДКАТАЛОГИ (ORDER-ONLY, ВАЖНО ТОЛЬКО ИХ НАЛИЧИЕ) ---------- 

# /tools существует только для run_case.exe
# в итоге эта цепочка будет разложена на 5 правил вида "$(BINDIR)/t1_dist_matrix_cpp: $(MKDIR) $@"

# Make зайдёт сюда только если где-то будет, например, $(BINDIR)/common, но папки /common ещё нет. 
# тогда make придёт сюда и сделает инструкцию MKDIR
$(CPP_DIRS) $(C_DIRS) $(BINDIR)\common $(BINDIR)\tools $(TESTDIR):
	$(MKDIR) $@


# ---------- ЗАВИСИМОСТИ - (ПЕРЕ)СОБРАТЬ ЦЕЛЬ ЕСЛИ: ПРЕРЕКВИЗИТ БЫЛ ИЗМЕНЁН ПОЗЖЕ ЦЕЛИ ИЛИ ЦЕЛЬ ЕЩЁ НЕ СУЩЕСТВУЕТ ---------- 
# цель: пререкв.1 пререкв.2 ...
# $@ - цель
# $< - первый пререквизит
# $^ - все пререквизиты	

# СБОРКИ
# правила для всеx .obj в подкаталогах /bin/..., 
# для которых есть зеркальные .cpp/.c (лежат в такой же папке, но она в корне проекта)
# флаг COMMON_FLAG - target-specific переменная. влияет на вид последущей линковки - статич. / динамич. 
# и order-only - чтобы папка для объектника существовала, если сборок ещё не было
$(BINDIR)\\%.obj: %.cpp | $(CPP_DIRS)
	$(CXX) /c $(COMMON_FLAG) /Fo:$@ $(CXXFLAGS) $<   

# аналогичное для Си
$(BINDIR)\\%.obj: %.c | $(C_DIRS) 
	$(CC) /c $(COMMON_FLAG) /Fo:$@ $(CFLAGS) $<

# STATIC (t1-t2) правила для geometry + string_utils, т.к. их имена объектников не совпадают с именами .src
$(BINDIR)\common\geometry_cpp.obj: common\geometry.cpp common\geometry.hpp | $(BINDIR)\common
	$(CXX) /c /DCOMMON_STATIC /Fo:$@ $(CXXFLAGS) $< 

$(BINDIR)\common\geometry_c.obj: common\geometry.c common\geometry.h | $(BINDIR)\common
	$(CC) /c /DCOMMON_STATIC /Fo:$@ $(CFLAGS) $<

$(BINDIR)\common\string_utils_cpp.obj: common\string_utils.cpp common\string_utils.hpp | $(BINDIR)\common
	$(CXX) /c /DCOMMON_STATIC /Fo:$@ $(CXXFLAGS) $<

$(BINDIR)\common\string_utils_c.obj: common\string_utils.c common\string_utils.h | $(BINDIR)\common
	$(CC) /c /DCOMMON_STATIC /Fo:$@ $(CFLAGS) $<

# DYNAMIC (t3-t4)
$(BINDIR)\common\geometry_cpp_dll.obj: common\geometry.cpp common\geometry.hpp | $(BINDIR)\common
	$(CXX) /c /DCOMMON_EXPORTS /Fo:$@ $(CXXFLAGS) $<

$(BINDIR)\common\geometry_c_dll.obj: common\geometry.c common\geometry.h | $(BINDIR)\common
	$(CC) /c /DCOMMON_EXPORTS /Fo:$@ $(CFLAGS) $<

$(BINDIR)\common\string_utils_cpp_dll.obj: common\string_utils.cpp common\string_utils.hpp | $(BINDIR)\common
	$(CXX) /c /DCOMMON_EXPORTS /Fo:$@ $(CXXFLAGS) $<

$(BINDIR)\common\string_utils_c_dll.obj: common\string_utils.c common\string_utils.h | $(BINDIR)\common
	$(CC) /c /DCOMMON_EXPORTS /Fo:$@ $(CFLAGS) $<

# ЛИНКОВКИ
# линковка для 1 задачи и модуля geometry. линковка статическая, тк тут один geometry без string_utils
$(BINDIR)\t1_dist_matrix_cpp\main.exe: $(T1_CPP_OBJ) | $(BINDIR)\t1_dist_matrix_cpp
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^ 

$(BINDIR)\t1_dist_matrix_c\main.exe: $(T1_C_OBJ) | $(BINDIR)\t1_dist_matrix_c 
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^

# линковка для 2 задачи. также статич., с одним модулем string_utils
$(BINDIR)\t2_passport_cpp\main.exe: $(T2_CPP_OBJ) | $(BINDIR)\t2_passport_cpp
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^	

$(BINDIR)\t2_passport_c\main.exe: $(T2_C_OBJ) | $(BINDIR)\t2_passport_c
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^

# линковка common_cpp.dll И common_c.dll: .lib также в целях, чтобы при линковке с main.exe .lib всегда существовал
# grouped target чтобы не было раскрытия на два правила
# жёсткий путь до dll а не $@, т.к. $@ - цель, которая триггернула правило. а не надо, чтобы был случай линка в .lib
CPP_DLL_OBJ := $(BINDIR)\common\geometry_cpp_dll.obj $(BINDIR)\common\string_utils_cpp_dll.obj
$(BINDIR)\common\common_cpp.dll $(BINDIR)\common\common_cpp.lib &: $(CPP_DLL_OBJ) | $(BINDIR)\common
	$(LINK) $(LINK_FLAGS) /DLL /OUT:$(BINDIR)\common\common_cpp.dll $^

C_DLL_OBJ := $(BINDIR)\common\geometry_c_dll.obj $(BINDIR)\common\string_utils_c_dll.obj
$(BINDIR)\common\common_c.dll $(BINDIR)\common\common_c.lib &: $(C_DLL_OBJ) | $(BINDIR)\common
	$(LINK) $(LINK_FLAGS) /DLL /OUT:$(BINDIR)\common\common_c.dll $^

# линковка t3 - t4 с dll
# common_cpp.lib имеет свой рецепт для перестройки на случай, если его удалят. поэтому он - полноценный элемент дерева файлов
$(BINDIR)\t3_bbox_cpp\main.exe: $(BINDIR)\t3_bbox_cpp\main.obj $(BINDIR)\common\common_cpp.lib | $(BINDIR)\t3_bbox_cpp
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^

$(BINDIR)\t3_bbox_c\main.exe: $(BINDIR)\t3_bbox_c\main.obj $(BINDIR)\common\common_c.lib | $(BINDIR)\t3_bbox_c
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^ 

# ----- СБОРКА ПРИКЛАДНЫХ ПРОГРАММ (ДЛЯ ТЕСТИРОВАНИЯ) -----
# RUN_CASE - правило + рецепт
# небольшой .exe, который будет проверять, что код выхода программы совпадает с заданным кодом
# Fo - file output (obj) Fe - file executable (exe)
$(BINDIR)\tools\run_case.exe: tests\run_case.cpp | $(BINDIR)\tools
	$(CXX) /Fo:$(BINDIR)\tools\run_case.obj /Fe:$@ $(CXXFLAGS) $<

# FIND_SUBSTR - правило + рецепт
# берёт file1 file2 и ищёт подстроку file2 в file1; сравнивает побайтово
$(BINDIR)\tools\check_contains.exe: tests\check_contains.cpp | $(BINDIR)\tools
	$(CXX) /Fo:$(BINDIR)\tools\check_contains.obj /Fe:$@ $(CXXFLAGS) $<


# ---------- ТЕСТЫ ---------- 
# (при успешном выполнеии появляются .ok маркеры с соотв. именами в build/.../test)
# TASK 1
# cpp tests
# exit-code тесты
# после && будет выполнение, только если до && код выхода == 0. работает и в shell, и в cmd 
$(TESTDIR)\t1_cpp_abc.ok: $(T1_CPP_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo abc | $(T1_CPP_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && $(TOUCH) $@

$(TESTDIR)\t1_cpp_float.ok: $(T1_CPP_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 2.3 | $(T1_CPP_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && $(TOUCH) $@

$(TESTDIR)\t1_cpp_low.ok: $(T1_CPP_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 2 | $(T1_CPP_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(USAGE) && $(TOUCH) $@

$(TESTDIR)\t1_cpp_high.ok: $(T1_CPP_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 21 | $(T1_CPP_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(USAGE) && $(TOUCH) $@

$(TESTDIR)\t1_cpp_nul.ok: $(T1_CPP_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "type $(NULLDEV) | $(T1_CPP_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(NO_INPUT) && $(TOUCH) $@

# тест с корретными данными. проверка на норм. выходные данные
$(TESTDIR)\t1_cpp_norm.ok: $(T1_CPP_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 3 | $(T1_CPP_EXE) > $(BINDIR)\t1_dist_matrix_cpp\main_out.txt 2> $(NULLDEV)" 0 && \
        $(GREP) /C:"   0.000   1.732   1.732" $(BINDIR)\t1_dist_matrix_cpp\main_out.txt && $(TOUCH) $@

# C tests
# exit-code тесты
$(TESTDIR)\t1_c_abc.ok: $(T1_C_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo abc | $(T1_C_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && $(TOUCH) $@

$(TESTDIR)\t1_c_float.ok: $(T1_C_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 2.3 | $(T1_C_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && $(TOUCH) $@

$(TESTDIR)\t1_c_low.ok: $(T1_C_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 2 | $(T1_C_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(USAGE) && $(TOUCH) $@

$(TESTDIR)\t1_c_high.ok: $(T1_C_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 21 | $(T1_C_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(USAGE) && $(TOUCH) $@

$(TESTDIR)\t1_c_nul.ok: $(T1_C_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "type $(NULLDEV) | $(T1_C_EXE) > $(NULLDEV) 2> $(NULLDEV)" $(NO_INPUT) && $(TOUCH) $@

# нормальные входные данные. проверка выходных данных
$(TESTDIR)\t1_c_norm.ok: $(T1_C_EXE) $(RUN_CASE) | $(TESTDIR)
	$(RUN_CASE) "echo 3 | $(T1_C_EXE) > $(BINDIR)\t1_dist_matrix_c\main_out.txt 2> $(NULLDEV)" 0 && \
        $(GREP) /C:"   0.000   1.732   1.732" $(BINDIR)\t1_dist_matrix_c\main_out.txt && $(TOUCH) $@

# TASK 2 cpp tests
$(TESTDIR)\t2_cpp_norm.ok: $(T2_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t2_input tests\expect\t2_norm | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_input > $(BINDIR)\t2_passport_cpp\t2_norm 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t2_passport_cpp\t2_norm tests\expect\t2_norm && $(TOUCH) $@

$(TESTDIR)\t2_cpp_eof_name.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_eof_name | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_eof_name > $(NULLDEV) 2> $(NULLDEV)" $(NO_INPUT) && \
        $(TOUCH) $@ 

$(TESTDIR)\t2_cpp_empty_name.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_empty_name | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_empty_name > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@ 

$(TESTDIR)\t2_cpp_eof_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_eof_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_eof_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(NO_INPUT) && \
        $(TOUCH) $@

$(TESTDIR)\t2_cpp_empty_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_empty_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_empty_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)\t2_cpp_negative_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_negative_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_negative_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(USAGE) && \
        $(TOUCH) $@ 

$(TESTDIR)\t2_cpp_fractional_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_fractional_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_fractional_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)\t2_cpp_nan_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests\input_data\t2_nan_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_CPP_EXE) < tests\input_data\t2_nan_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

# TASK 2 c tests
$(TESTDIR)\t2_c_norm.ok: $(T2_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t2_input tests\expect\t2_norm | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_input > $(BINDIR)\t2_passport_c\t2_norm 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t2_passport_c\t2_norm tests\expect\t2_norm && $(TOUCH) $@

$(TESTDIR)\t2_c_eof_name.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_eof_name | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_eof_name > $(NULLDEV) 2> $(NULLDEV)" $(NO_INPUT) && \
        $(TOUCH) $@ 

$(TESTDIR)\t2_c_empty_name.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_empty_name | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_empty_name > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)\t2_c_eof_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_eof_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_eof_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(NO_INPUT) && \
        $(TOUCH) $@

$(TESTDIR)\t2_c_empty_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_empty_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_empty_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)\t2_c_negative_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_negative_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_negative_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(USAGE) && \
        $(TOUCH) $@

$(TESTDIR)\t2_c_fractional_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_fractional_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_fractional_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)\t2_c_nan_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests\input_data\t2_nan_vertexes | $(TESTDIR)
	$(RUN_CASE) "$(T2_C_EXE) < tests\input_data\t2_nan_vertexes > $(NULLDEV) 2> $(NULLDEV)" $(DATA) && \
        $(TOUCH) $@

# Task 3 cpp tests
# т.к. make делает каждый рецепт в новом cmd, то set PATH делается тут же, т.к. он только для данного процесса cmd
SET_DLL_PATH := set "PATH=$(CURDIR)\$(BINDIR)\common;%PATH%"
# тесты с кодами ошибок 
$(TESTDIR)\t3_cpp_empty_case.ok: $(T3_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_empty_case tests\expect\t3_empty_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_CPP_EXE) < tests\input_data\t3_empty_case > $(NULLDEV) 2> $(BINDIR)\t3_bbox_cpp\t3_empty_case" $(NO_INPUT) && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_cpp\t3_empty_case tests\expect\t3_empty_case && $(TOUCH) $@

$(TESTDIR)\t3_cpp_not_digit_case.ok: $(T3_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_not_digit_case tests\expect\t3_not_digit_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_CPP_EXE) < tests\input_data\t3_not_digit_case > $(NULLDEV) 2> $(BINDIR)\t3_bbox_cpp\t3_not_digit_case" $(DATA) && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_cpp\t3_not_digit_case tests\expect\t3_not_digit_case && $(TOUCH) $@

$(TESTDIR)\t3_cpp_too_few_args_case.ok: $(T3_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_too_few_args_case tests\expect\t3_too_few_args_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_CPP_EXE) < tests\input_data\t3_too_few_args_case > $(NULLDEV) 2> $(BINDIR)\t3_bbox_cpp\t3_too_few_args_case" $(DATA) && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_cpp\t3_too_few_args_case tests\expect\t3_too_few_args_case && $(TOUCH) $@ 

# тесты на нормальных входных данных
$(TESTDIR)\t3_cpp_test1.ok: $(T3_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_test1 tests\expect\t3_test1 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_CPP_EXE) < tests\input_data\t3_test1 > $(BINDIR)\t3_bbox_cpp\t3_test1 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_cpp\t3_test1 tests\expect\t3_test1 && $(TOUCH) $@

$(TESTDIR)\t3_cpp_test2.ok: $(T3_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_test2 tests\expect\t3_test2 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_CPP_EXE) < tests\input_data\t3_test2 > $(BINDIR)\t3_bbox_cpp\t3_test2 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_cpp\t3_test2 tests\expect\t3_test2 && $(TOUCH) $@  

$(TESTDIR)\t3_cpp_test3.ok: $(T3_CPP_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_test3 tests\expect\t3_test3 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_CPP_EXE) < tests\input_data\t3_test3 > $(BINDIR)\t3_bbox_cpp\t3_test3 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_cpp\t3_test3 tests\expect\t3_test3 && $(TOUCH) $@

# T3 c tests
# тесты с кодами ошибок
$(TESTDIR)\t3_c_empty_case.ok: $(T3_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_empty_case tests\expect\t3_empty_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_C_EXE) < tests\input_data\t3_empty_case > $(NULLDEV) 2> $(BINDIR)\t3_bbox_c\t3_empty_case" $(NO_INPUT) && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_c\t3_empty_case tests\expect\t3_empty_case && $(TOUCH) $@ 

$(TESTDIR)\t3_c_not_digit_case.ok: $(T3_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_not_digit_case tests\expect\t3_not_digit_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_C_EXE) < tests\input_data\t3_not_digit_case > $(NULLDEV) 2> $(BINDIR)\t3_bbox_c\t3_not_digit_case" $(DATA) && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_c\t3_not_digit_case tests\expect\t3_not_digit_case && $(TOUCH) $@ 

$(TESTDIR)\t3_c_too_few_args_case.ok: $(T3_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_too_few_args_case tests\expect\t3_too_few_args_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_C_EXE) < tests\input_data\t3_too_few_args_case > $(NULLDEV) 2> $(BINDIR)\t3_bbox_c\t3_too_few_args_case" $(DATA) && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_c\t3_too_few_args_case tests\expect\t3_too_few_args_case && $(TOUCH) $@ 

# тесты на нормальных входных данных
$(TESTDIR)\t3_c_test1.ok: $(T3_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_test1 tests\expect\t3_test1 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_C_EXE) < tests\input_data\t3_test1 > $(BINDIR)\t3_bbox_c\t3_test1 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_c\t3_test1 tests\expect\t3_test1 && $(TOUCH) $@

$(TESTDIR)\t3_c_test2.ok: $(T3_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_test2 tests\expect\t3_test2 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_C_EXE) < tests\input_data\t3_test2 > $(BINDIR)\t3_bbox_c\t3_test2 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_c\t3_test2 tests\expect\t3_test2 && $(TOUCH) $@

$(TESTDIR)\t3_c_test3.ok: $(T3_C_EXE) $(RUN_CASE) $(FIND_SUBSTR) tests\input_data\t3_test3 tests\expect\t3_test3 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE) "$(T3_C_EXE) < tests\input_data\t3_test3 > $(BINDIR)\t3_bbox_c\t3_test3 2> $(NULLDEV)" 0 && \
        $(FIND_SUBSTR) $(BINDIR)\t3_bbox_c\t3_test3 tests\expect\t3_test3 && $(TOUCH) $@


# ---------- ФИКТИВНЫЕ ЦЕЛИ (PHONY TARGETS) ---------- 
# фиктивные, т.к. по итогу их выполнения не будет итоговой цели как файла. они нужны для побочного результата - дерева пререквизитов
# а т.к. файла от такой phony target никогда не будет, то make каждый раз будет видеть:
# цели нет, значит строим пререквизиты - побоч. цель выполняется. (или выполняем рецепт, смотря что после цели)
# чтобы make не запутался (т.к. может быть, что появится файл с имененм phony target), делается спец. раздел: .PHONY: ... в помощь make
# этот раздел не может иметь рецепта, т.к. просто помечает цели как фиктивные 

.PHONY: all t1 t2 t3 test format format-check clean

# phony-s для получения файлов
all: t1 t2 t3
	@echo T1-T3 C and CPP COMPILED

t1: $(T1_CPP_EXE) $(T1_C_EXE)
	@echo T1_CPP T1_C COMPILED

t2: $(T2_CPP_EXE) $(T2_C_EXE)
	@echo T2_CPP T2_C COMPILED

t3: $(T3_CPP_EXE) $(T3_C_EXE)
	@echo T3_CPP T3_C COMPILED

test: format-check $(T1_CPP_TESTS) $(T1_C_TESTS) $(T2_CPP_TESTS) $(T2_C_TESTS) $(T3_CPP_TESTS) $(T3_C_TESTS)
	@echo T1_CPP T1_C T2_CPP T2_C T3_CPP T3_C: ALL TESTS PASSED 

# phony-s для работы над файлами  
format: 
	clang-format -i $(wildcard **/*.cpp **/*.c **/*.hpp **/*.h)
# --dry-run - не меняй файл, только выведи несоотв. (и warning, и error); Werror - warning переходят в разряд error
format-check: 
	clang-format --dry-run -Werror $(wildcard **/*.cpp **/*.c **/*.h **/*.hpp)
clean: 
	$(RMDIR) build 
	@echo BUILD DIR REMOVED

