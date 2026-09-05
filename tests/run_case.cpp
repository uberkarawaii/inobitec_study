#include <print>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <system_error>
#endif

// run_case <ожидаемый_код> <файл_входа> <файл_stdout> <файл_stderr> -- <exe> [арг...]
int main(int argc, char* argv[]){
if (argc < 7){
std::print(stderr, "Ожидалось не менее 6 аргументов: <expect_code> <file_input> <file_stdout> <file_stderr> -- <exe>. Получено: {}\n", argc - 1);
return 1;
}

if (std::string(argv[5]) != "--"){
std::print(stderr, "Ожидался разделитель \"--\" на 5-й позиции, получено: {}\n", argv[5]);
return 1;
}

int expected_code = 0;

try{
expected_code = std::stoi(argv[1]);
} catch (const std::invalid_argument&) {
std::print(stderr, "Ожидаемый exit-code - целое число. Получено: {}\n", argv[1]);
return 1;
} catch (const std::out_of_range&) {
std::print(stderr, "Ожидается exit-code типа int, а полученное число({}) - за пределами int.\n", argv[1]);
return 1;
}

#ifdef _WIN32
// hStdInput/hStdOutput/hStdError из STARTUPINFO становятся потоками 0/1/2 дочернего процесса

// дескриптор безопасности для объектов, которые созданы внутри программы(здесь - CreateFileA)
SECURITY_ATTRIBUTES sa{};
sa.nLength = sizeof(sa);
sa.bInheritHandle = true; // хэндл может наследоваться дочерним процессом
sa.lpSecurityDescriptor = nullptr; 

// TODO вынести дублирование 3 блоков
// если in будет неудачным, то out не закрывается и тд по цепочке 
// ОС вроде всё уберёт при завершении начального процесса
HANDLE in = CreateFileA(argv[2], GENERIC_READ, FILE_SHARE_READ,
                        &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

if (in == INVALID_HANDLE_VALUE) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось открыть файл входа {}: {} (код {})\n", argv[2], ec.message(), current_err);
    return 1;
}

HANDLE out = CreateFileA(argv[3], GENERIC_WRITE, FILE_SHARE_READ,
                         &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


if (out == INVALID_HANDLE_VALUE) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось создать файл выхода {}: {} (код {})\n", argv[3], ec.message(), current_err);
    return 1;
}

HANDLE err = CreateFileA(argv[4], GENERIC_WRITE, FILE_SHARE_READ,
                         &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

if (err == INVALID_HANDLE_VALUE) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось создать файл ошибки {}: {} (код {})\n", argv[4], ec.message(), current_err);
    return 1;
}

STARTUPINFOA si{};
si.cb = sizeof(si);
si.dwFlags = STARTF_USESTDHANDLES;
si.hStdInput = in;
si.hStdOutput = out;
si.hStdError = err;

// обнуление чтобы не осталось мусора в полях
PROCESS_INFORMATION pi{};

std::string cmdline = "\"" + std::string(argv[6]) + "\"";
for (int i = 7; i < argc; ++i) {
    cmdline += ' ';
    cmdline += argv[i];
}

// был отказ от CreateProcessW в CreateProcessA пользу чтобы не конверт. в char* -> wchar_t
// плохо: не-ASCII пути зависят от кодировки. но у меня нет кириллицы в путях 
BOOL ok = CreateProcessA(argv[6], cmdline.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

if (!ok) {
    DWORD current_err = GetLastError();
    // <system_error>: переводит код Windows в текст
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось запустить {}: {} (код {})\n", argv[6], ec.message(), current_err);
    return 1;
}

CloseHandle(in); CloseHandle(out); CloseHandle(err);

DWORD exit_code = 0;
if (WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_OBJECT_0) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось дождаться завершения {}: {} (код {})\n", argv[6], ec.message(), current_err);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
}
if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
    DWORD current_err = GetLastError();
    std::error_code ec(static_cast<int>(current_err), std::system_category());
    std::print(stderr, "не удалось получить код завершения {}: {} (код {})\n", argv[6],  ec.message(), current_err);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
}
CloseHandle(pi.hProcess);
CloseHandle(pi.hThread);

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