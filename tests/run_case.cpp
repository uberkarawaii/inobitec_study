#include <print>
#include <string>
#include <string_view>
#include <system_error>

#ifdef _WIN32
#include <windows.h>

#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>

#endif

#ifdef _WIN32
// печать ошибки - передаётся строка со смыслом проблемы и имя файла, с которым была проблема
void print_win_error(std::string_view context, std::string_view target) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "Can not {} {}: {} (code {})\n", context, target, ec.message(), current_err);
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

#else
// класс-обёртка для файловго дескриптора
class posix_fd {
    int fd_ = -1;

public:
    posix_fd(int fd = -1) : fd_(fd) {}
    ~posix_fd() {
        if (fd_ >= 0)
            close(fd_);
    }
    posix_fd(const posix_fd&) = delete;
    posix_fd& operator=(const posix_fd&) = delete;
    int get() const { return fd_; }
    explicit operator bool() const { return fd_ >= 0; }
};

// вынесение печати ошибки
void print_errno_error(std::string_view context, std::string_view target) {
    // системные вызовы POSIX при ошибках будут выставлять errno - это макрос в рамках потока
    // сохранение errno в локальной переменной, чтобы поймать состояние, когда он не был ничем перезаписан
    int current_err = errno;
    std::error_code ec(current_err, std::generic_category());
    std::print(stderr, "Can not {} {}: {} (code {})\n", context, target, ec.message(), current_err);
}

// вынесение работы с файловым дескриптором и возврат его обёрки в posix_fd при удаче
posix_fd open_io_file(const char* path, int flags, int mode, std::string_view context) {
    // flags - флаги того, что можно произоводить с этим файлом - чтение / запись / и т.д.
    // mode - права файла
    int fd = open(path, flags, mode);

    if (fd == -1) {
        print_errno_error(context, path);
        return posix_fd{};
    }

    return posix_fd{fd};
}
// вынесение кода "команда не найдена"
constexpr int kExecFailed = 127;

#endif

// run_case <ожидаемый_код> <файл_входа> <файл_stdout> <файл_stderr> -- <exe> [арг...]
int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::print(stderr,
                   "6 or more arg-s are needed: <expect_code> <file_input> <file_stdout> <file_stderr> -- <exe>. "
                   "Received: {}\n",
                   argc - 1);
        return 1;
    }

    if (std::string(argv[5]) != "--") {
        std::print(stderr, "Separator \"--\" is needed on 5th position, but {} was received instead\n", argv[5]);
        return 1;
    }

    int expected_code = 0;

    try {
        expected_code = std::stoi(argv[1]);
    } catch (const std::invalid_argument&) {
        std::print(stderr, "Expected exit-code is integer. Received: {}\n", argv[1]);
        return 1;
    } catch (const std::out_of_range&) {
        std::print(stderr, "Expected exit-code is integer and received number({}) - is beyond int.\n", argv[1]);
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
        win_handle in = open_io_file(argv[2], GENERIC_READ, OPEN_EXISTING, "open input file");
        if (!in)
            return 1;

        win_handle out = open_io_file(argv[3], GENERIC_WRITE, CREATE_ALWAYS, "create output file");
        if (!out)
            return 1;

        win_handle err = open_io_file(argv[4], GENERIC_WRITE, CREATE_ALWAYS, "create error file");
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
            print_win_error("run", argv[6]);
            return 1;
        }
    } // здесь происходит удаление in/out/err через деструкторы для run_case

    win_handle hproc{pi.hProcess};
    win_handle hthread{pi.hThread};

    DWORD exit_code = 0;
    // run_case блокируется и ждёт, пока не завершится процесс pi.hProcess
    // ждёт бесконечно, но ловит сбои ожидания и если они есть - бросит ошибку и завершится
    if (WaitForSingleObject(hproc.get(), INFINITE) != WAIT_OBJECT_0) {
        print_win_error("reach the completion of", argv[6]);
        return 1;
    }
    // GetExitCodeProcess - снять код завершения процесса pi.hProcess и положить в exit_code
    // проверка, если вдруг не удалось снять взять код завершения
    if (!GetExitCodeProcess(hproc.get(), &exit_code)) {
        print_win_error("get the execution code", argv[6]);
        return 1;
    }

    // сравнение снятого кода процесса и ожидаемого
    if (static_cast<int>(exit_code) != expected_code) {
        std::print(stderr, "expected code is {}; received code is {}\n", expected_code, exit_code);
        return 1;
    }

    return 0;
