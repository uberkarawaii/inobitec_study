#include <cstdio>

int main() {
    char buf[1024];
    for (char& c : buf)
        c = 'x';
    for (int i = 0; i < 1024; ++i) // 1024 * 1024 = 1 MiB
        if (std::fwrite(buf, 1, sizeof buf, stdout) != sizeof buf)
            return 1;
    return 0;
}
