typedef unsigned long long uint64_t;
typedef unsigned int uint;
typedef unsigned long size_t;
#define STDIN_FD 0
#define STDOUT_FD 1
const char *source = "typedef unsigned long long uint64_t;\n"
"typedef unsigned int uint;\n"
"typedef unsigned long size_t;\n"
"#define STDIN_FD 0\n"
"#define STDOUT_FD 1\n"
"const char *source = \"@\";\n"
"__attribute__((naked)) void _start(void) {\n"
"    __asm__ volatile (\n"
"        \"mov $0, %rdi\\n\"\n"
"        \"mov $0, %rsi\\n\"\n"
"        \"call main\\n\"\n"
"        \"mov %eax, %edi\\n\"\n"
"        \"mov $231, %rax\\n\"\n"
"        \"syscall\"\n"
"    );\n"
"}\n"
"static int sys_write(int fd, const void *buf, unsigned int count) {\n"
"    int ret;\n"
"    __asm__ volatile (\n"
"        \"mov $1, %%rax\\n\"\n"
"        \"mov %1, %%rdi\\n\"\n"
"        \"mov %2, %%rsi\\n\"\n"
"        \"mov %3, %%rdx\\n\"\n"
"        \"syscall\\n\"\n"
"        \"mov %%eax, %0\\n\"\n"
"        : \"=r\"(ret)\n"
"        : \"r\"((uint64_t)fd), \"r\"(buf), \"r\"((uint64_t)count)\n"
"        : \"rax\", \"rdi\", \"rsi\", \"rdx\"\n"
"    );\n"
"    return ret;\n"
"}\n"
"static int sys_read(int fd, void *buf, unsigned int count) {\n"
"    int ret;\n"
"    __asm__ volatile (\n"
"        \"mov $0, %%rax\\n\"\n"
"        \"mov %1, %%rdi\\n\"\n"
"        \"mov %2, %%rsi\\n\"\n"
"        \"mov %3, %%rdx\\n\"\n"
"        \"syscall\\n\"\n"
"        \"mov %%eax, %0\\n\"\n"
"        : \"=r\"(ret)\n"
"        : \"r\"((uint64_t)fd), \"r\"(buf), \"r\"((uint64_t)count)\n"
"        : \"rax\", \"rdi\", \"rsi\", \"rdx\"\n"
"    );\n"
"    return ret;\n"
"}\n"
"int putchar(int c) {\n"
"    char ch = (char)c;\n"
"    if (sys_write(STDOUT_FD, &ch, 1) != 1) return -1;\n"
"    return c;\n"
"}\n"
"int fputs(const char *s) {\n"
"    size_t len = 0;\n"
"    while (s[len]) len++;\n"
"    int written = sys_write(STDOUT_FD, s, (unsigned int)len);\n"
"    return (written == (int)len) ? (int)len : -1;\n"
"}\n"
"int puts(const char *s) {\n"
"    if (fputs(s) < 0) return -1;\n"
"    if (putchar('\\n') < 0) return -1;\n"
"    return 1;\n"
"}\n"
"size_t strlen(const char *s) {\n"
"    size_t len = 0;\n"
"    while (s[len]) len++;\n"
"    return len;\n"
"}\n"
"int strncmp(const char *a, const char *b, size_t n) {\n"
"    size_t i = 0;\n"
"    for (; i < n; i++) {\n"
"        unsigned char ca = (unsigned char)a[i];\n"
"        unsigned char cb = (unsigned char)b[i];\n"
"        if (ca != cb) return (int)(ca - cb);\n"
"        if (ca == 0) return 0;\n"
"    }\n"
"    return 0;\n"
"}\n"
"size_t strcspn(const char *s, const char *reject) {\n"
"    size_t i = 0;\n"
"    for (; s[i]; i++) {\n"
"        size_t j = 0;\n"
"        while (reject[j]) {\n"
"            if (s[i] == reject[j]) return i;\n"
"            j++;\n"
"        }\n"
"    }\n"
"    return i;\n"
"}\n"
"int atoi(const char *s) {\n"
"    int sign = 1;\n"
"    int val = 0;\n"
"    while (*s == ' ' || *s == '\\t' || *s == '\\n') s++;\n"
"    if (*s == '-') { sign = -1; s++; }\n"
"    else if (*s == '+') s++;\n"
"    while (*s >= '0' && *s <= '9') {\n"
"        val = val * 10 + (*s - '0');\n"
"        s++;\n"
"    }\n"
"    return sign * val;\n"
"}\n"
"int fgets(char *buf, int maxlen) {\n"
"    int i = 0;\n"
"    while (i < maxlen - 1) {\n"
"        char c;\n"
"        int r = sys_read(STDIN_FD, &c, 1);\n"
"        if (r <= 0) break;\n"
"        buf[i++] = c;\n"
"        if (c == '\\n') break;\n"
"    }\n"
"    if (i == 0) return 0;\n"
"    buf[i] = '\\0';\n"
"    return 1;\n"
"}\n"
"int printf(const char *fmt, int val) {\n"
"    int count = 0;\n"
"    while (*fmt) {\n"
"        if (*fmt == '%') {\n"
"            fmt++;\n"
"            if (*fmt == '%') {\n"
"                if (putchar('%') < 0) return -1;\n"
"                count++;\n"
"            } else if (*fmt == 'd') {\n"
"                char buf[20];\n"
"                int i = 0;\n"
"                unsigned int uval;\n"
"                int negative = 0;\n"
"                if (val == 0) {\n"
"                    buf[i++] = '0';\n"
"                } else {\n"
"                    if (val < 0) {\n"
"                        negative = 1;\n"
"                        uval = (unsigned int)(-val);\n"
"                    } else {\n"
"                        uval = (unsigned int)val;\n"
"                    }\n"
"                    while (uval) {\n"
"                        buf[i++] = '0' + (uval % 10);\n"
"                        uval /= 10;\n"
"                    }\n"
"                    if (negative) buf[i++] = '-';\n"
"                }\n"
"                for (int j = i - 1; j >= 0; j--) {\n"
"                    if (putchar(buf[j]) < 0) return -1;\n"
"                    count++;\n"
"                }\n"
"            } else {\n"
"                if (putchar('%') < 0) return -1;\n"
"                if (putchar(*fmt) < 0) return -1;\n"
"                count += 2;\n"
"            }\n"
"            fmt++;\n"
"        } else {\n"
"            if (putchar(*fmt) < 0) return -1;\n"
"            count++;\n"
"            fmt++;\n"
"        }\n"
"    }\n"
"    return count;\n"
"}\n"
"void print(const char *src) {\n"
"    static const char hex[] = \"0123456789abcdef\";\n"
"    unsigned char c;\n"
"    while ((c = (unsigned char)*src++)) {\n"
"        if (c == '\\\\') fputs(\"\\\\\\\\\");\n"
"        else if (c == '\"') fputs(\"\\\\\\\"\");\n"
"        else if (c == '\\n') fputs(\"\\\\n\");\n"
"        else if (c == '\\t') fputs(\"\\\\t\");\n"
"        else if (c >= 32 && c < 127) putchar(c);\n"
"        else {\n"
"            putchar('\\\\');\n"
"            putchar('x');\n"
"            putchar(hex[c >> 4]);\n"
"            putchar(hex[c & 0xF]);\n"
"        }\n"
"    }\n"
"}\n"
"void is_prime(const char *cn) {\n"
"    int n = atoi(cn);\n"
"    if (n <= 1) {\n"
"        puts(\"False\");\n"
"        return;\n"
"    }\n"
"    for (int i = 2; i * i <= n; i++) {\n"
"        if (n % i == 0) {\n"
"            puts(\"False\");\n"
"            return;\n"
"        }\n"
"    }\n"
"    puts(\"True\");\n"
"}\n"
"void fibonacci(const char *cn) {\n"
"    int n = atoi(cn);\n"
"    int a = 0, b = 1, c;\n"
"    if (n == 1) {\n"
"        printf(\"1\\n\", 0);\n"
"        return;\n"
"    }\n"
"    for (int i = 2; i <= n; i++) {\n"
"        c = a + b;\n"
"        a = b;\n"
"        b = c;\n"
"    }\n"
"    printf(\"%d\\n\", b);\n"
"}\n"
"void sum(const char *input) {\n"
"    int count = 0, sum = 0, val = 0;\n"
"    const char *p = input;\n"
"    while (*p >= '0' && *p <= '9') {\n"
"        count = count * 10 + (*p - '0');\n"
"        p++;\n"
"    }\n"
"    while (*p == ' ') p++;\n"
"    for (int i = 0; i < count; i++) {\n"
"        val = 0;\n"
"        while (*p >= '0' && *p <= '9') {\n"
"            val = val * 10 + (*p - '0');\n"
"            p++;\n"
"        }\n"
"        sum += val;\n"
"        while (*p == ' ') p++;\n"
"    }\n"
"    printf(\"%d\\n\", sum);\n"
"}\n"
"size_t strlen_wrapper(const char *s) {return strlen(s);}\n"
"void reverse(const char *input) {\n"
"    int len = (int)strlen_wrapper(input);\n"
"    for (int i = len - 1; i >= 0; i--) putchar(input[i]);\n"
"    putchar('\\n');\n"
"}\n"
"void caesar(const char *input) {\n"
"    int shift = 0;\n"
"    const char *p = input;\n"
"    while (*p >= '0' && *p <= '9') {\n"
"        shift = shift * 10 + (*p - '0');\n"
"        p++;\n"
"    }\n"
"    if (*p == ' ') p++;\n"
"    char c;\n"
"    while ((c = *p++)) {\n"
"        if (c >= 'a' && c <= 'z') {\n"
"            putchar((c - 'a' + shift) % 26 + 'a');\n"
"        } else {\n"
"            putchar(c);\n"
"        }\n"
"    }\n"
"    putchar('\\n');\n"
"}\n"
"size_t strcspn_wrapper(const char *s, const char *reject) {return strcspn(s, reject);}\n"
"int main() {\n"
"    char input[35] = {0};\n"
"    char next_task[15] = {0};\n"
"    if (!fgets(input, sizeof(input))) input[0] = '\\0';\n"
"    else input[strcspn_wrapper(input, \"\\n\")] = '\\0';\n"
"    if (!fgets(next_task, sizeof(next_task))) next_task[0] = '\\0';\n"
"    else next_task[strcspn_wrapper(next_task, \"\\n\")] = '\\0';\n"
"    if (next_task[0] == '\\0') {\n"
"        int i;\n"
"        for (i = 0; i < sizeof(next_task) - 1 && input[i] != '\\0'; i++) {\n"
"            next_task[i] = input[i];\n"
"        }\n"
"        next_task[i] = '\\0';\n"
"    }\n"
"    ^(input);\n"
"    const char *p = source;\n"
"    int sourc = 0, up = 0;\n"
"    while (*p) {\n"
"        if (sourc == 0 && *p == '@') {\n"
"            print(source);\n"
"            sourc++;\n"
"            p++;\n"
"        } else if (*p == '^' && up == 0) {\n"
"            up++;\n"
"            if (strncmp(next_task, \"is_prime\", 8) == 0 || strncmp(next_task, \"fibonacci\", 9) == 0 || strncmp(next_task, \"sum\", 3) == 0 || strncmp(next_task, \"reverse\", 7) == 0 || strncmp(next_task, \"caesar\", 6) == 0) {\n"
"                fputs(next_task);\n"
"            } else {\n"
"                fputs(\"no\");\n"
"            }\n"
"            p++;\n"
"        } else {\n"
"            putchar(*p++);\n"
"        }\n"
"    }\n"
"    return 0;\n"
"}";
__attribute__((naked)) void _start(void) {
    __asm__ volatile (
        "mov $0, %rdi\n"
        "mov $0, %rsi\n"
        "call main\n"
        "mov %eax, %edi\n"
        "mov $231, %rax\n"
        "syscall"
    );
}
static int sys_write(int fd, const void *buf, unsigned int count) {
    int ret;
    __asm__ volatile (
        "mov $1, %%rax\n"
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "mov %3, %%rdx\n"
        "syscall\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint64_t)fd), "r"(buf), "r"((uint64_t)count)
        : "rax", "rdi", "rsi", "rdx"
    );
    return ret;
}
static int sys_read(int fd, void *buf, unsigned int count) {
    int ret;
    __asm__ volatile (
        "mov $0, %%rax\n"
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "mov %3, %%rdx\n"
        "syscall\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint64_t)fd), "r"(buf), "r"((uint64_t)count)
        : "rax", "rdi", "rsi", "rdx"
    );
    return ret;
}
int putchar(int c) {
    char ch = (char)c;
    if (sys_write(STDOUT_FD, &ch, 1) != 1) return -1;
    return c;
}
int fputs(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    int written = sys_write(STDOUT_FD, s, (unsigned int)len);
    return (written == (int)len) ? (int)len : -1;
}
int puts(const char *s) {
    if (fputs(s) < 0) return -1;
    if (putchar('\n') < 0) return -1;
    return 1;
}
size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}
int strncmp(const char *a, const char *b, size_t n) {
    size_t i = 0;
    for (; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb) return (int)(ca - cb);
        if (ca == 0) return 0;
    }
    return 0;
}
size_t strcspn(const char *s, const char *reject) {
    size_t i = 0;
    for (; s[i]; i++) {
        size_t j = 0;
        while (reject[j]) {
            if (s[i] == reject[j]) return i;
            j++;
        }
    }
    return i;
}
int atoi(const char *s) {
    int sign = 1;
    int val = 0;
    while (*s == ' ' || *s == '\t' || *s == '\n') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }
    return sign * val;
}
int fgets(char *buf, int maxlen) {
    int i = 0;
    while (i < maxlen - 1) {
        char c;
        int r = sys_read(STDIN_FD, &c, 1);
        if (r <= 0) break;
        buf[i++] = c;
        if (c == '\n') break;
    }
    if (i == 0) return 0;
    buf[i] = '\0';
    return 1;
}
int printf(const char *fmt, int val) {
    int count = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == '%') {
                if (putchar('%') < 0) return -1;
                count++;
            } else if (*fmt == 'd') {
                char buf[20];
                int i = 0;
                unsigned int uval;
                int negative = 0;
                if (val == 0) {
                    buf[i++] = '0';
                } else {
                    if (val < 0) {
                        negative = 1;
                        uval = (unsigned int)(-val);
                    } else {
                        uval = (unsigned int)val;
                    }
                    while (uval) {
                        buf[i++] = '0' + (uval % 10);
                        uval /= 10;
                    }
                    if (negative) buf[i++] = '-';
                }
                for (int j = i - 1; j >= 0; j--) {
                    if (putchar(buf[j]) < 0) return -1;
                    count++;
                }
            } else {
                if (putchar('%') < 0) return -1;
                if (putchar(*fmt) < 0) return -1;
                count += 2;
            }
            fmt++;
        } else {
            if (putchar(*fmt) < 0) return -1;
            count++;
            fmt++;
        }
    }
    return count;
}
void print(const char *src) {
    static const char hex[] = "0123456789abcdef";
    unsigned char c;
    while ((c = (unsigned char)*src++)) {
        if (c == '\\') fputs("\\\\");
        else if (c == '"') fputs("\\\"");
        else if (c == '\n') fputs("\\n");
        else if (c == '\t') fputs("\\t");
        else if (c >= 32 && c < 127) putchar(c);
        else {
            putchar('\\');
            putchar('x');
            putchar(hex[c >> 4]);
            putchar(hex[c & 0xF]);
        }
    }
}
void is_prime(const char *cn) {
    int n = atoi(cn);
    if (n <= 1) {
        puts("False");
        return;
    }
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            puts("False");
            return;
        }
    }
    puts("True");
}
void fibonacci(const char *cn) {
    int n = atoi(cn);
    int a = 0, b = 1, c;
    if (n == 1) {
        printf("1\n", 0);
        return;
    }
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    printf("%d\n", b);
}
void sum(const char *input) {
    int count = 0, sum = 0, val = 0;
    const char *p = input;
    while (*p >= '0' && *p <= '9') {
        count = count * 10 + (*p - '0');
        p++;
    }
    while (*p == ' ') p++;
    for (int i = 0; i < count; i++) {
        val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            p++;
        }
        sum += val;
        while (*p == ' ') p++;
    }
    printf("%d\n", sum);
}
size_t strlen_wrapper(const char *s) {return strlen(s);}
void reverse(const char *input) {
    int len = (int)strlen_wrapper(input);
    for (int i = len - 1; i >= 0; i--) putchar(input[i]);
    putchar('\n');
}
void caesar(const char *input) {
    int shift = 0;
    const char *p = input;
    while (*p >= '0' && *p <= '9') {
        shift = shift * 10 + (*p - '0');
        p++;
    }
    if (*p == ' ') p++;
    char c;
    while ((c = *p++)) {
        if (c >= 'a' && c <= 'z') {
            putchar((c - 'a' + shift) % 26 + 'a');
        } else {
            putchar(c);
        }
    }
    putchar('\n');
}
size_t strcspn_wrapper(const char *s, const char *reject) {return strcspn(s, reject);}
int main() {
    char input[35] = {0};
    char next_task[15] = {0};
    if (!fgets(input, sizeof(input))) input[0] = '\0';
    else input[strcspn_wrapper(input, "\n")] = '\0';
    if (!fgets(next_task, sizeof(next_task))) next_task[0] = '\0';
    else next_task[strcspn_wrapper(next_task, "\n")] = '\0';
    if (next_task[0] == '\0') {
        int i;
        for (i = 0; i < sizeof(next_task) - 1 && input[i] != '\0'; i++) {
            next_task[i] = input[i];
        }
        next_task[i] = '\0';
    }
    no(input);
    const char *p = source;
    int sourc = 0, up = 0;
    while (*p) {
        if (sourc == 0 && *p == '@') {
            print(source);
            sourc++;
            p++;
        } else if (*p == '^' && up == 0) {
            up++;
            if (strncmp(next_task, "is_prime", 8) == 0 || strncmp(next_task, "fibonacci", 9) == 0 || strncmp(next_task, "sum", 3) == 0 || strncmp(next_task, "reverse", 7) == 0 || strncmp(next_task, "caesar", 6) == 0) {
                fputs(next_task);
            } else {
                fputs("no");
            }
            p++;
        } else {
            putchar(*p++);
        }
    }
    return 0;
}