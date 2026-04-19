#include "shell.h"
#include "parser.h"
#include "executor.h"
#include <iostream>
#include <string>
#include <unistd.h>   // getcwd, isatty
#include <limits.h>   // PATH_MAX

void Shell::print_prompt() const {
    // Show current directory in the prompt, like a real shell
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        std::cout << "\033[32m" << cwd << "\033[0m";  // green
    }
    std::cout << " \033[34m$\033[0m ";  // blue $
    std::cout.flush();
}

void Shell::run() {
    std::string line;

    while (true) {
        // Only show prompt if we're connected to a terminal
        // (not when piping: echo "ls" | ./myshell)
        if (isatty(STDIN_FILENO)) {
            print_prompt();
        }

        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D) — exit cleanly
            std::cout << "\n";
            break;
        }

        // Skip blank lines
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
            continue;
        }

        auto pipeline = Parser::parse(line);
        Executor::execute(pipeline);
    }
}