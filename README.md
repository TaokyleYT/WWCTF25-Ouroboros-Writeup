# Ouroboros - Writeup

## Files related to solving the challenge are in root folder

## Please open issue should you have any questions. It will be added to the respective Q&A section

**Author: Taokyle - THEM?!**

OS: this is the only chal I solved cuz other chals are either too hard for me or too easy that my teammates alr solved it before me :skull:

## Situation

Ouroboros

author: yun

Is your code a true ouroboros?

`nc chal.wwctf.com 6000`

Attachments:\
[chall.py](./chall.py)\
[Dockerfile](./Dockerfile)\
[nsjail.cfg](./nsjail.cfg)\
[wrapper.sh](./wrapper.sh)

## The Beginning

First, since this is a **docker** environment, lets take a look at `Dockerfile` to see what the environment is like, and where the flag is.

```dockerfile
FROM python:alpine
RUN apk update && \
    apk add --no-cache socat gcc nsjail
RUN passwd -l root \
    && adduser -S -D -H -u 1000 user
WORKDIR /home/user
ENV FLAG=wwf{this_is_not_a_flag}
COPY chall.py wrapper.sh /home/user/
RUN chmod -R 555 /home/user
USER user
CMD socat -dd TCP4-LISTEN:1337,fork,reuseaddr SYSTEM:/home/user/wrapper.sh
```

Ok it seems that there is a flag in `FLAG` env variable. Let's also take a look at what `wrapper.sh` does to find out the entry point

```bash
#!/bin/sh
python3 -u /home/user/chall.py
```

Interesting, it just **runs chall.py**. Let's go analyze `chall.py`, our :star: in this chal

## The Beginning - checkpoint Q&A

Q - What does the `-u` flag in `wrapper.sh` do? Does it affect how the script runs?\
A - `-u` is just used to run the python script with **unbuffered output**, this ensures the output from the chal is printed **immediately**.

## chall.py

<details open>
  <summary><b>Click to open/close the full <a href="./chall.py">chall.py</a> code</b></summary>

```python
import os
import random
import string
import sys
import tempfile
import hashlib
import subprocess
import shutil

BASE = os.path.join(tempfile.gettempdir(), 'sandbox')

sys.stdout.write('''\x1b[31;1m
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⣀⣀⣀⣀⣄⣀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣴⡶⢿⣟⡛⣿⢉⣿⠛⢿⣯⡈⠙⣿⣦⡀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⣠⡾⠻⣧⣬⣿⣿⣿⣿⣿⡟⠉⣠⣾⣿⠿⠿⠿⢿⣿⣦⠀⠀⠀
⠀⠀⠀⠀⣠⣾⡋⣻⣾⣿⣿⣿⠿⠟⠛⠛⠛⠀⢻⣿⡇⢀⣴⡶⡄⠈⠛⠀⠀⠀
⠀⠀⠀⣸⣿⣉⣿⣿⣿⡿⠋⠀⠀⠀⠀⠀⠀⠀⠈⢿⣇⠈⢿⣤⡿⣦⠀⠀⠀⠀
⠀⠀⢰⣿⣉⣿⣿⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠦⠀⢻⣦⠾⣆⠀⠀⠀
⠀⠀⣾⣏⣿⣿⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⡶⢾⡀⠀⠀
⠀⠀⣿⠉⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣧⣼⡇⠀⠀
⠀⠀⣿⡛⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣿⣧⣼⡇⠀⠀
⠀⠀⠸⡿⢻⣿⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⣿⣥⣽⠁⠀⠀
⠀⠀⠀⢻⡟⢙⣿⣿⣿⣦⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⣿⣧⣸⡏⠀⠀⠀
⠀⠀⠀⠀⠻⣿⡋⣻⣿⣿⣿⣦⣤⣀⣀⣀⣀⣀⣠⣴⣿⣿⢿⣥⣼⠟⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠈⠻⣯⣤⣿⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⠛⣷⣴⡿⠋⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠈⠙⠛⠾⣧⣼⣟⣉⣿⣉⣻⣧⡿⠟⠋⠁⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
\x1b[0m''')


def prompt_code():
    sys.stdout.write('Welcome to the ouroboros challenge, give me your code: (empty line to end)\n')
    sys.stdout.flush()
    code = ""
    w = input()
    while w:
        code += w + '\n'
        w = input()
    return code


def run_code(code, stdin):
    if not code.strip():
        sys.stderr.write('Your code was so short it couldn\'t find its own tail.\n')
        sys.stdout.flush()
        return
    if len(code) > 99999:
        sys.stderr.write('Your code was so long it couldn\'t find its own tail.\n')
        sys.stdout.flush()
        return
    h = hashlib.sha256(code.encode()).hexdigest()
    jobdir = os.path.join(BASE, h)
    os.makedirs(jobdir, exist_ok=True)
    sandbox_root = os.path.join(jobdir, 'root')
    os.makedirs(sandbox_root, exist_ok=True)

    src_path = os.path.join(jobdir, 'main.c')
    with open(src_path, 'w') as f:
        f.write(code)

    bin_path = os.path.join(sandbox_root, 'main')
    proc = subprocess.run([
        'gcc', '-O0', '-std=c99', '-fno-common', '-pipe', '-static', '-nostdlib', '-o', bin_path,
        src_path,
    ], capture_output=True)
    os.remove(src_path)

    if proc.returncode != 0:
        sys.stdout.write('Your code was so bad it couldn\'t find its own tail.\n')
        sys.stdout.flush()
        print(proc.stderr.decode(), file=sys.stderr)
        shutil.rmtree(jobdir)
        return
    nsjail_cmd = [
        "nsjail",
        "--mode", "o",
        "--user", "99999",
        "--group", "99999",
        "--disable_proc",
        "--disable_clone_newnet",
        "--disable_clone_newipc",
        "--disable_clone_newuts",
        "--disable_clone_newpid",
        "--rlimit_as", "64",
        "--rlimit_cpu", "1",
        "--rlimit_fsize", "1",
        "--rlimit_nofile", "1",
        "--rlimit_nproc", "1",
        "--chroot", sandbox_root,
        "--bindmount_ro", f"{sandbox_root}:/",
        "--seccomp_string", "ALLOW { read, write, close, execve, exit_group } DEFAULT KILL_PROCESS",
        "--",
        "/main",
    ]
    proc = subprocess.run(
        nsjail_cmd,
        input=stdin.encode(),
        capture_output=True,
        timeout=1,
    )
    out = proc.stdout.decode().strip()
    shutil.rmtree(jobdir)
    if proc.returncode != 0:
        sys.stdout.write('Your code was so bad it died.\n')
        sys.stdout.flush()
        return
    return out


def _reverse():
    s = ''.join(random.choices(string.ascii_lowercase, k=random.randint(3, 10)))
    return s, s[::-1]


def _sum():
    nums = [random.randint(0, 20) for _ in range(random.randint(3, 7))]
    return f'{len(nums)} {" ".join(map(str, nums))}', sum(nums)


def _is_prime():
    n = random.randint(2, 100)
    prime = n > 1 and all(n % i for i in range(2, int(n**0.5) + 1))
    return n, prime


def _fibonacci():
    n = random.randint(1, 15)
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return n, b


def _caesar():
    shift = random.randint(1, 25)
    text = ''.join(random.choices(string.ascii_lowercase, k=random.randint(5, 12)))
    encoded = ''.join(
        chr((ord(c) - 97 + shift) % 26 + 97) if 'a' <= c <= 'z' else c
        for c in text
    )
    return f'{shift} {text}', encoded


def task():
    fn = random.choice([_reverse, _sum, _is_prime, _fibonacci, _caesar])
    return fn.__name__[1:], *fn()


def main():
    original = code = prompt_code()
    tasks = [task() for _ in range(69)]
    code = run_code(code, tasks[0][0])
    if code is None:
        return
    for i, (_, input_data, expected_output) in enumerate(tasks):
        sys.stdout.write(f'task {i+1}/{len(tasks)}\n')
        sys.stdout.flush()
        out = run_code(code, str(input_data) + '\n'+(tasks[i+1][0] if i + 1 < len(tasks) else ''))
        if out is None:
            return
        ans = out[:out.find('\n')] if '\n' in out else out
        code = out[out.find('\n')+1:] if '\n' in out else ''
        if ans != str(expected_output):
            sys.stdout.write('Your code was so bad it couldn\'t find its own tail.\n')
            sys.stdout.flush()
            return
    if original.strip() != code.strip():
        sys.stdout.write('Your code was so bad it couldn\'t find its own tail.\n')
        print(code)
        print("\n\n")
        print(original)
        return
    else:
        sys.stdout.write('Your code was a true ouroboros!\n')
        sys.stdout.write(os.environ.get('FLAG', 'flag{this_is_a_fake_flag}') + '\n')
    sys.stdout.flush()


if __name__ == '__main__':
    main()
```

