#include <print>
#include <string>
#include <string_view>
#include <system_error>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
// печать ошибки - передаётся строка со смыслом проблемы и имя файла, с которым была проблема
void print_win_error(std::string_view context, std::string_view target) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось {} {}: {} (код {})\n", context, target, ec.message(), current_err);
}

// обёртка для хэндлов. сделана, когда нашлась проблема каскадных CloseHandle
// у HANDLE нет деструктора, это вообще не класс, а псевдоним для числа-указателя
// а обёртка позволяет закрыть хэндл при выходе из скоупа, а не руками перед return ...
class win_handle {
    // h_: член класса win_handle, приватный
    // по дефолту и при создании из результата неудачного вызова будет = INVALID_HANDLE_VALUE
    HANDLE h_ = INVALID_HANDLE_VALUE;

public:
    // конструктор. значение h по умолчанию - INVALID_HANDLE_VALUE
    // при передаче реального хэндла внутренний h_ будет им инициализирован
    win_handle(HANDLE h = INVALID_HANDLE_VALUE) : h_(h) {}
    // деструктор. при валидном h_ - закрываем его
    ~win_handle() {
        if (h_ != INVALID_HANDLE_VALUE && h_ != nullptr)
            CloseHandle(h_);
    }

    // устранение ситуации двойного закрытия:
    // нельзя копировать через конструктор - т.е. нельзя сделать объект win_handle от win_handle, можно от HANDLE
    win_handle(const win_handle&) = delete;
    // нельзя сделать win_handle a = b, где b - тоже win_handle
    win_handle& operator=(const win_handle&) = delete;

    // отдать HANDLE - т.е. просто число. ему не сделают CloseHandle и управление остаётся у обёртки
    HANDLE get() const { return h_; }
    // преобразование хэндла к 0/1 (explicit) - только в логических операциях, не в неявных преобразованиях
    explicit operator bool() const { return h_ != INVALID_HANDLE_VALUE && h_ != nullptr; }
};

// вынесение создания и обработки ошибки создания хэндла
win_handle open_io_file(const char* path, DWORD desired_access, DWORD creation_disposition, std::string_view context) {
    // дескриптор безопасности для объектов, которые созданы/открыты внутри программы (для файлов in/out/err)
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);           // размер структ. в байтах; будет размер самого SECURITY_ATTRIBUTES
    sa.bInheritHandle = true;          // хэндл с этим sa может наследоваться дочерним процессом
    sa.lpSecurityDescriptor = nullptr; // разрешения безопасности - по дефолту

    // хэндл на переданный файл (созданный / открытый)
    // FILE_SHARE_READ - другому процессу тоже можно открыть на чтение данный файл. для НЕ-run_case и НЕ-main процессов
    HANDLE h =
        CreateFileA(path, desired_access, FILE_SHARE_READ, &sa, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);

    // при неудачном создании хэндла - ввести сообщение и вернуть объект
    if (h == INVALID_HANDLE_VALUE) {
        print_win_error(context, path);
        return win_handle{}; // конаструктор по умолчанию вернёт INVALID_HANDLE_VALUE
    }
    // возврат объекта через prvalue - сработает copy elision. нет копирования
    // объект будет в памяти сразу на нужном месте
    return win_handle{h};
}

#endif

// run_case <ожидаемый_код> <файл_входа> <файл_stdout> <файл_stderr> -- <exe> [арг...]
int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::print(stderr,
                   "Ожидалось не менее 6 аргументов: <expect_code> <file_input> <file_stdout> <file_stderr> -- <exe>. "
                   "Получено: {}\n",
                   argc - 1);
        return 1;
    }

    if (std::string(argv[5]) != "--") {
        std::print(stderr, "Ожидался разделитель \"--\" на 5-й позиции, получено: {}\n", argv[5]);
        return 1;
    }

    int expected_code = 0;

    try {
        expected_code = std::stoi(argv[1]);
    } catch (const std::invalid_argument&) {
        std::print(stderr, "Ожидаемый exit-code - целое число. Получено: {}\n", argv[1]);
        return 1;
    } catch (const std::out_of_range&) {
        std::print(stderr, "Ожидается exit-code типа int, а полученное число({}) - за пределами int.\n", argv[1]);
        return 1;
    }

