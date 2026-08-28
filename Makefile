# ---------- ПАПКИ В /build ---------- 
# папка с учётом режима сборки. если не было передано иного, будет /debug
CONFIG ?= debug
# директория для бинарных файлов. внутри неё будут и debug, и release
BINDIR := build/$(CONFIG)
# для бинарников тестовых программ. по умолчанию будет build/debug/test
TESTDIR := $(BINDIR)/test


# ---------- ПЛАТФОРМОЗАВИСИМОЕ: ИНСТРУМЕНТЫ ДЛЯ СБОРКИ И ФЛАГИ, ЗАВИСЯЩИЕ ОТ РЕЖИМА (для cl, link) ---------- 
ifeq ($(OS), Windows_NT)
  # версия для MSVC
  CC := cl
  CXX := cl
  COMPILE_ONLY := /c
  LINK := link
  CSTD := /std:c17
  CXXSTD := /std:c++latest
  WARN := /W4 /permissive-
  OBJ_OUT := /Fo:
  EXE_OUT := /Fe:
  LINK_OUT := /OUT:
  DEFINE := /D
  DLL := /DLL
  GREP := findstr
  NULLDEV := nul
  MKDIR := mkdir
  TOUCH := type nul >
  # /s - всё содержимое; /q - quietly
  RMDIR := rmdir /s /q
  # изменение слешей в путях для виндоус. = а не := чтобы раскрывалось при каждом вызове, а не единств. раз
  WCMD = $(subst /,\,$1)
  # расширения исполняемых и объектников
  EXE_EXT :=.exe
  OBJ_EXT :=.obj
  # чтобы код был позиционно независимым (только для линукс)
  PIC_FLAGS :=
  # для раснесения сведений о символах - чтобы не было конфликта при едином vcX.pdb
  # будет давать name.obj.pdb - неконфликтно с итоговым name.pdb
  FD = /Fd:$@.pdb
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
  # флаги под линукс
  CC := gcc
  CXX := g++
  COMPILE_ONLY := -c
  LINK := g++
  CSTD := -std=c17
  CXXSTD := -std=c++23
  WARN := -Wall -Wextra
  OBJ_OUT := -o
  EXE_OUT := -o
  LINK_OUT := -o
  DEFINE := -D
  DLL := -shared
  GREP := grep
  NULLDEV := /dev/null
  MKDIR := mkdir -p
  TOUCH := touch
  # -rf: -r recursive -f force. необратимо, т.к. нет "корзины"
  RMDIR := rm -rf
  # изменение слешей в путях для виндоус. = а не := чтобы раскрывалось при каждом вызове, а не единств. раз
  WCMD = $1
  # расширения исполняемых и объектников
  EXE_EXT :=
  OBJ_EXT :=.o
  # чтобы код был позиционно независимым (только для линукс)
  PIC_FLAGS := -fPIC
  # windows-flag, на линуксе пустой
  FD :=
  ifeq ($(CONFIG), debug)
  # -O0 - без оптимизаций; 
  # -g - отладочкая инф., отдельного pdb не будет. лежит прямо в .о; 
  # для MDd прямого аналога нет (D_GLIBCXX_DEBUG но это не совсем то), поэтому пока без него 
  CONFIG_FLAGS := -O0 -g -fsanitize=address
  # -fsanitize=address даже при линковке чтобы линковщик нашёл runtime-санитайзер
  # DEBUG не нужен - т.к. отладочная будет в .o, не в отдельном .pdb
  LINK_FLAGS := -fsanitize=address
  else
  # -O2 - с оптимизацией
  # MD нет - на линуксе нет выбора CRT
  CONFIG_FLAGS := -O2 -g -DNDEBUG 
  # пока линковочные флаги пусты, потом возможно будет что-то типа -ffunction-sections -fdata-sections -Wl --gc-sections --icf=safe
  LINK_FLAGS := 
  endif
  # для включения .d файлов с заголовочниками, иначе make будет их "игнорировать"
  -include $(wildcard $(BINDIR)/*/*.d)
  # итоговый набор флагов cl
  # -MMD - вклчить не глобальные заголовочники в .d и сделать это не отдельной опреацией препроцессора, а во время компиляции
  # -MP - фантомные цели для header-ов чтобы спастись от падения и достичть пересборки .o при корректном сценарии и старом .d
  CXXFLAGS := $(CXXSTD) $(WARN) -MMD -MP $(CONFIG_FLAGS)  
  CFLAGS := $(CSTD) $(WARN) -MMD -MP $(CONFIG_FLAGS)
endif


# ---------- ПЕРЕМЕННЫЕ ДЛЯ ЦЕЛЕЙ (ЭТО ОБЪЕКТНИКИ, ИСПОЛНЯЕМЫЕ ФАЙЛЫ, ТЕСТОВЫЕ ФАЙЛЫ) ---------- 
# исполняемые файлы прикладных программ - сразу развилка на win и linux - по слешам
RUN_CASE := $(BINDIR)/tools/run_case$(EXE_EXT)
CHECK := $(BINDIR)/tools/check$(EXE_EXT)
ifeq ($(OS), Windows_NT)
  # если под windows То у исп. файла слеши должны быть обратные
  RUN_CASE_CMD := $(subst /,\,$(RUN_CASE))
  CHECK_CMD := $(subst /,\,$(CHECK))
else
  RUN_CASE_CMD := $(RUN_CASE)
  CHECK_CMD := $(CHECK)
endif

# объектники и исполняемые (цели и под-цели)
T1_CPP_OBJ := $(BINDIR)/t1_dist_matrix_cpp/main$(OBJ_EXT) $(BINDIR)/common/geometry_cpp$(OBJ_EXT)
T1_C_OBJ := $(BINDIR)/t1_dist_matrix_c/main$(OBJ_EXT) $(BINDIR)/common/geometry_c$(OBJ_EXT)
T1_CPP_EXE := $(BINDIR)/t1_dist_matrix_cpp/main$(EXE_EXT)
T1_C_EXE := $(BINDIR)/t1_dist_matrix_c/main$(EXE_EXT)

T2_CPP_OBJ := $(BINDIR)/t2_passport_cpp/main$(OBJ_EXT) $(BINDIR)/common/string_utils_cpp$(OBJ_EXT)
T2_C_OBJ := $(BINDIR)/t2_passport_c/main$(OBJ_EXT) $(BINDIR)/common/string_utils_c$(OBJ_EXT)
T2_CPP_EXE := $(BINDIR)/t2_passport_cpp/main$(EXE_EXT)
T2_C_EXE := $(BINDIR)/t2_passport_c/main$(EXE_EXT)

T3_CPP_EXE := $(BINDIR)/t3_bbox_cpp/main$(EXE_EXT)
T3_C_EXE := $(BINDIR)/t3_bbox_c/main$(EXE_EXT)

T4_CPP_EXE := $(BINDIR)/t4_filter_cpp/main$(EXE_EXT)
T4_C_EXE := $(BINDIR)/t4_filter_c/main$(EXE_EXT)

# target-specific переменные - для определения линковки на этапе компиляции - статическая / динамическая 
$(T1_CPP_EXE) $(T1_C_EXE) $(T2_CPP_EXE) $(T2_C_EXE): COMMON_FLAG := $(DEFINE)COMMON_STATIC
# для t3 - t4 этот флаг пустой - так функции будут помечаться, как импортируемые из dll
$(T3_CPP_EXE) $(T3_C_EXE) $(T4_CPP_EXE) $(T4_C_EXE): COMMON_FLAG :=

# цели (тестовые)
# тут нельзя через wildcard: если так сделать, то при построении дерева make будет сверяться с файловой системой
# и не увидит там ни одного из .ok файлов - прогона тестов ещё не было. тогда ни для одного .ok файла рецепт для неё не выполнится
# поэтом с абсолютными путями
T1_CPP_TESTS := $(TESTDIR)/t1_cpp_abc.ok \
	    $(TESTDIR)/t1_cpp_float.ok \
	    $(TESTDIR)/t1_cpp_low.ok \
	    $(TESTDIR)/t1_cpp_high.ok \
	    $(TESTDIR)/t1_cpp_nul.ok \
            $(TESTDIR)/t1_cpp_norm.ok

T1_C_TESTS := $(TESTDIR)/t1_c_abc.ok \
	    $(TESTDIR)/t1_c_float.ok \
	    $(TESTDIR)/t1_c_low.ok \
	    $(TESTDIR)/t1_c_high.ok \
	    $(TESTDIR)/t1_c_nul.ok \
            $(TESTDIR)/t1_c_norm.ok

T2_CPP_TESTS := $(TESTDIR)/t2_cpp_norm.ok \
                $(TESTDIR)/t2_cpp_eof_name.ok \
                $(TESTDIR)/t2_cpp_empty_name.ok \
                $(TESTDIR)/t2_cpp_eof_vertexes.ok \
                $(TESTDIR)/t2_cpp_empty_vertexes.ok \
                $(TESTDIR)/t2_cpp_negative_vertexes.ok \
                $(TESTDIR)/t2_cpp_fractional_vertexes.ok \
                $(TESTDIR)/t2_cpp_nan_vertexes.ok

T2_C_TESTS := $(TESTDIR)/t2_c_norm.ok \
                $(TESTDIR)/t2_c_eof_name.ok \
                $(TESTDIR)/t2_c_empty_name.ok \
                $(TESTDIR)/t2_c_eof_vertexes.ok \
                $(TESTDIR)/t2_c_empty_vertexes.ok \
                $(TESTDIR)/t2_c_negative_vertexes.ok \
                $(TESTDIR)/t2_c_fractional_vertexes.ok \
                $(TESTDIR)/t2_c_nan_vertexes.ok

T3_CPP_TESTS := $(TESTDIR)/t3_cpp_empty_case.ok \
                $(TESTDIR)/t3_cpp_not_digit_case.ok \
                $(TESTDIR)/t3_cpp_too_few_args_case.ok \
                $(TESTDIR)/t3_cpp_test1.ok \
                $(TESTDIR)/t3_cpp_test2.ok \
                $(TESTDIR)/t3_cpp_test3.ok 

T3_C_TESTS := $(TESTDIR)/t3_c_empty_case.ok \
                $(TESTDIR)/t3_c_not_digit_case.ok \
                $(TESTDIR)/t3_c_too_few_args_case.ok \
                $(TESTDIR)/t3_c_test1.ok \
                $(TESTDIR)/t3_c_test2.ok \
                $(TESTDIR)/t3_c_test3.ok 

T4_CPP_TESTS := $(TESTDIR)/t4_cpp_radius_empty.ok \
                $(TESTDIR)/t4_cpp_radius_too_much.ok \
                $(TESTDIR)/t4_cpp_radius_symbol.ok \
                $(TESTDIR)/t4_cpp_radius_inf.ok \
                $(TESTDIR)/t4_cpp_radius_negative.ok \
                $(TESTDIR)/t4_cpp_symbol_coords.ok \
                $(TESTDIR)/t4_cpp_too_few_coords.ok \
                $(TESTDIR)/t4_cpp_empty_coords.ok \
                $(TESTDIR)/t4_cpp_test1.ok \
                $(TESTDIR)/t4_cpp_test2.ok \
                $(TESTDIR)/t4_cpp_test3.ok 

T4_C_TESTS := $(TESTDIR)/t4_c_radius_empty.ok \
                $(TESTDIR)/t4_c_radius_too_much.ok \
                $(TESTDIR)/t4_c_radius_symbol.ok \
                $(TESTDIR)/t4_c_radius_inf.ok \
                $(TESTDIR)/t4_c_radius_negative.ok \
                $(TESTDIR)/t4_c_symbol_coords.ok \
                $(TESTDIR)/t4_c_too_few_coords.ok \
                $(TESTDIR)/t4_c_empty_coords.ok \
                $(TESTDIR)/t4_c_test1.ok \
                $(TESTDIR)/t4_c_test2.ok \
                $(TESTDIR)/t4_c_test3.ok

# exit-коды
USAGE := 64
DATA := 65
NO_INPUT := 66
IO_FAIL := 74

# отдельно вынесенные имена каталогов - визуально сократить order-only (абс. пути), т.к. % паттерн не м.б. в связке с order-only
CPP_DIRS := $(BINDIR)/t1_dist_matrix_cpp $(BINDIR)/t2_passport_cpp $(BINDIR)/t3_bbox_cpp $(BINDIR)/t4_filter_cpp
C_DIRS := $(BINDIR)/t1_dist_matrix_c $(BINDIR)/t2_passport_c $(BINDIR)/t3_bbox_c $(BINDIR)/t4_filter_c


# ---------- ПОДКАТАЛОГИ (ORDER-ONLY, ВАЖНО ТОЛЬКО ИХ НАЛИЧИЕ) ---------- 

# /tools существует для прикладных программ - run_case и check
# в итоге эта цепочка будет разложена на 5 правил вида "$(BINDIR)/t1_dist_matrix_cpp: $(MKDIR) $@"

# Make зайдёт сюда только если где-то будет, например, $(BINDIR)/common, но папки /common ещё нет. 
# тогда make придёт сюда и сделает инструкцию MKDIR
$(CPP_DIRS) $(C_DIRS) $(BINDIR)/common $(BINDIR)/tools $(TESTDIR):
	$(MKDIR) $(call WCMD,$@)


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

# заголовочники также пререкв. obj - иначе при изменении hdr obj останется старым
# излишние заголовочники для t1-t2 т.к. это незатратно и уменьшает усложнение которое есть, если писать разделение, под какой obj какие hdr

# деп-лист заголовочников (только для видноус)
ifeq ($(OS), Windows_NT)
 
GEOM_CPP_HDR := common/geometry.hpp
GEOM_C_HDR := common/geometry.h

STR_UTIL_CPP_HDR := common/string_utils.hpp
STR_UTIL_C_HDR := common/string_utils.h

CPP_OBJ_HDR := common/exit_codes.hpp $(STR_UTIL_CPP_HDR) $(GEOM_CPP_HDR)
C_OBJ_HDR := common/exit_codes.h $(STR_UTIL_C_HDR) $(GEOM_C_HDR)

else

GEOM_CPP_HDR := 
GEOM_C_HDR := 

STR_UTIL_CPP_HDR := 
STR_UTIL_C_HDR := 

CPP_OBJ_HDR := 
C_OBJ_HDR := 

endif


$(BINDIR)/%$(OBJ_EXT): %.cpp $(CPP_OBJ_HDR) | $(CPP_DIRS)
	$(CXX) $(COMPILE_ONLY) $(COMMON_FLAG) $(OBJ_OUT)$@ $(CXXFLAGS) $(FD) $<   

# аналогичное для Си
$(BINDIR)/%$(OBJ_EXT): %.c $(C_OBJ_HDR) | $(C_DIRS) 
	$(CC) $(COMPILE_ONLY) $(COMMON_FLAG) $(OBJ_OUT)$@ $(CFLAGS) $(FD) $<

# STATIC (t1-t2) правила для geometry + string_utils, т.к. их имена объектников не совпадают с именами .src
$(BINDIR)/common/geometry_cpp$(OBJ_EXT): common/geometry.cpp $(GEOM_CPP_HDR) | $(BINDIR)/common
	$(CXX) $(COMPILE_ONLY) $(DEFINE)COMMON_STATIC $(OBJ_OUT)$@ $(PIC_FLAGS) $(CXXFLAGS) $(FD) $< 

$(BINDIR)/common/geometry_c$(OBJ_EXT): common/geometry.c $(GEOM_C_HDR) | $(BINDIR)/common
	$(CC) $(COMPILE_ONLY) $(DEFINE)COMMON_STATIC $(OBJ_OUT)$@ $(PIC_FLAGS) $(CFLAGS) $(FD) $<

$(BINDIR)/common/string_utils_cpp$(OBJ_EXT): common/string_utils.cpp $(STR_UTIL_CPP_HDR) | $(BINDIR)/common
	$(CXX) $(COMPILE_ONLY) $(DEFINE)COMMON_STATIC $(OBJ_OUT)$@ $(PIC_FLAGS) $(CXXFLAGS) $(FD) $<

$(BINDIR)/common/string_utils_c$(OBJ_EXT): common/string_utils.c $(STR_UTIL_C_HDR) | $(BINDIR)/common
	$(CC) $(COMPILE_ONLY) $(DEFINE)COMMON_STATIC $(OBJ_OUT)$@ $(PIC_FLAGS) $(CFLAGS) $(FD) $<

# DYNAMIC (t3-t4)
# под линкусом флагов в заголовочниках вообще нет, поэтому и различия объектников (которое есть на винде) не будет
ifeq ($(OS), Windows_NT)
$(BINDIR)/common/geometry_cpp_dll$(OBJ_EXT): common/geometry.cpp $(GEOM_CPP_HDR) | $(BINDIR)/common
	$(CXX) $(COMPILE_ONLY) $(DEFINE)COMMON_EXPORTS $(OBJ_OUT)$@ $(CXXFLAGS) $(FD) $<

$(BINDIR)/common/geometry_c_dll$(OBJ_EXT): common/geometry.c $(GEOM_C_HDR) | $(BINDIR)/common
	$(CC) $(COMPILE_ONLY) $(DEFINE)COMMON_EXPORTS $(OBJ_OUT)$@ $(CFLAGS) $(FD) $<

$(BINDIR)/common/string_utils_cpp_dll$(OBJ_EXT): common/string_utils.cpp $(STR_UTIL_CPP_HDR) | $(BINDIR)/common
	$(CXX) $(COMPILE_ONLY) $(DEFINE)COMMON_EXPORTS $(OBJ_OUT)$@ $(CXXFLAGS) $(FD) $<

$(BINDIR)/common/string_utils_c_dll$(OBJ_EXT): common/string_utils.c $(STR_UTIL_C_HDR) | $(BINDIR)/common
	$(CC) $(COMPILE_ONLY) $(DEFINE)COMMON_EXPORTS $(OBJ_OUT)$@ $(CFLAGS) $(FD) $<
endif

# ЛИНКОВКИ
# линковка для 1 задачи и модуля geometry. линковка статическая, тк тут один geometry без string_utils
$(BINDIR)/t1_dist_matrix_cpp/main$(EXE_EXT): $(T1_CPP_OBJ) | $(BINDIR)/t1_dist_matrix_cpp
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^ 

$(BINDIR)/t1_dist_matrix_c/main$(EXE_EXT): $(T1_C_OBJ) | $(BINDIR)/t1_dist_matrix_c 
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^

# линковка для 2 задачи. также статич., с одним модулем string_utils
$(BINDIR)/t2_passport_cpp/main$(EXE_EXT): $(T2_CPP_OBJ) | $(BINDIR)/t2_passport_cpp
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^	

$(BINDIR)/t2_passport_c/main$(EXE_EXT): $(T2_C_OBJ) | $(BINDIR)/t2_passport_c
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^

# разнесение имён объектников и dll+lib / so до их использования по разным переменным
ifeq ($(OS), Windows_NT)
  # Объектники с пометками import / export на функциях 
  CPP_DLL_OBJ := $(BINDIR)/common/geometry_cpp_dll$(OBJ_EXT) $(BINDIR)/common/string_utils_cpp_dll$(OBJ_EXT)
  C_DLL_OBJ := $(BINDIR)/common/geometry_c_dll$(OBJ_EXT) $(BINDIR)/common/string_utils_c_dll$(OBJ_EXT)

  CPP_SHARED      := $(BINDIR)/common/common_cpp.dll
  CPP_SHARED_LINK := $(BINDIR)/common/common_cpp.lib    
  C_SHARED        := $(BINDIR)/common/common_c.dll
  C_SHARED_LINK   := $(BINDIR)/common/common_c.lib
else
  # под линуксом объектники обычные, без пометок inport / export
  CPP_OBJ := $(BINDIR)/common/geometry_cpp$(OBJ_EXT) $(BINDIR)/common/string_utils_cpp$(OBJ_EXT)
  C_OBJ := $(BINDIR)/common/geometry_c$(OBJ_EXT) $(BINDIR)/common/string_utils_c$(OBJ_EXT)
  # под линуксом линк будет с самой .so
  CPP_SHARED      := $(BINDIR)/common/libcommon_cpp.so
  CPP_SHARED_LINK := $(CPP_SHARED)                       
  C_SHARED        := $(BINDIR)/common/libcommon_c.so
  C_SHARED_LINK   := $(C_SHARED)
endif

# ===== ЛИНКОВКА ДИНАМИЧЕСКОЙ БИБЛИОТЕКИ =====
ifeq ($(OS), Windows_NT)
# линковка common_cpp.dll И common_c.dll: .lib также в целях, чтобы при линковке с main.exe .lib всегда существовал
# grouped target чтобы не было раскрытия на два правила
# жёсткий путь до dll а не $@, т.к. $@ - цель, которая триггернула правило. а не надо, чтобы был случай линка в .lib
# под виндоус важно держать .lib в дереве т.к. потом линковка конечного .exe будет именно с .lib

# удаление .lib каждый раз в начале, чтобы она пересоздавалась каждый раз. вне зависимости от содержания.
# иначе при незатрагивающих её изменениях линкер не заменяет её на новый возникающий .lib и .lib всегда старая -> всегда триггерит пересборку dll
# минус перед del - игнорировать ошибку если .lib не существует (первый прогон/ручное удалнеие) - так make пойдёт дальше и не упадёт

$(CPP_SHARED) $(CPP_SHARED_LINK) &: $(CPP_DLL_OBJ) | $(BINDIR)/common
	-del /q $(call WCMD,$(CPP_SHARED_LINK)) 2>$(NULLDEV)
	$(LINK) $(LINK_FLAGS) $(DLL) $(LINK_OUT)$(CPP_SHARED) $^

$(C_SHARED) $(C_SHARED_LINK) &: $(C_DLL_OBJ) | $(BINDIR)/common
	-del /q $(call WCMD,$(C_SHARED_LINK)) 2>$(NULLDEV)
	$(LINK) $(LINK_FLAGS) $(DLL) $(LINK_OUT)$(C_SHARED) $^
else
# под линукс будет только .so (dll) и не будет .lib с таблицей
$(CPP_SHARED): $(CPP_OBJ) | $(BINDIR)/common
	$(LINK) $(LINK_FLAGS) $(DLL) $(LINK_OUT)$(CPP_SHARED) $^

$(C_SHARED): $(C_OBJ) | $(BINDIR)/common
	$(LINK) $(LINK_FLAGS) $(DLL) $(LINK_OUT)$(C_SHARED) $^

endif
                    	
# линковка t3 - t4 с dll
# линковка либо с .lib (windows) либо с .so (Linux)
# (под windows) common_cpp.lib имеет свой рецепт для перестройки на случай, если его удалят. поэтому он - полноценный элемент дерева файлов
$(BINDIR)/t3_bbox_cpp/main$(EXE_EXT): $(BINDIR)/t3_bbox_cpp/main$(OBJ_EXT) $(CPP_SHARED_LINK) | $(BINDIR)/t3_bbox_cpp
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^

$(BINDIR)/t3_bbox_c/main$(EXE_EXT): $(BINDIR)/t3_bbox_c/main$(OBJ_EXT) $(C_SHARED_LINK) | $(BINDIR)/t3_bbox_c
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^

$(BINDIR)/t4_filter_cpp/main$(EXE_EXT): $(BINDIR)/t4_filter_cpp/main$(OBJ_EXT) $(CPP_SHARED_LINK) | $(BINDIR)/t4_filter_cpp
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^

$(BINDIR)/t4_filter_c/main$(EXE_EXT): $(BINDIR)/t4_filter_c/main$(OBJ_EXT) $(C_SHARED_LINK) | $(BINDIR)/t4_filter_c
	$(LINK) $(LINK_FLAGS) $(LINK_OUT)$@ $^

# ----- СБОРКА ПРИКЛАДНЫХ ПРОГРАММ (ДЛЯ ТЕСТИРОВАНИЯ) -----
# RUN_CASE - .exe, который будет проверять, что код выхода программы совпадает с заданным кодом
# CHECK - --equal f1 f2 - сравнеиние 2 файлов на равенство; --contains f1 f2 - поиск подстроки f2 в f1
# сравнение происходит побайтово
ifeq ($(OS), Windows_NT)
# obj_out = Fo - file output (obj) exe_out = Fe - file executable (exe)

# filter-out - взять из флагов всё, что не /Zi и /fsanitize=address, т.к. не надо отлаживать программы для тестирования программ
# с некотрой вероятностью отсутствие asan в них может вызвать проблемы при утечке памяти
# если так, то 1) $(filter-out ...$(CXXFLAGS)) -> $(CXXFLAGS) 2) добавить $(FD) - для своего .obj.pdb рядом

$(BINDIR)/tools/run_case.exe: tests/run_case.cpp | $(BINDIR)/tools
	$(CXX) $(OBJ_OUT)$(BINDIR)/tools/run_case$(OBJ_EXT) $(EXE_OUT)$@ $(filter-out /Zi /fsanitize=address,$(CXXFLAGS)) $<

$(BINDIR)/tools/check.exe: tests/check.cpp | $(BINDIR)/tools
	$(CXX) $(OBJ_OUT)$(BINDIR)/tools/check$(OBJ_EXT) $(EXE_OUT)$@ $(filter-out /Zi /fsanitize=address,$(CXXFLAGS)) $<

else
# под линуксом проблем с asan не будет, поэтому здесь он остаётся. без filter-out
$(BINDIR)/tools/run_case: tests/run_case.cpp | $(BINDIR)/tools
	$(CXX) $(EXE_OUT)$@ $(CXXFLAGS) $<

$(BINDIR)/tools/check: tests/check.cpp | $(BINDIR)/tools
	$(CXX) $(EXE_OUT)$@ $(CXXFLAGS) $<
endif


# ---------- ТЕСТЫ ---------- 
# (при успешном выполнеии появляются .ok маркеры с соотв. именами в build/.../test)
# выходные данные нормальных тестов проверяются через полное равенство reuslt и expected, 
# для ошибочных случаев проверяется содержание подстроки из expected в result

# TASK 1
# cpp tests
# exit-code тесты
# после && будет выполнение, только если до && код выхода == 0. работает и в shell, и в cmd
# флаги для check.exe 
EQ := --equal
CONTAINS := --contains 
$(TESTDIR)/t1_cpp_abc.ok: $(T1_CPP_EXE) $(RUN_CASE) tests/input_data/t1_abc | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_CPP_EXE) < tests/input_data/t1_abc > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && $(TOUCH) $@

$(TESTDIR)/t1_cpp_float.ok: $(T1_CPP_EXE) $(RUN_CASE) tests/input_data/t1_float | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_CPP_EXE) < tests/input_data/t1_float > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && $(TOUCH) $@

$(TESTDIR)/t1_cpp_low.ok: $(T1_CPP_EXE) $(RUN_CASE) tests/input_data/t1_low | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_CPP_EXE) < tests/input_data/t1_low > $(NULLDEV) 2> $(NULLDEV))" $(USAGE) && $(TOUCH) $@

$(TESTDIR)/t1_cpp_high.ok: $(T1_CPP_EXE) $(RUN_CASE) tests/input_data/t1_high | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_CPP_EXE) < tests/input_data/t1_high > $(NULLDEV) 2> $(NULLDEV))" $(USAGE) && $(TOUCH) $@

$(TESTDIR)/t1_cpp_nul.ok: $(T1_CPP_EXE) $(RUN_CASE) tests/input_data/t1_nul | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_CPP_EXE) < tests/input_data/t1_nul > $(NULLDEV) 2> $(NULLDEV))" $(NO_INPUT) && $(TOUCH) $@

# тест с корретными данными. проверка на норм. выходные данные
$(TESTDIR)/t1_cpp_norm.ok: $(T1_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t1_test1 tests/expect/t1_test1 | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_CPP_EXE) < tests/input_data/t1_test1 > $(BINDIR)/t1_dist_matrix_cpp/t1_test1 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t1_dist_matrix_cpp/t1_test1 tests/expect/t1_test1) && $(TOUCH) $@

# C tests
# exit-code тесты
$(TESTDIR)/t1_c_abc.ok: $(T1_C_EXE) $(RUN_CASE) tests/input_data/t1_abc | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_C_EXE) < tests/input_data/t1_abc > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && $(TOUCH) $@

$(TESTDIR)/t1_c_float.ok: $(T1_C_EXE) $(RUN_CASE) tests/input_data/t1_float | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_C_EXE) < tests/input_data/t1_float > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && $(TOUCH) $@

$(TESTDIR)/t1_c_low.ok: $(T1_C_EXE) $(RUN_CASE) tests/input_data/t1_low | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_C_EXE) < tests/input_data/t1_low > $(NULLDEV) 2> $(NULLDEV))" $(USAGE) && $(TOUCH) $@

$(TESTDIR)/t1_c_high.ok: $(T1_C_EXE) $(RUN_CASE) tests/input_data/t1_high | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_C_EXE) < tests/input_data/t1_high > $(NULLDEV) 2> $(NULLDEV))" $(USAGE) && $(TOUCH) $@

$(TESTDIR)/t1_c_nul.ok: $(T1_C_EXE) $(RUN_CASE) tests/input_data/t1_nul | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_C_EXE) < tests/input_data/t1_nul > $(NULLDEV) 2> $(NULLDEV))" $(NO_INPUT) && $(TOUCH) $@

# тест с корректными данными. проверка на норм. выходные данные
$(TESTDIR)/t1_c_norm.ok: $(T1_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t1_test1 tests/expect/t1_test1 | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T1_C_EXE) < tests/input_data/t1_test1 > $(BINDIR)/t1_dist_matrix_c/t1_test1 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t1_dist_matrix_c/t1_test1 tests/expect/t1_test1) && $(TOUCH) $@

# TASK 2 cpp tests
# тест на нормальных данных
$(TESTDIR)/t2_cpp_norm.ok: $(T2_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t2_input tests/expect/t2_norm | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_input > $(BINDIR)/t2_passport_cpp/t2_norm 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t2_passport_cpp/t2_norm tests/expect/t2_norm) && $(TOUCH) $@

# тесты на коды ошибок
$(TESTDIR)/t2_cpp_eof_name.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_eof_name | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_eof_name > $(NULLDEV) 2> $(NULLDEV))" $(NO_INPUT) && \
        $(TOUCH) $@

$(TESTDIR)/t2_cpp_empty_name.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_empty_name | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_empty_name > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@ 

$(TESTDIR)/t2_cpp_eof_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_eof_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_eof_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(NO_INPUT) && \
        $(TOUCH) $@

$(TESTDIR)/t2_cpp_empty_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_empty_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_empty_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)/t2_cpp_negative_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_negative_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_negative_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(USAGE) && \
        $(TOUCH) $@ 

$(TESTDIR)/t2_cpp_fractional_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_fractional_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_fractional_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)/t2_cpp_nan_vertexes.ok: $(T2_CPP_EXE) $(RUN_CASE) tests/input_data/t2_nan_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_CPP_EXE) < tests/input_data/t2_nan_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@

# TASK 2 c tests
# тест на нормальных данных
$(TESTDIR)/t2_c_norm.ok: $(T2_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t2_input tests/expect/t2_norm | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_input > $(BINDIR)/t2_passport_c/t2_norm 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t2_passport_c/t2_norm tests/expect/t2_norm) && $(TOUCH) $@

# тесты на коды ошибок
$(TESTDIR)/t2_c_eof_name.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_eof_name | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_eof_name > $(NULLDEV) 2> $(NULLDEV))" $(NO_INPUT) && \
        $(TOUCH) $@

$(TESTDIR)/t2_c_empty_name.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_empty_name | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_empty_name > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@ 

$(TESTDIR)/t2_c_eof_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_eof_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_eof_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(NO_INPUT) && \
        $(TOUCH) $@

$(TESTDIR)/t2_c_empty_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_empty_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_empty_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)/t2_c_negative_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_negative_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_negative_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(USAGE) && \
        $(TOUCH) $@ 

$(TESTDIR)/t2_c_fractional_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_fractional_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_fractional_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@

$(TESTDIR)/t2_c_nan_vertexes.ok: $(T2_C_EXE) $(RUN_CASE) tests/input_data/t2_nan_vertexes | $(TESTDIR)
	$(RUN_CASE_CMD) "$(call WCMD,$(T2_C_EXE) < tests/input_data/t2_nan_vertexes > $(NULLDEV) 2> $(NULLDEV))" $(DATA) && \
        $(TOUCH) $@

# Task 3 cpp tests
# определение переменной для нахождения пути динамической библиотеки
# т.к. make делает каждый рецепт в новом cmd, то set PATH делается тут же, т.к. он только для данного процесса cmd
ifeq ($(OS), Windows_NT)
  SET_DLL_PATH := set "PATH=$(CURDIR)/$(BINDIR)/common;%PATH%"
else
  SET_DLL_PATH := export LD_LIBRARY_PATH=$(CURDIR)/$(BINDIR)/common:$$LD_LIBRARY_PATH
endif

# тесты с кодами ошибок 
$(TESTDIR)/t3_cpp_empty_case.ok: $(T3_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_empty_case tests/expect/t3_empty_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_CPP_EXE) < tests/input_data/t3_empty_case > $(NULLDEV) 2> $(BINDIR)/t3_bbox_cpp/t3_empty_case)" $(NO_INPUT) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t3_bbox_cpp/t3_empty_case tests/expect/t3_empty_case) && $(TOUCH) $@

$(TESTDIR)/t3_cpp_not_digit_case.ok: $(T3_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_not_digit_case tests/expect/t3_not_digit_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_CPP_EXE) < tests/input_data/t3_not_digit_case > $(NULLDEV) 2> $(BINDIR)/t3_bbox_cpp/t3_not_digit_case)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t3_bbox_cpp/t3_not_digit_case tests/expect/t3_not_digit_case) && $(TOUCH) $@

$(TESTDIR)/t3_cpp_too_few_args_case.ok: $(T3_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_too_few_args_case tests/expect/t3_too_few_args_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_CPP_EXE) < tests/input_data/t3_too_few_args_case > $(NULLDEV) 2> $(BINDIR)/t3_bbox_cpp/t3_too_few_args_case)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t3_bbox_cpp/t3_too_few_args_case tests/expect/t3_too_few_args_case) && $(TOUCH) $@ 

# тесты на нормальных входных данных
$(TESTDIR)/t3_cpp_test1.ok: $(T3_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_test1 tests/expect/t3_test1 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_CPP_EXE) < tests/input_data/t3_test1 > $(BINDIR)/t3_bbox_cpp/t3_test1 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t3_bbox_cpp/t3_test1 tests/expect/t3_test1) && $(TOUCH) $@

$(TESTDIR)/t3_cpp_test2.ok: $(T3_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_test2 tests/expect/t3_test2 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_CPP_EXE) < tests/input_data/t3_test2 > $(BINDIR)/t3_bbox_cpp/t3_test2 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t3_bbox_cpp/t3_test2 tests/expect/t3_test2) && $(TOUCH) $@  

$(TESTDIR)/t3_cpp_test3.ok: $(T3_CPP_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_test3 tests/expect/t3_test3 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_CPP_EXE) < tests/input_data/t3_test3 > $(BINDIR)/t3_bbox_cpp/t3_test3 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t3_bbox_cpp/t3_test3 tests/expect/t3_test3) && $(TOUCH) $@

# T3 c tests
# тесты с кодами ошибок
$(TESTDIR)/t3_c_empty_case.ok: $(T3_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_empty_case tests/expect/t3_empty_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_C_EXE) < tests/input_data/t3_empty_case > $(NULLDEV) 2> $(BINDIR)/t3_bbox_c/t3_empty_case)" $(NO_INPUT) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t3_bbox_c/t3_empty_case tests/expect/t3_empty_case) && $(TOUCH) $@

$(TESTDIR)/t3_c_not_digit_case.ok: $(T3_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_not_digit_case tests/expect/t3_not_digit_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_C_EXE) < tests/input_data/t3_not_digit_case > $(NULLDEV) 2> $(BINDIR)/t3_bbox_c/t3_not_digit_case)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t3_bbox_c/t3_not_digit_case tests/expect/t3_not_digit_case) && $(TOUCH) $@

$(TESTDIR)/t3_c_too_few_args_case.ok: $(T3_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_too_few_args_case tests/expect/t3_too_few_args_case | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_C_EXE) < tests/input_data/t3_too_few_args_case > $(NULLDEV) 2> $(BINDIR)/t3_bbox_c/t3_too_few_args_case)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t3_bbox_c/t3_too_few_args_case tests/expect/t3_too_few_args_case) && $(TOUCH) $@ 

# тесты на нормальных входных данных
$(TESTDIR)/t3_c_test1.ok: $(T3_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_test1 tests/expect/t3_test1 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_C_EXE) < tests/input_data/t3_test1 > $(BINDIR)/t3_bbox_c/t3_test1 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t3_bbox_c/t3_test1 tests/expect/t3_test1) && $(TOUCH) $@

$(TESTDIR)/t3_c_test2.ok: $(T3_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_test2 tests/expect/t3_test2 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_C_EXE) < tests/input_data/t3_test2 > $(BINDIR)/t3_bbox_c/t3_test2 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t3_bbox_c/t3_test2 tests/expect/t3_test2) && $(TOUCH) $@  

$(TESTDIR)/t3_c_test3.ok: $(T3_C_EXE) $(RUN_CASE) $(CHECK) tests/input_data/t3_test3 tests/expect/t3_test3 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T3_C_EXE) < tests/input_data/t3_test3 > $(BINDIR)/t3_bbox_c/t3_test3 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t3_bbox_c/t3_test3 tests/expect/t3_test3) && $(TOUCH) $@

# T4 cpp tests
# неверные значения радиуса. подаётся числом, т.к. это аргумент командной строки
# пустой радиус
$(TESTDIR)/t4_cpp_radius_empty.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_empty | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_radius_empty)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_radius_empty tests/expect/t4_radius_empty) && $(TOUCH) $@ 

# два аргумента, хотя нужно ввести только один
$(TESTDIR)/t4_cpp_radius_too_much.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_too_much | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 2 3 > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_radius_too_much)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_radius_too_much tests/expect/t4_radius_too_much) && $(TOUCH) $@

# нечисловые символы в радиусе
$(TESTDIR)/t4_cpp_radius_symbol.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_symbol | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 4ch > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_radius_symbol)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_radius_symbol tests/expect/t4_radius_symbol) && $(TOUCH) $@ 

# бесконечность, хотя радиус должен быть конечным числом
$(TESTDIR)/t4_cpp_radius_inf.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_inf | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) inf > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_radius_inf)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_radius_inf tests/expect/t4_radius_inf) && $(TOUCH) $@ 

# отрицат. значение радиуса
$(TESTDIR)/t4_cpp_radius_negative.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_negative | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) -10 > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_radius_negative)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_radius_negative tests/expect/t4_radius_negative) && $(TOUCH) $@    

# тесты с ошибочными значениями точек. точки уже из файла
# буквы среди чисел
$(TESTDIR)/t4_cpp_symbol_coords.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_symbol_coords tests/input_data/t4_symbol_coords | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 1 < tests/input_data/t4_symbol_coords > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_symbol_coords)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_symbol_coords tests/expect/t4_symbol_coords) && $(TOUCH) $@

# меньше трёх координат
$(TESTDIR)/t4_cpp_too_few_coords.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_too_few_coords tests/input_data/t4_too_few_coords | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 1 < tests/input_data/t4_too_few_coords > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_too_few_coords)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_too_few_coords tests/expect/t4_too_few_coords) && $(TOUCH) $@  

# отсутствие координат
$(TESTDIR)/t4_cpp_empty_coords.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_empty_coords tests/input_data/t4_empty_coords | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 1 < tests/input_data/t4_empty_coords > $(NULLDEV) 2> $(BINDIR)/t4_filter_cpp/t4_empty_coords)" $(NO_INPUT) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_empty_coords tests/expect/t4_empty_coords) && $(TOUCH) $@

# единственная точка и она проходит
$(TESTDIR)/t4_cpp_test1.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_test1 tests/input_data/t4_test1 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 8 < tests/input_data/t4_test1 > $(BINDIR)/t4_filter_cpp/t4_test1 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_test1 tests/expect/t4_test1) && $(TOUCH) $@

# много точек и только одна проходит
$(TESTDIR)/t4_cpp_test2.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_test2 tests/input_data/t4_test2 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 10 < tests/input_data/t4_test2 > $(BINDIR)/t4_filter_cpp/t4_test2 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_test2 tests/expect/t4_test2) && $(TOUCH) $@

# одна точка, но она не проходит 
$(TESTDIR)/t4_cpp_test3.ok: $(T4_CPP_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_test3 tests/input_data/t4_test3 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_CPP_EXE) 5 < tests/input_data/t4_test3 > $(BINDIR)/t4_filter_cpp/t4_test3 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t4_filter_cpp/t4_test3 tests/expect/t4_test3) && $(TOUCH) $@

# T4 c tests
# неверные значения радиуса. подаётся числом, т.к. это аргумент командной строки
# пустой радиус
$(TESTDIR)/t4_c_radius_empty.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_empty | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_radius_empty)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_radius_empty tests/expect/t4_radius_empty) && $(TOUCH) $@ 

# два аргумента, хотя нужно ввести только один
$(TESTDIR)/t4_c_radius_too_much.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_too_much | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 2 3 > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_radius_too_much)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_radius_too_much tests/expect/t4_radius_too_much) && $(TOUCH) $@

# нечисловые символы в радиусе
$(TESTDIR)/t4_c_radius_symbol.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_symbol | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 4ch > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_radius_symbol)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_radius_symbol tests/expect/t4_radius_symbol) && $(TOUCH) $@ 

# бесконечность, хотя радиус должен быть конечным числом
$(TESTDIR)/t4_c_radius_inf.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_inf | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) inf > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_radius_inf)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_radius_inf tests/expect/t4_radius_inf) && $(TOUCH) $@ 

# отрицат. значение радиуса
$(TESTDIR)/t4_c_radius_negative.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_radius_negative | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) -10 > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_radius_negative)" $(USAGE) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_radius_negative tests/expect/t4_radius_negative) && $(TOUCH) $@    

# тесты с ошибочными значениями точек. точки уже из файла
# буквы среди чисел
$(TESTDIR)/t4_c_symbol_coords.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_symbol_coords tests/input_data/t4_symbol_coords | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 1 < tests/input_data/t4_symbol_coords > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_symbol_coords)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_symbol_coords tests/expect/t4_symbol_coords) && $(TOUCH) $@

# меньше трёх координат
$(TESTDIR)/t4_c_too_few_coords.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_too_few_coords tests/input_data/t4_too_few_coords | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 1 < tests/input_data/t4_too_few_coords > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_too_few_coords)" $(DATA) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_too_few_coords tests/expect/t4_too_few_coords) && $(TOUCH) $@  

# отсутствие координат
$(TESTDIR)/t4_c_empty_coords.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_empty_coords tests/input_data/t4_empty_coords | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 1 < tests/input_data/t4_empty_coords > $(NULLDEV) 2> $(BINDIR)/t4_filter_c/t4_empty_coords)" $(NO_INPUT) && \
        $(CHECK_CMD) $(CONTAINS) $(call WCMD,$(BINDIR)/t4_filter_c/t4_empty_coords tests/expect/t4_empty_coords) && $(TOUCH) $@

# единственная точка и она проходит
$(TESTDIR)/t4_c_test1.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_test1 tests/input_data/t4_test1 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 8 < tests/input_data/t4_test1 > $(BINDIR)/t4_filter_c/t4_test1 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t4_filter_c/t4_test1 tests/expect/t4_test1) && $(TOUCH) $@

# много точек и только одна проходит
$(TESTDIR)/t4_c_test2.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_test2 tests/input_data/t4_test2 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 10 < tests/input_data/t4_test2 > $(BINDIR)/t4_filter_c/t4_test2 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t4_filter_c/t4_test2 tests/expect/t4_test2) && $(TOUCH) $@

# одна точка, но она не проходит 
$(TESTDIR)/t4_c_test3.ok: $(T4_C_EXE) $(RUN_CASE) $(CHECK) tests/expect/t4_test3 tests/input_data/t4_test3 | $(TESTDIR)
	$(SET_DLL_PATH) && \
        $(RUN_CASE_CMD) "$(call WCMD,$(T4_C_EXE) 5 < tests/input_data/t4_test3 > $(BINDIR)/t4_filter_c/t4_test3 2> $(NULLDEV))" 0 && \
        $(CHECK_CMD) $(EQ) $(call WCMD,$(BINDIR)/t4_filter_c/t4_test3 tests/expect/t4_test3) && $(TOUCH) $@


# ---------- ФИКТИВНЫЕ ЦЕЛИ (PHONY TARGETS) ---------- 
# фиктивные, т.к. по итогу их выполнения не будет итоговой цели как файла. они нужны для побочного результата - дерева пререквизитов
# а т.к. файла от такой phony target никогда не будет, то make каждый раз будет видеть:
# цели нет, значит строим пререквизиты - побоч. цель выполняется. (или выполняем рецепт, смотря что после цели)
# чтобы make не запутался (т.к. может быть, что появится файл с имененм phony target), делается спец. раздел: .PHONY: ... в помощь make
# этот раздел не может иметь рецепта, т.к. просто помечает цели как фиктивные 

.PHONY: all t1 t2 t3 t4 test format format-check clean

# по умолчанию, для make без конкр. цели, будет делаться all; иначе в таком случае выполняется первая  по порядку цель из makefile
.DEFAULT_GOAL := all

# phony-s для получения файлов
all: t1 t2 t3 t4
	@echo T1-T4 C and CPP COMPILED

t1: $(T1_CPP_EXE) $(T1_C_EXE)
	@echo T1_CPP T1_C COMPILED

t2: $(T2_CPP_EXE) $(T2_C_EXE)
	@echo T2_CPP T2_C COMPILED

t3: $(T3_CPP_EXE) $(T3_C_EXE)
	@echo T3_CPP T3_C COMPILED

t4: $(T4_CPP_EXE) $(T4_C_EXE)
	@echo T4_CPP T4_C COMPILED 

test: format-check $(T1_CPP_TESTS) $(T1_C_TESTS) $(T2_CPP_TESTS) $(T2_C_TESTS) $(T3_CPP_TESTS) $(T3_C_TESTS) $(T4_CPP_TESTS) $(T4_C_TESTS)
	@echo T1_CPP T1_C T2_CPP T2_C T3_CPP T3_C T4_CPP T4_C: ALL TESTS PASSED 

# phony-s для работы над файлами  
format: 
	clang-format -i $(wildcard **/*.cpp **/*.c **/*.hpp **/*.h)

# поверх - проверка на случай, когда у пользователя нет clang-format-a; падения не будет, но выведется предупреждение и рекомендация установки
# --dry-run - не меняй файл, только выведи несоотв. (и warning, и error); Werror - warning переходят в разряд error
ifeq ($(OS), Windows_NT)
format-check:
	@echo Checking file format (clang-format --dry-run -Werror)...
	@where clang-format >nul 2>nul & if errorlevel 1\
        (echo clang-format was not found, so file format was not checked. The following installation is recomended: winget install --id=LLVM.ClangFormat -e)\
        else (clang-format --dry-run -Werror $(wildcard **/*.cpp **/*.c **/*.h **/*.hpp) && echo File format is correct)
else
# тут другя логика: в if успех - exit-code = 0, а не наоборот 
format-check:
	@echo Checking file format: clang-format --dry-run -Werror...	
	@if command -v clang-format >/dev/null 2>&1; then clang-format --dry-run -Werror $(wildcard **/*.cpp **/*.c **/*.h **/*.hpp) &&\
        echo "File format is correct"; else\
        echo "clang-format was not found, so file format was not checked. The following installation is recomended: sudo apt install clang-format"; fi
endif

clean: 
	$(RMDIR) build 
	@echo BUILD DIR REMOVED

