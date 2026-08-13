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
# объектники и исполняемые
T1_CPP_OBJ := $(BINDIR)\t1_dist_matrix_cpp\main.obj $(BINDIR)\common\geometry_cpp.obj
T1_C_OBJ := $(BINDIR)\t1_dist_matrix_c\main.obj $(BINDIR)\common\geometry_c.obj
T1_CPP_EXE := $(BINDIR)\t1_dist_matrix_cpp\main.exe
T1_C_EXE := $(BINDIR)\t1_dist_matrix_c\main.exe

# тестовые
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

# exit-коды
USAGE := 64
DATA := 65
NO_INPUT := 66
IO_FAIL := 74

# отдельно вынесенные имена каталогов - визуально сократить order-only (абс. пути), т.к. % паттерн не м.б. в связке с order-only
CPP_DIRS := $(BINDIR)\t1_dist_matrix_cpp
C_DIRS := $(BINDIR)\t1_dist_matrix_c


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

# это правило захватит все .obj в подкаталогах /bin/..., 
# для которых есть зеркальные .cpp/.c (лежат в такой же папке, но она в корне проекта)
# и mkdir т.к. при первой сборке нужная папка в bindir не будет существовать 
$(BINDIR)\\%.obj: %.cpp | $(CPP_DIRS)
	$(CXX) /c /Fo:$@ $(CXXFLAGS) $<   

# аналогичное для Си
$(BINDIR)\\%.obj: %.c | $(C_DIRS) 
	$(CC) /c /Fo:$@ $(CFLAGS) $<

# правила для geometry, т.к. имена его объектников не совпадают с именами .src
$(BINDIR)\common\geometry_cpp.obj: common\geometry.cpp common\geometry.hpp | $(BINDIR)\common
	$(CXX) /c /DCOMMON_STATIC /Fo:$@ $(CXXFLAGS) $< 

$(BINDIR)\common\geometry_c.obj: common\geometry.c common\geometry.h | $(BINDIR)\common
	$(CC) /c /DCOMMON_STATIC /Fo:$@ $(CFLAGS) $<

# линковка. для первой задачи и модуля geometry линковка статическая, тк тут один geometry без string_utils
$(BINDIR)\t1_dist_matrix_cpp\main.exe: $(T1_CPP_OBJ) | $(BINDIR)\t1_dist_matrix_cpp
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^ 

$(BINDIR)\t1_dist_matrix_c\main.exe: $(T1_C_OBJ) | $(BINDIR)\t1_dist_matrix_c 
	$(LINK) $(LINK_FLAGS) /OUT:$@ $^


# ---------- ТЕСТЫ ---------- 
# (при успешном выполнеии появляются .ok маркеры с соотв. именами в build/.../test)
# небольшой .exe, который будет проверять, что код выхода программы совпадает с заданным кодом
# Fo - file output (obj) Fe - file executable (exe)
RUN_CASE := $(BINDIR)\tools\run_case.exe
$(BINDIR)\tools\run_case.exe: tests\run_case.cpp | $(BINDIR)\tools
	$(CXX) /Fo:$(BINDIR)\tools\run_case.obj /Fe:$@ $(CXXFLAGS) $<

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

# ---------- ФИКТИВНЫЕ ЦЕЛИ (PHONY TARGETS) ---------- 
# фиктивные, т.к. по итогу их выполнения не будет итоговой цели как файла. они нужны для побочного результата - дерева пререквизитов
# а т.к. файла от такой phony target никогда не будет, то make каждый раз будет видеть:
# цели нет, значит строим пререквизиты - побоч. цель выполняется. (или выполняем рецепт, смотря что после цели)
# чтобы make не запутался (т.к. может быть, что появится файл с имененм phony target), делается спец. раздел: .PHONY: ... в помощь make
# не может иметь рецепта, т.к. просто помечает цели как фиктивные

.PHONY: all t1 test format format-check

# phony-s для получения файлов
all: t1
t1: $(T1_CPP_EXE) $(T1_C_EXE)
test: format-check $(T1_CPP_TESTS) $(T1_C_TESTS)

# phony-s для работы над файлами  
format: 
	clang-format -i $(wildcard **/*.cpp **/*.c **/*.hpp **/*.h)
# --dry-run - не меняй файл, только выведи несоотв. (и warning, и error); Werror - warning переходят в разряд error
format-check: 
	clang-format --dry-run -Werror $(wildcard **/*.cpp **/*.c **/*.h **/*.hpp)
clean: 
	$(RMDIR) build












