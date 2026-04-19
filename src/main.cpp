#include "shell.h"
#include <csignal>
#include <iostream>

int main() {
    // Ignore Ctrl+C in the shell itself (children handle it normally)
    // This way Ctrl+C kills a running "cat" but not the shell
    signal(SIGINT, SIG_IGN);

    std::cout << "myshell 1.0 — type 'help' or 'exit'\n";

    Shell shell;
    shell.run();

    return 0;
}