</details>

<br>

We can split the program into 3 parts

### The tasks

```python
def _reverse():
    s = ''.join(random.choices(string.ascii_lowercase, k=random.randint(3, 10)))
    return s, s[::-1]


def _sum():
    nums = [random.randint(0, 20) for _ in range(random.randint(3, 7))]
    return f'{len(nums)} {" ".join(map(str, nums))}', sum(nums)


def _is_prime():
    n = random.randint(2, 100)
    prime = n > 1 and all(n % i for i in range(2, int(n**0.5) + 1))
    return n, prime


def _fibonacci():
    n = random.randint(1, 15)
    a, b = 0, 1
    for _ in range(n - 1):
        a, b = b, a + b
    return n, b


def _caesar():
    shift = random.randint(1, 25)
    text = ''.join(random.choices(string.ascii_lowercase, k=random.randint(5, 12)))
    encoded = ''.join(
        chr((ord(c) - 97 + shift) % 26 + 97) if 'a' <= c <= 'z' else c
        for c in text
    )
    return f'{shift} {text}', encoded


def task():
    fn = random.choice([_reverse, _sum, _is_prime, _fibonacci, _caesar])
    return fn.__name__[1:], *fn()
```

There are 5 tasks in total, each task is a function that returns a tuple with 2 elements. The first is the inputs that you need to find out the output, the second is the expected output.

They are

**`reverse`**

* input a string consisting of 3 to 10 lowercase letters, output the reversed string.

**`sum`**

* input 3 to 7 integers from 0 to 20 split with a space, output their sum

* *special: the input are not just the integers, the first element is how many integers are there, the following are the integers*

**`is_prime`**

* input an integer from 2 to 100, output whether it is a prime

* *special: the output need to be "True" or "False", not just 1 and 0*

**`fibonacci`**

* input an integer from 1 to 15, output the n-th fibonacci number, where n is the input

**`caesar`**

* input a shift amount and a string consisting of 5 to 12 lowercase letters, output the caesar shifted cipher

There is also a task distribute function `task()`, which will **return** a random task's **name**, and the **input** and **expected output** of that task.

### The runner

```python
def run_code(code, stdin):
    if not code.strip():
        sys.stderr.write('Your code was so short it couldn\'t find its own tail.\n')
        sys.stdout.flush()
        return
    if len(code) > 99999:
        sys.stderr.write('Your code was so long it couldn\'t find its own tail.\n')
        sys.stdout.flush()
        return
    h = hashlib.sha256(code.encode()).hexdigest()
    jobdir = os.path.join(BASE, h)
    os.makedirs(jobdir, exist_ok=True)
    sandbox_root = os.path.join(jobdir, 'root')
    os.makedirs(sandbox_root, exist_ok=True)

    src_path = os.path.join(jobdir, 'main.c')
    with open(src_path, 'w') as f:
        f.write(code)

    bin_path = os.path.join(sandbox_root, 'main')
    proc = subprocess.run([
        'gcc', '-O0', '-std=c99', '-fno-common', '-pipe', '-static', '-nostdlib', '-o', bin_path,
        src_path,
    ], capture_output=True)
    os.remove(src_path)

    if proc.returncode != 0:
        sys.stdout.write('Your code was so bad it couldn\'t find its own tail.\n')
        sys.stdout.flush()
        print(proc.stderr.decode(), file=sys.stderr)
        shutil.rmtree(jobdir)
        return
    nsjail_cmd = [
        "nsjail",
        "--mode", "o",
        "--user", "99999",
        "--group", "99999",
        "--disable_proc",
        "--disable_clone_newnet",
        "--disable_clone_newipc",
        "--disable_clone_newuts",
        "--disable_clone_newpid",
        "--rlimit_as", "64",
        "--rlimit_cpu", "1",
        "--rlimit_fsize", "1",
        "--rlimit_nofile", "1",
        "--rlimit_nproc", "1",
        "--chroot", sandbox_root,
        "--bindmount_ro", f"{sandbox_root}:/",
        "--seccomp_string", "ALLOW { read, write, close, execve, exit_group } DEFAULT KILL_PROCESS",
        "--",
        "/main",
    ]
    proc = subprocess.run(
        nsjail_cmd,
        input=stdin.encode(),
        capture_output=True,
        timeout=1,
    )
    out = proc.stdout.decode().strip()
    shutil.rmtree(jobdir)
    if proc.returncode != 0:
        sys.stdout.write('Your code was so bad it died.\n')
        sys.stdout.flush()
        return
    return out
```

