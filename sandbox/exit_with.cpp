#include <charconv>
#include <cstring>
#include <system_error>

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 2;
    int code = 0;
    const char* first = argv[1];
    const char* last = argv[1] + std::strlen(argv[1]);
    if (std::from_chars(first, last, code).ec != std::errc{})
        return 2;
    return code;
}