#else
    // linux
    // fork() положит в pid идентификатор процесса (пока его нет, и дефолтно -1)
    // нужно для разделения действий: -1 - что сделать при fail; 0 - что сделать ребёнку
    // положит. число - что сделать родителю
    pid_t pid = -1;
    {
        // создание файловых дескрипторов в родителе, чтобы потом передать их ребёнку
        posix_fd in = open_io_file(argv[2], O_RDONLY | O_CLOEXEC, 0, "open input file");
        if (!in)
            return 1;

        posix_fd out = open_io_file(argv[3], O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644, "create output file");
        if (!out)
            return 1;

        posix_fd err = open_io_file(argv[4], O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644, "create error file");
        if (!err)
            return 1;

        pid = fork(); // копия родительского процесса run_case
        // действия если не получилось создать дочерний процесс (pid=-1)
        if (pid < 0) {
            print_errno_error("create a child process to run", argv[6]);
            return 1;
        }
        // такой pid будет у ребёнка. в этом блоке - логика и действия ребёнка
        if (pid == 0) {
            // ребёнок: перенаправить 0/1/2 на файлы
            if (dup2(in.get(), STDIN_FILENO) < 0 || dup2(out.get(), STDOUT_FILENO) < 0 ||
                dup2(err.get(), STDERR_FILENO) < 0) {
                int errno_copy = errno;
                std::print(stderr, "Can not redirect streams: {}\n",
                           std::error_code(errno_copy, std::generic_category()).message());
                _exit(kExecFailed);
            }
            // замена кода run_case на код main (или что передали) в ребёнке
            // перенаправленные потоки остаются как и были
            execv(argv[6], &argv[6]);

            // поймать ошибку execv
            int errno_copy = errno;
            std::print(stderr, "Can not run {}: {}\n", argv[6],
                       std::error_code(errno_copy, std::generic_category()).message());
            // завершить процесс ребёнка
            _exit(kExecFailed);

            // у процесса ребёнка - оконачние либо в execv либо в _exit; потому за этот скоуп не выйдет
            // и не должен, т.к. далее - для родителя
        }
    } // тут 3шт fd run_case-а уничтожатся деструкторами обёртки, т.к. выход из спец. скоупа

    // контейнер для exit-кода ребёнка
    int status = 0;
    // для ожидния завершения процесса ребёнка
    // pid - ожидай окончания процесса с этим id
    // status - положи код по ссылке
    // 0 - жди именно завершения, а остановки или иного
    // при waitpid < 0 не удалось дождаться именно завершения процесса с этим pid. вывод причины
    // упускается случай errno == EINTR. по идее, никто не должен послать сигналы тест. обвязке
    if (waitpid(pid, &status, 0) < 0) {
        print_errno_error("wait for the process of", argv[6]);
        return 1;
    }

    // развилка того, как завершился ребёнок: штатно или внештатно
    // WIFEXITED - макрос, даст true, если ребёнок завершился штатно, а не от sigkill/sigsegv/etc
    if (WIFEXITED(status)) {
        // WEXITSTATUS - из всех битов информации извлекается именно код возврата
        int exit_code = WEXITSTATUS(status);
        // когда реальный код и ожидаемый не совпали
        if (exit_code != expected_code) {
            std::print(stderr, "expected code is {}; received code is {}\n", expected_code, exit_code);
            return 1;
        }
    }
    // процесс завершился не сам, а с опред. сигналом завершения
    // в случае остановки когда asan нашёл ошибку / при невозможном обращении к памяти / ...
    else if (WIFSIGNALED(status)) {
        std::print(stderr, "expected code is {}; process was terminated by signal {}\n", expected_code,
                   WTERMSIG(status));
        return 1;
    }
    // если не удалось отловить через WIFSIGNALED. тут завершение ребёнка также внештатное
    // надо завершиться с ошибкой
    else {
        std::print(stderr, "expected code is {}; process ended with unexpected status\n", expected_code);
        return 1;
    }

    return 0;
#endif
}