First, it checks **how long** the input c code is. It cannot be **empty** <sub>(but you can't run anything if theres no code so we ignore lol)</sub>, and it cannot contain more than **99999 bytes** <sub>(thats alot)</sub>

It then creates a directory for the job, named from the hash of the code (so that the **same code** used the **same directory**), and creates a subdirectory for the sandbox.

It writes the input c code into `main.c` in that sandbox, **compiles** the code using **gcc**, and executes it using **nsjail**. Finally, if the code ran successfully, it returns the output.

### The entry point

```python
def prompt_code():
    sys.stdout.write('Welcome to the ouroboros challenge, give me your code: (empty line to end)\n')
    sys.stdout.flush()
    code = ""
    w = input()
    while w:
        code += w + '\n'
        w = input()
    return code

def main():
    original = code = prompt_code()
    tasks = [task() for _ in range(69)]
    code = run_code(code, tasks[0][0])
    if code is None:
        return
    for i, (_, input_data, expected_output) in enumerate(tasks):
        sys.stdout.write(f'task {i+1}/{len(tasks)}\n')
        sys.stdout.flush()
        out = run_code(code, str(input_data) + '\n'+(tasks[i+1][0] if i + 1 < len(tasks) else ''))
        if out is None:
            return
        ans = out[:out.find('\n')] if '\n' in out else out
        code = out[out.find('\n')+1:] if '\n' in out else ''
        if ans != str(expected_output):
            sys.stdout.write('Your code was so bad it couldn\'t find its own tail.\n')
            sys.stdout.flush()
            return
    if original.strip() != code.strip():
        sys.stdout.write('Your code was so bad it couldn\'t find its own tail.\n')
        print(code)
        print("\n\n")
        print(original)
        return
    else:
        sys.stdout.write('Your code was a true ouroboros!\n')
        sys.stdout.write(os.environ.get('FLAG', 'flag{this_is_a_fake_flag}') + '\n')
    sys.stdout.flush()
```

When the code runs, it first **prompts** us to **paste** in a c code (can be multilined), and submits upon us entering an empty line.

It generates **69 tasks** (69 nice wwwwww), and runs the code on each task.

The input for the code would be the **input** for the **current** task, newline, and then the task **name** for the **next** task (NOT the current task)

The output of the code should be the **output** for the **current** task, newline, and then the entire c code that would run on the next task.

**Special i/o**

* The **first** input that the code will receive would be **just** the task name for the next task, the current task input will be **missing** because technically that is a startup instead of a task

* The **output** c code for the **last** task should be **identical** to the original c code (aka the code you pasted in the beginning)

Then, if all 69 tasks are solved correctly, and the final output c code is identical to the original c code, it will print the flag.

## chall.py - checkpoint Q&A

Q - What does ouroboros have to do with this chall?\
A - Ouroboros is an ancient symbol depicting a snake biting it's own tail, just like a loop.\
This chal requires your code to be like an ouroboros, it survives (do tasks), and then eat itself (prints an exact copy of itself out, with only necesserary changes)

## Initial ideas

The first approach I thought of was to try a way to **read** and **extract** the **FLAG env** variable and output it, skipping the code.\
However by finding all places in the chall script where message related to the code might get printed out, I found out that it can only happen either **after solving** all 69 tasks, **or** stuff it in **stderr** and trigger a **compilation error** when compiling the next code.\
For the first exitpoint, since we've already got that far we might as well make it solve the entire challenge and get the flag from a legit print :smirk:.\
For the second exitpoint, From the nsjail run script we find this argument: `"--rlimit_nofile", "1",`\
Which means that we do **not** have access to **stderr** channel, so this is not an option.

To be honest, writing a c program which solves the challenge is easier than whatever other approaches in extracting the flag, so I headed straight to craft the skeleton of the payload.

### The skeleton

```c
const char *source = "copy of the entire code";
void is_prime(const char *input) {
    //solves for is_prime
}
void fibonacci(const char *input) {
    //solves for fibonacci
}
void sum(const char *input) {
    //solves for sum
}
void reverse(const char *input) {
    //solves for reverse
}
void caesar(const char *input) {
    //solves for caesar
}
int main() {
    //seperates the input into next task's name and current task input
    //runs the current task (hardcoded by the last one)
    //prints the source, replace current task function with next one
}
```

## Initial ideas - checkpoint Q&A

Q - Did you use AI to craft the payload?\
A - No :skull: me poor no money to use AI

## nostdlib

While I was completing the skeleton code, I realized that we have to add 2 placeholders in the code instead of just the task function, because the source need to include itself, which includes itself, which..... and that would be a forever loop.\
The fix is to also add a placeholder for the source, and fill the content with an escaped version of itself. For that I have implemented a function `print`

Here is the first iteration of the code.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char *source = "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\nconst char *source = \"%SOURCE%\";\nvoid print(const char *src) {\n    static const char hex[] = \"0123456789abcdef\";\n    unsigned char c;\n    while ((c = (unsigned char)*src++)) {\n        if (c == '\\\\') fputs(\"\\\\\\\\\", stdout);\n        else if (c == '\"') fputs(\"\\\\\\\"\", stdout);\n        else if (c == '\\n') fputs(\"\\\\n\", stdout);\n        else if (c == '\\t') fputs(\"\\\\t\", stdout);\n        else if (c >= 32 && c < 127) putchar(c);\n        else {\n            putchar('\\\\');\n            putchar('x');\n            putchar(hex[c >> 4]);\n            putchar(hex[c & 0xF]);\n        }\n    }\n}\nvoid is_prime(const char *cn) {\n    int n = atoi(cn);\n    if (n <= 1) {\n        puts(\"False\");\n        return;\n    }\n    for (int i = 2; i*i <= n; i++) {\n        if (n % i == 0) {\n            puts(\"False\");\n            return;\n        }\n    }\n    puts(\"True\");\n}\nvoid fibonacci(const char *cn) {\n    int n = atoi(cn);\n    int a = 0, b = 1, c;\n    if (n == 1) {\n        printf(\"1\\n\");\n        return;\n    }\n    for (int i = 2; i <= n; i++) {\n        c = a + b;\n        a = b; \n        b = c;\n    }\n    printf(\"%d\\n\", b);\n}\nvoid sum(const char *input) {\n    int count, sum = 0, val;\n    const char *p = input;\n    count = 0;\n    while (*p >= '0' && *p <= '9') {\n        count = count*10 + (*p - '0');\n        p++;\n    }\n    while (*p == ' ') p++;\n    for (int i=0;i<count;i++) {\n        val = 0;\n        while (*p >= '0' && *p <= '9') {\n            val = val*10 + (*p - '0');\n            p++;\n        }\n        sum += val;\n        while (*p == ' ') p++;\n    }\n    printf(\"%d\\n\", sum);\n}\nvoid reverse(const char *input) {\n    int len = (int)strlen(input);\n    for (int i = len-1; i >= 0; i--) putchar(input[i]);\n    putchar('\\n');\n}\nvoid caesar(const char *input) {\n    int shift = 0;\n    const char *p = input;\n    while (*p >= '0' && *p <= '9') {\n        shift = shift*10 + (*p - '0');\n        p++;\n    }\n    if (*p == ' ') p++;\n    char c;\n    while ((c = *p++)) {\n        if (c >= 'a' && c <= 'z') {\n            putchar((c - 'a' + shift) % 26 + 'a');\n        } else {\n            putchar(c);\n        }\n    }\n    putchar('\\n');\n}\nint main() {\n    char input[35] = {0};\n    char next_task[15] = {0};\n    if (!fgets(input, sizeof(input), stdin)) input[0] = '\\0';\n    else input[strcspn(input, \"\\n\")] = '\\0';\n    if (!fgets(next_task, sizeof(next_task), stdin)) next_task[0] = '\\0';\n    else next_task[strcspn(next_task, \"\\n\")] = '\\0';\n    |(input);\n    const char *p = source;\n    int sourc = 0, pip = 0;\n    while (*p) {\n        if (sourc==0&&strncmp(p, \"%SOURCE%\", 8) == 0) {\n            print(source);\n            sourc++;\n            p += 8;\n        }\n        else if (*p == '|'&&pip==0) {\n            pip++;\n            if (next_task[0] == '\\0') fputs(\"is_prime\", stdout);\n            else fputs(next_task, stdout);\n            p++;\n            if (*p == '(') {\n                putchar('(');\n                while (*++p && *p != ')') putchar(*p);\n                if (*p == ')') putchar(')');\n            }\n        }\n        else {\n            putchar(*p++);\n        }\n    }\n    return 0;\n}";
void print(const char *src) {
    static const char hex[] = "0123456789abcdef";
    unsigned char c;
    while ((c = (unsigned char)*src++)) {
        if (c == '\\') fputs("\\\\", stdout);
        else if (c == '"') fputs("\\\"", stdout);
        else if (c == '\n') fputs("\\n", stdout);
        else if (c == '\t') fputs("\\t", stdout);
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
    for (int i = 2; i*i <= n; i++) {
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
        printf("1\n");
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
    int count, sum = 0, val;
    const char *p = input;
    count = 0;
    while (*p >= '0' && *p <= '9') {
        count = count*10 + (*p - '0');
        p++;
    }
    while (*p == ' ') p++;
    for (int i=0;i<count;i++) {
        val = 0;
        while (*p >= '0' && *p <= '9') {
            val = val*10 + (*p - '0');
            p++;
        }
        sum += val;
        while (*p == ' ') p++;
    }
    printf("%d\n", sum);
}
void reverse(const char *input) {
    int len = (int)strlen(input);
    for (int i = len-1; i >= 0; i--) putchar(input[i]);
    putchar('\n');
}
void caesar(const char *input) {
    int shift = 0;
    const char *p = input;
    while (*p >= '0' && *p <= '9') {
        shift = shift*10 + (*p - '0');
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
int main() {
    char input[35] = {0};
    char next_task[15] = {0};
    if (!fgets(input, sizeof(input), stdin)) input[0] = '\0';
    else input[strcspn(input, "\n")] = '\0';
    if (!fgets(next_task, sizeof(next_task), stdin)) next_task[0] = '\0';
    else next_task[strcspn(next_task, "\n")] = '\0';
    is_prime(input);
    const char *p = source;
    int sourc = 0, pip = 0;
    while (*p) {
        if (sourc==0&&strncmp(p, "%SOURCE%", 8) == 0) {
            print(source);
            sourc++;
            p += 8;
        }
        else if (*p == '|'&&pip==0) {
            pip++;
            if (next_task[0] == '\0') fputs("is_prime", stdout);
            else fputs(next_task, stdout);
            p++;
            if (*p == '(') {
                putchar('(');
                while (*++p && *p != ')') putchar(*p);
                if (*p == ')') putchar(')');
            }
        }
        else {
            putchar(*p++);
        }
    }
    return 0;
}
```

I have made a c program that would perfectly solve the challenge.... *right*?

**Wrong**. It spitted out some error. More specifically, `undefined reference to {function}`

It turns out that I've overlooked the `gcc` compile line in `chall.py`, as I only thought it's going to compile the program. Turns out, the `-nostdlib` flag in the `gcc` command had **messed up** the program completely.

Now, I have to find a way to use `stdio.h` and `stdlib.h` in my program. Most obvious way? directly copy the content in those 2 header files into the `main.c`. But not only would that make the program too big, it would also make the program too slow (from the huge code output).

It seems that we need to reinvent std libs.

## nostdlib - checkpoint Q&A

Q - What are the placeholders?\
A - When the source printing sector encounter `%SOURCE%`, it would replace that with the source code. And when it encounter `|`, it would replace that with the next task function.

Q - What does the `-nostdlib` flag do? Why does it affect so much?\
A - If you think of the basic need for this program, it would be **input** and **output**. Maybe through scanf and printf.\
However, they are provided from `stdio.h` and `stdlib.h`. The `-nostdlib` flag basically removes all these libraries, and our program would be unable to do them through standard libraries. Which we then have to reinvent them ourselves.

## Reinventing stdlibs

### Identifying what functions we need

First of all, we need functions related to io operations.\
Sadly because I have been writing the code following my instincts, many many different stdout methods are used, and we kinda need lots of functions.\
Such as `putchar`, `fputs`, `puts`, `printf`, and `fgets`.

These can be achieved using syscall 0 and syscall 1, which is `write` and `read`, into and out of `stdout` and `stdin`.

We also need functions related to string operations.\
which are just `strlen`, `strcspn`, and `strncmp`.

These are just basic string manipulations.

And we need `atoi`, as we need to convert strings to numbers for some tasks.

This is just some ascii bytes convertions.

### Implementing the code

Apart from implementing the above said functions, I have also modified the placeholders in the source to be `@` for source and `^` for task func.\
These two symbols have no special meanings, choosing them is just because the symbols are not used anywhere else in the program.

Moreover, I forgot that it would run without the inputs for the first time, which panics my code when it defaults to is_prime for the task function. So I added `no`, a new default task function that does nothing.

<details open>
  <summary><b>Click to open/close the full one-step-away solve script</b></summary>

```c
typedef unsigned long long uint64_t;
typedef unsigned int uint;
typedef unsigned long size_t;
#define STDIN_FD 0
#define STDOUT_FD 1
const char *source = "A copy of this entire code with placeholders replaced. I'm gonna skip that part until the final working code, so that I can add comments on the code, and make this code more comfortable to look at.";
__attribute__((naked)) void _start(void) {
    __asm__ volatile (
        "mov $0, %rdi\n"   //(argc = 0)
        "mov $0, %rsi\n"   //(argv = NULL)
        "call main\n"      // Call main(argc, argv)
        "mov %eax, %edi\n" // use return value of main as exit status
        "mov $231, %rax\n" // syscall 231 (exit_group)
        "syscall\n"
        ".byte 0xf, 0x0b"  // Just in case syscall returns (which shouldnt happen, but just in case), execute 'ud2' to cause invalid opcode
    );
}
static int sys_write(int fd, const void *buf, unsigned int count) {
    int ret;
    __asm__ volatile (
        "mov $1, %%rax\n" // syscall 1 (write)
        "mov %1, %%rdi\n" // fd in rdi
        "mov %2, %%rsi\n" // buf pointer in rsi
        "mov %3, %%rdx\n" // count in rdx
        "syscall\n"
        "mov %%eax, %0\n" // move num of bytes written to ret
        : "=r"(ret)                                         // output operand
        : "r"((uint64_t)fd), "r"(buf), "r"((uint64_t)count) // input operands
        : "rax", "rdi", "rsi", "rdx"                        // clobbered registers
    );
    return ret;
}
static int sys_read(int fd, void *buf, unsigned int count) {
    int ret;
    __asm__ volatile (
        "mov $0, %%rax\n" // syscall 0 (read)
        "mov %1, %%rdi\n" // fd in rdi
        "mov %2, %%rsi\n" // buf pointer in rsi
        "mov %3, %%rdx\n" // count in rdx
        "syscall\n"
        "mov %%eax, %0\n" // move num of bytes written to ret
        : "=r"(ret)                                         // output operand
        : "r"((uint64_t)fd), "r"(buf), "r"((uint64_t)count) // input operands
        : "rax", "rdi", "rsi", "rdx"                        // clobbered registers
    );
    return ret;
}
int putchar(int c) {
    char ch = (char)c;
    if (sys_write(STDOUT_FD, &ch, 1) != 1) return -1; //return -1 if write fails (didnt write 1 byte to stdout)
    return c;
}
int fputs(const char *s) {
    size_t len = 0;
    while (s[len]) len++; //shouldve used strlen but I wrote this before writing strlen so
    int written = sys_write(STDOUT_FD, s, (unsigned int)len);
    return (written == (int)len) ? (int)len : -1; //return -1 if write fails (didnt write the full string to stdout)
}
int puts(const char *s) {
    if (fputs(s) < 0) return -1; //basically fputs
    if (putchar('\n') < 0) return -1; //plus trailing newline
    return 1;
}
size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++; // increment string length until null terminator (\0) which returns false
    return len;
}
int strncmp(const char *a, const char *b, size_t n) {
    size_t i = 0;
    for (; i < n; i++) { // dont ask me why I didnt stuff i=0 inside for loop, I was planning to use while loop lol
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb) return (int)(ca - cb);
        if (ca == 0) return 0;
    }
    return 0;
}
size_t strcspn(const char *s, const char *reject) { // ok it should be search but I named it reject lol
    size_t i = 0;
    for (; s[i]; i++) {
        size_t j = 0;
        while (reject[j]) {
            if (s[i] == reject[j]) return i; // literally linear search
            j++;
        }
    }
    return i;
}
int atoi(const char *s) {
    int sign = 1;
    int val = 0;
    while (*s == ' ' || *s == '\t' || *s == '\n') s++; // trim heading whitespace
    if (*s == '-') { sign = -1; s++; } // handle negative sign
    else if (*s == '+') s++; // who even use a heading pos sign :skull:
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0'); // convert string to int, 
        s++;
    }
    return sign * val;
}
int fgets(char *buf, int maxlen) {
    int i = 0;
    while (i < maxlen - 1) {
        char c;
        int r = sys_read(STDIN_FD, &c, 1); // read 1 byte
        if (r <= 0) break; // break if read fails
        buf[i++] = c;
        if (c == '\n') break; // break if newline
    }
    if (i == 0) return 0;
    buf[i] = '\0';
    return 1;
}
int printf(const char *fmt, int val) { // I really should just invent an integer printer instead of this printf ahh
    int count = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == '%') { // %% -> %
                if (putchar('%') < 0) return -1;
                count++;
            } else if (*fmt == 'd') { // %d -> int
                // basically itoa
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
        if (c == '\\') fputs("\\\\"); // if backslash, escape backslash
        else if (c == '"') fputs("\\\""); // if double quote, escape double quote
        else if (c == '\n') fputs("\\n"); // if newline, escape newline
        else if (c == '\t') fputs("\\t"); // if tab, escape tab
        else if (c >= 32 && c < 127) putchar(c); // if printable, print as is
        else { // if not printable, print as hex
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
```

</details>

<br>

Now this should compile perfectly under `-nostdlib` and run correctly for the chal.

### Big oopsies

Well good news is that, this indeed runs perfectly locally. Bad news? It doesn't run on the chal server.\
After creating a ticket and communicating with the chal author about the problem, I found out that it was my <sub>skill</sub> issue.\
It turns out my nsjail installation was broken, and I was testing this code locally without nsjail :flushed:.

After rebuilding the docker image, with an uncorrupted nsjail, the code dies, proving this code is still **wrong**.\
But since it works without nsjail, I knew we are close to solving this.

## Reinventing stdlibs - checkpoint Q&A

Q - How did the nsjail corrupt?\
A - This shouldn't be here at the Q&A but, and because, I don't know actually :sob: maybe it's bc of my super bad and fluctuating internet?

Q - How long did it took you to type all those functions?\
A - idk 12 hours :skull:\
I sat in front of my computer for 4 hours straight coding and debugging, and then I went to have a dinner break and then continue coding for another 8 hours\
*Absolute ~~Cinema~~ Pain*

## One step away

Now we can actually pinpoint the only barrier to be a **syscall violation** to nsjail.

After looking around the code, the only direct syscalls are `syscall 0 (write)`, `syscall 1 (read)`, and `syscall 231 (exit_group)`.\
These 3 syscalls are permitted by nsjail `"--seccomp_string", "ALLOW { read, write, close, execve, exit_group } DEFAULT KILL_PROCESS",`

After wasting 3 hours :skull:, I found out how :clown_face: I am.

In the program entry point, there is

```c
__attribute__((naked)) void _start(void) {
    __asm__ volatile (
        "mov $0, %rdi\n"   //(argc = 0)
        "mov $0, %rsi\n"   //(argv = NULL)
        "call main\n"      // Call main(argc, argv)
        "mov %eax, %edi\n" // use return value of main as exit status
        "mov $231, %rax\n" // syscall 231 (exit_group)
        "syscall\n"
        ".byte 0xf, 0x0b"  // Just in case syscall returns (which shouldnt happen, but just in case), execute 'ud2' to cause invalid opcode
    );
}
```

By looking, it only executes `syscall 231` and exits. However, at the end there lies `.byte 0xf, 0x0b`, which is `ud2`.\
It is a **trap instruction** that would forcefully exit the program if syscall failed to exit the program, but turns out nsjail dont like it.

By removing that line of code, the program can now perfectly run on the chal server.

*Also Im deeply sorry for using a windows machine, which pasting multiline code is somehow slow, and the netcat connection just exits before the pasting complete.*\
*Therefore I have compressed every function into a single line of code, which results into this not so readable code.*\
*(the uncompressed solve script is [here](./sol.c))*

```c
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
"__attribute__((naked)) void _start(void) {__asm__ volatile (\"mov $0, %rdi\\nmov $0, %rsi\\ncall main\\nmov %eax, %edi\\nmov $231, %rax\\nsyscall\");}\n"
"static int sys_write(int fd, const void *buf, unsigned int count) {int ret;__asm__ volatile (\"mov $1, %%rax\\nmov %1, %%rdi\\nmov %2, %%rsi\\nmov %3, %%rdx\\nsyscall\\nmov %%eax, %0\\n\": \"=r\"(ret): \"r\"((uint64_t)fd), \"r\"(buf), \"r\"((uint64_t)count): \"rax\", \"rdi\", \"rsi\", \"rdx\");return ret;}\n"
"static int sys_read(int fd, void *buf, unsigned int count) {int ret;__asm__ volatile (\"mov $0, %%rax\\nmov %1, %%rdi\\nmov %2, %%rsi\\nmov %3, %%rdx\\nsyscall\\nmov %%eax, %0\\n\": \"=r\"(ret): \"r\"((uint64_t)fd), \"r\"(buf), \"r\"((uint64_t)count): \"rax\", \"rdi\", \"rsi\", \"rdx\");return ret;}\n"
"int putchar(int c) {char ch = (char)c;if (sys_write(STDOUT_FD, &ch, 1) != 1) return -1;return c;}\n"
"int fputs(const char *s) {size_t len = 0;while (s[len]) len++;int written = sys_write(STDOUT_FD, s, (unsigned int)len);return (written == (int)len) ? (int)len : -1;}\n"
"int puts(const char *s) {if (fputs(s) < 0) return -1;if (putchar('\\n') < 0) return -1;return 1;}\n"
"size_t strlen(const char *s) {size_t len = 0;while (s[len]) len++;return len;}\n"
"int strncmp(const char *a, const char *b, size_t n) {size_t i = 0;for (; i < n; i++) {unsigned char ca = (unsigned char)a[i];unsigned char cb = (unsigned char)b[i];if (ca != cb) return (int)(ca - cb);if (ca == 0) return 0;}return 0;}\n"
"size_t strcspn(const char *s, const char *reject) {size_t i = 0;for (; s[i]; i++) {size_t j = 0;while (reject[j]) {if (s[i] == reject[j]) return i;j++;}}return i;}\n"
"int atoi(const char *s) {int sign = 1;int val = 0;while (*s == ' ' || *s == '\\t' || *s == '\\n') s++;if (*s == '-') { sign = -1; s++; }else if (*s == '+') s++;while (*s >= '0' && *s <= '9') {val = val * 10 + (*s - '0');s++;}return sign * val;}\n"
"int fgets(char *buf, int maxlen) {int i = 0;while (i < maxlen - 1) {char c;int r = sys_read(STDIN_FD, &c, 1);if (r <= 0) break;buf[i++] = c;if (c == '\\n') break;}if (i == 0) return 0;buf[i] = '\\0';return 1;}\n"
"int printf(const char *fmt, int val) {int count = 0;while (*fmt) {if (*fmt == '%') {fmt++;if (*fmt == '%') {if (putchar('%') < 0) return -1;count++;} else if (*fmt == 'd') {char buf[20];int i = 0;unsigned int uval;int negative = 0;if (val == 0) {buf[i++] = '0';} else {if (val < 0) {negative = 1;uval = (unsigned int)(-val);} else {uval = (unsigned int)val;}while (uval) {buf[i++] = '0' + (uval % 10);uval /= 10;}if (negative) buf[i++] = '-';}for (int j = i - 1; j >= 0; j--) {if (putchar(buf[j]) < 0) return -1;count++;}} else {if (putchar('%') < 0) return -1;if (putchar(*fmt) < 0) return -1;count += 2;}fmt++;} else {if (putchar(*fmt) < 0) return -1;count++;fmt++;}}return count;}\n"
"void print(const char *src) {static const char hex[] = \"0123456789abcdef\";unsigned char c;while ((c = (unsigned char)*src++)) {if (c == '\\\\') fputs(\"\\\\\\\\\");else if (c == '\"') fputs(\"\\\\\\\"\");else if (c == '\\n') fputs(\"\\\\n\\\"\\n\\\"\");else if (c == '\\t') fputs(\"\\\\t\");else if (c >= 32 && c < 127) putchar(c);else {putchar('\\\\');putchar('x');putchar(hex[c >> 4]);putchar(hex[c & 0xF]);}}}\n"
"void is_prime(const char *cn) {int n = atoi(cn);if (n <= 1) {puts(\"False\");return;}for (int i = 2; i * i <= n; i++) {if (n % i == 0) {puts(\"False\");return;}}puts(\"True\");}\n"
"void fibonacci(const char *cn) {int n = atoi(cn);int a = 0, b = 1, c;if (n == 1) {printf(\"1\\n\", 0);return;}for (int i = 2; i <= n; i++) {c = a + b;a = b;b = c;}printf(\"%d\\n\", b);}\n"
"void sum(const char *input) {int count = 0, sum = 0, val = 0;const char *p = input;while (*p >= '0' && *p <= '9') {count = count * 10 + (*p - '0');p++;}while (*p == ' ') p++;for (int i = 0; i < count; i++) {val = 0;while (*p >= '0' && *p <= '9') {val = val * 10 + (*p - '0');p++;}sum += val;while (*p == ' ') p++;}printf(\"%d\\n\", sum);}\n"
"void no(const char *input) {puts(\"\");};\n"
"size_t strlen_wrapper(const char *s) {return strlen(s);}\n"
"void reverse(const char *input) {int len = (int)strlen_wrapper(input);for (int i = len - 1; i >= 0; i--) putchar(input[i]);putchar('\\n');}\n"
"void caesar(const char *input) {int shift = 0;const char *p = input;while (*p >= '0' && *p <= '9') {shift = shift * 10 + (*p - '0');p++;}if (*p == ' ') p++;char c;while ((c = *p++)) {if (c >= 'a' && c <= 'z') {putchar((c - 'a' + shift) % 26 + 'a');} else {putchar(c);}}putchar('\\n');}\n"
"size_t strcspn_wrapper(const char *s, const char *reject) {return strcspn(s, reject);}\n"
"int main() {char input[35] = {0};char next_task[15] = {0};if (!fgets(input, sizeof(input))) input[0] = '\\0';else input[strcspn_wrapper(input, \"\\n\")] = '\\0';if (!fgets(next_task, sizeof(next_task))) next_task[0] = '\\0';else next_task[strcspn_wrapper(next_task, \"\\n\")] = '\\0';if (next_task[0] == '\\0') {int i;for (i = 0; i < sizeof(next_task) - 1 && input[i] != '\\0'; i++) {next_task[i] = input[i];}next_task[i] = '\\0';}^(input);const char *p = source;int sourc = 0, up = 0;while (*p) {if (sourc == 0 && *p == '@') {print(source);sourc++;p++;} else if (*p == '^' && up == 0) {up++;if (strncmp(next_task, \"is_prime\", 8) == 0 || strncmp(next_task, \"fibonacci\", 9) == 0 || strncmp(next_task, \"sum\", 3) == 0 || strncmp(next_task, \"reverse\", 7) == 0 || strncmp(next_task, \"caesar\", 6) == 0) {fputs(next_task);} else {fputs(\"no\");}p++;} else {putchar(*p++);}}return 0;}";
__attribute__((naked)) void _start(void) {__asm__ volatile ("mov $0, %rdi\nmov $0, %rsi\ncall main\nmov %eax, %edi\nmov $231, %rax\nsyscall");}
static int sys_write(int fd, const void *buf, unsigned int count) {int ret;__asm__ volatile ("mov $1, %%rax\nmov %1, %%rdi\nmov %2, %%rsi\nmov %3, %%rdx\nsyscall\nmov %%eax, %0\n": "=r"(ret): "r"((uint64_t)fd), "r"(buf), "r"((uint64_t)count): "rax", "rdi", "rsi", "rdx");return ret;}
static int sys_read(int fd, void *buf, unsigned int count) {int ret;__asm__ volatile ("mov $0, %%rax\nmov %1, %%rdi\nmov %2, %%rsi\nmov %3, %%rdx\nsyscall\nmov %%eax, %0\n": "=r"(ret): "r"((uint64_t)fd), "r"(buf), "r"((uint64_t)count): "rax", "rdi", "rsi", "rdx");return ret;}
int putchar(int c) {char ch = (char)c;if (sys_write(STDOUT_FD, &ch, 1) != 1) return -1;return c;}
int fputs(const char *s) {size_t len = 0;while (s[len]) len++;int written = sys_write(STDOUT_FD, s, (unsigned int)len);return (written == (int)len) ? (int)len : -1;}
int puts(const char *s) {if (fputs(s) < 0) return -1;if (putchar('\n') < 0) return -1;return 1;}
size_t strlen(const char *s) {size_t len = 0;while (s[len]) len++;return len;}
int strncmp(const char *a, const char *b, size_t n) {size_t i = 0;for (; i < n; i++) {unsigned char ca = (unsigned char)a[i];unsigned char cb = (unsigned char)b[i];if (ca != cb) return (int)(ca - cb);if (ca == 0) return 0;}return 0;}
size_t strcspn(const char *s, const char *reject) {size_t i = 0;for (; s[i]; i++) {size_t j = 0;while (reject[j]) {if (s[i] == reject[j]) return i;j++;}}return i;}
int atoi(const char *s) {int sign = 1;int val = 0;while (*s == ' ' || *s == '\t' || *s == '\n') s++;if (*s == '-') { sign = -1; s++; }else if (*s == '+') s++;while (*s >= '0' && *s <= '9') {val = val * 10 + (*s - '0');s++;}return sign * val;}
int fgets(char *buf, int maxlen) {int i = 0;while (i < maxlen - 1) {char c;int r = sys_read(STDIN_FD, &c, 1);if (r <= 0) break;buf[i++] = c;if (c == '\n') break;}if (i == 0) return 0;buf[i] = '\0';return 1;}
int printf(const char *fmt, int val) {int count = 0;while (*fmt) {if (*fmt == '%') {fmt++;if (*fmt == '%') {if (putchar('%') < 0) return -1;count++;} else if (*fmt == 'd') {char buf[20];int i = 0;unsigned int uval;int negative = 0;if (val == 0) {buf[i++] = '0';} else {if (val < 0) {negative = 1;uval = (unsigned int)(-val);} else {uval = (unsigned int)val;}while (uval) {buf[i++] = '0' + (uval % 10);uval /= 10;}if (negative) buf[i++] = '-';}for (int j = i - 1; j >= 0; j--) {if (putchar(buf[j]) < 0) return -1;count++;}} else {if (putchar('%') < 0) return -1;if (putchar(*fmt) < 0) return -1;count += 2;}fmt++;} else {if (putchar(*fmt) < 0) return -1;count++;fmt++;}}return count;}
void print(const char *src) {static const char hex[] = "0123456789abcdef";unsigned char c;while ((c = (unsigned char)*src++)) {if (c == '\\') fputs("\\\\");else if (c == '"') fputs("\\\"");else if (c == '\n') fputs("\\n\"\n\"");else if (c == '\t') fputs("\\t");else if (c >= 32 && c < 127) putchar(c);else {putchar('\\');putchar('x');putchar(hex[c >> 4]);putchar(hex[c & 0xF]);}}}
void is_prime(const char *cn) {int n = atoi(cn);if (n <= 1) {puts("False");return;}for (int i = 2; i * i <= n; i++) {if (n % i == 0) {puts("False");return;}}puts("True");}
void fibonacci(const char *cn) {int n = atoi(cn);int a = 0, b = 1, c;if (n == 1) {printf("1\n", 0);return;}for (int i = 2; i <= n; i++) {c = a + b;a = b;b = c;}printf("%d\n", b);}
void sum(const char *input) {int count = 0, sum = 0, val = 0;const char *p = input;while (*p >= '0' && *p <= '9') {count = count * 10 + (*p - '0');p++;}while (*p == ' ') p++;for (int i = 0; i < count; i++) {val = 0;while (*p >= '0' && *p <= '9') {val = val * 10 + (*p - '0');p++;}sum += val;while (*p == ' ') p++;}printf("%d\n", sum);}
void no(const char *input) {puts("");};
size_t strlen_wrapper(const char *s) {return strlen(s);}
void reverse(const char *input) {int len = (int)strlen_wrapper(input);for (int i = len - 1; i >= 0; i--) putchar(input[i]);putchar('\n');}
void caesar(const char *input) {int shift = 0;const char *p = input;while (*p >= '0' && *p <= '9') {shift = shift * 10 + (*p - '0');p++;}if (*p == ' ') p++;char c;while ((c = *p++)) {if (c >= 'a' && c <= 'z') {putchar((c - 'a' + shift) % 26 + 'a');} else {putchar(c);}}putchar('\n');}
size_t strcspn_wrapper(const char *s, const char *reject) {return strcspn(s, reject);}
int main() {char input[35] = {0};char next_task[15] = {0};if (!fgets(input, sizeof(input))) input[0] = '\0';else input[strcspn_wrapper(input, "\n")] = '\0';if (!fgets(next_task, sizeof(next_task))) next_task[0] = '\0';else next_task[strcspn_wrapper(next_task, "\n")] = '\0';if (next_task[0] == '\0') {int i;for (i = 0; i < sizeof(next_task) - 1 && input[i] != '\0'; i++) {next_task[i] = input[i];}next_task[i] = '\0';}no(input);const char *p = source;int sourc = 0, up = 0;while (*p) {if (sourc == 0 && *p == '@') {print(source);sourc++;p++;} else if (*p == '^' && up == 0) {up++;if (strncmp(next_task, "is_prime", 8) == 0 || strncmp(next_task, "fibonacci", 9) == 0 || strncmp(next_task, "sum", 3) == 0 || strncmp(next_task, "reverse", 7) == 0 || strncmp(next_task, "caesar", 6) == 0) {fputs(next_task);} else {fputs("no");}p++;} else {putchar(*p++);}}return 0;}
```

and running this on the chal server gave us the

### Flag

`wwf{you_are_a_quine_master_now_congratulations}`

## One step away - checkpoint Q&A

Q - Why do you compress the code instead of using pwntools or file content piping?\
A - I don't know why but when I do that, the server gave **no response** at all, no error, no flag, no status, nothing.\
Since I just want to solve this chal asap, I used this kinda *unethical* method to submit the code :)\
And hey, you can still have the non-compressed and also working [solve script](./sol.c)

## Aftermath

After I get the flag, I helped my team to become the **3rd** team to solve this chal.

But since I'm not a native english speaker, I don't know what is ouroboros, and what is quine.

From this challenge I learned

* the definition of **quine** and **ouroboros** (lol).
* that you can use such cursed flag like **`-nostdlib`** for `gcc`. (I would remember this flag so much because it brought me **so much P A I N**.)
* how to make **quine** programs
* that it's **bad for your health** to stay awake until 4am and then sleep and then wake up at 6am :sad:
* What is and how to use **`nsjail`**

## Aftermath - checkpoint Q&A

Q - :moyai:\
A - I don't think you need a Q&A for aftermath lol
