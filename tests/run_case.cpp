#include <cstdlib>
#include <iostream>
#include <print>
#include <string>

// по совету deepseek синтаксис такой: run_case <команда> <ожидаемый код>
// <команда> - вводимое значение (echo) | prog_name.exe > null 2> null
// argv[1]: <команда>
// argv[2]: <ожидаемый код>
int main(int argc, char* argv[]) {

    // проверка что введено 2 аргумента, помимо имени программы run_case
    if (argc != 3) {
        std::print("Ожидаллось 2 аргумента: <команда> <ожидаемый код>. Получено {} аргументов\n", argc - 1);
        return 1;
    }

    int expected_code = std::stoi(argv[2]);
    // std::system(command) позовёт, в случае винды, cmd.exe со строкой command
    // std::system вернёт код, с которым завершилась программа
    int real_code = std::system(argv[1]);

    if (real_code != expected_code) {
        std::print(stderr, "Ожидался код: {}. Был получен код: {}\n", expected_code, real_code);
        return 1;
    }

    return 0;
}