// виндоус-специфичная ветка для создания дочернего процесса с переданными аргументами и сравнение кода
#ifdef _WIN32
    // если процесс будет создан успешно, здесь будут
    // hProcess - хэндл на объект ядра процесс. нужен для ожидания завершения, снятия кода
    // hThread - хэндл на объект ядра поток(главный поток процесса). не используется, но нужно будет закрыть его
    // пустой конструктор чтобы не осталось мусора в полях
    PROCESS_INFORMATION pi{};

    //  логический блок для своевременного уничтожения in/out/err Обёрток
    {

        // хэндлы на файлы входа, выхода и ошибки
        win_handle in = open_io_file(argv[2], GENERIC_READ, OPEN_EXISTING, "открыть файл входа");
        if (!in)
            return 1;

        win_handle out = open_io_file(argv[3], GENERIC_WRITE, CREATE_ALWAYS, "создать файл выхода");
        if (!out)
            return 1;

        win_handle err = open_io_file(argv[4], GENERIC_WRITE, CREATE_ALWAYS, "создать файл ошибки");
        if (!err)
            return 1;

        // параметры для будущего процесса
        // hStdInput/hStdOutput/hStdError - это будущие хэндлы на открытые/созданные файлы in/out/err
        // сработают при создании процесса с этим STARTUPINFOA. дадут ребёнку полную копию объектов ядра, как у run_case
        STARTUPINFOA si{};
        si.cb = sizeof(si);                // размер структуры = размер STARTUPINFOA
        si.dwFlags = STARTF_USESTDHANDLES; // хэндлы воспринимаются как потоки
        si.hStdInput = in.get();
        si.hStdOutput = out.get();
        si.hStdError = err.get();

        // строка вида "main.exe" arg1 arg2 ...; кавычки - от пробелов в пути
        std::string cmdline = "\"" + std::string(argv[6]) + "\"";
        for (int i = 7; i < argc; ++i) {
            cmdline += ' ';
            cmdline += argv[i];
        }

        // был отказ от CreateProcessW в CreateProcessA пользу чтобы не конверт. в char* -> wchar_t
        // плохо: не-ASCII пути зависят от кодировки. но у меня нет кириллицы в путях
        // argv[6] - имя создаваемого процесса - аргумент-имя исполняемого файла
        // cmdline.data() - исполняемый файл и его аргументы командной строки
        // 3 и 4 NULL - нет своих параметров безопасности для процесса и потока
        // TRUE - этот процесс может наследовать. тут унаследует объекты ядра in/out/err от run_case
        // NULL - окружение по умолчанию - то есть от родителя
        // NULL - current working directory как у родителя
        // startup info = si, process information = pi
        if (!CreateProcessA(argv[6], cmdline.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            print_win_error("запустить", argv[6]);
            return 1;
        }
    } // здесь происходит удаление in/out/err через деструкторы для run_case

    win_handle hproc{pi.hProcess};
    win_handle hthread{pi.hThread};

    DWORD exit_code = 0;
    // run_case блокируется и ждёт, пока не завершится процесс pi.hProcess
    // ждёт бесконечно, но ловит сбои ожидания и если они есть - бросит ошибку и завершится
    if (WaitForSingleObject(hproc.get(), INFINITE) != WAIT_OBJECT_0) {
        print_win_error("дождаться завершения", argv[6]);
        return 1;
    }
    // GetExitCodeProcess - снять код завершения процесса pi.hProcess и положить в exit_code
    // проверка, если вдруг не удалось снять взять код завершения
    if (!GetExitCodeProcess(hproc.get(), &exit_code)) {
        print_win_error("получить код завершения", argv[6]);
        return 1;
    }

    // сравнение снятого кода процесса и ожидаемого
    if (static_cast<int>(exit_code) != expected_code) {
        std::print(stderr, "ожидался код {}; получен {}\n", expected_code, exit_code);
        return 1;
    }

    return 0;
#else // TODO linux
    std::print(stderr, "Линукс ветка ещё не реализована\n");
    return 1;
#endif
}