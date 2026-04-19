#include "executor.h"
#include <iostream>
#include <unistd.h>    // fork, execvp, dup2, pipe, chdir
#include <sys/wait.h>  // waitpid
#include <cstring>     // strerror
#include <cerrno>      // errno
#include <vector>

// ── Built-in commands ──────────────────────────────────────────────────────

bool Executor::execute_builtin(const Command& cmd) {
    if (cmd.args.empty()) return false;

    const std::string& name = cmd.args[0];

    if (name == "exit") {
        std::cout << "Goodbye!\n";
        exit(0);
    }

    if (name == "cd") {
        const char* path = (cmd.args.size() > 1)
            ? cmd.args[1].c_str()
            : getenv("HOME");       // "cd" with no args → go home

        if (chdir(path) != 0) {
            std::cerr << "cd: " << path << ": " << strerror(errno) << "\n";
        }
        return true;
    }

    if (name == "help") {
        std::cout << "myshell — built-in commands:\n"
                  << "  cd [dir]   change directory\n"
                  << "  exit       quit the shell\n"
                  << "  help       show this message\n";
        return true;
    }

    return false; // Not a built-in
}

// ── Single command (no pipe) ──────────────────────────────────────────────

void Executor::execute_single(const Command& cmd) {
    // Build argv: execvp needs a null-terminated char* array
    std::vector<char*> argv;
    for (const auto& arg : cmd.args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr); // execvp requires null terminator

    // execvp searches PATH automatically (so "ls" finds /bin/ls)
    // On success it never returns — the child process IS now "ls"
    if (execvp(argv[0], argv.data()) == -1) {
        std::cerr << argv[0] << ": " << strerror(errno) << "\n";
        exit(1); // Kill the child — don't return to shell code
    }
}

// ── Pipeline execution ────────────────────────────────────────────────────
//
//  ls -la | grep .cpp | wc -l
//
//  Concept: create a pipe between each adjacent pair of commands.
//  Each child's stdout is wired to the next child's stdin via dup2().
//
//    child[0] stdout → pipe[0] → child[1] stdin
//    child[1] stdout → pipe[1] → child[2] stdin

void Executor::execute_pipeline(const Pipeline& pipeline) {
    int n = pipeline.commands.size();

    // We need (n-1) pipes for n commands
    // Each pipe: [read_end, write_end]
    std::vector<std::array<int,2>> pipes(n - 1);
    for (auto& p : pipes) {
        if (pipe(p.data()) == -1) {
            std::cerr << "pipe: " << strerror(errno) << "\n";
            return;
        }
    }

    std::vector<pid_t> pids;

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // ── Child process ──

            // Wire stdin from the previous pipe (all but first command)
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            // Wire stdout to the next pipe (all but last command)
            if (i < n - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close ALL pipe ends in the child — we've dup'd what we need
            for (auto& p : pipes) {
                close(p[0]);
                close(p[1]);
            }

            execute_single(pipeline.commands[i]);
            exit(1); // Unreachable if execvp succeeded
        }

        if (pid < 0) {
            std::cerr << "fork: " << strerror(errno) << "\n";
        } else {
            pids.push_back(pid);
        }
    }

    // Parent: close all pipe ends (critical — otherwise children block waiting
    // for stdin to close, which never happens if parent holds the write end)
    for (auto& p : pipes) {
        close(p[0]);
        close(p[1]);
    }

    // Wait for all children to finish
    for (pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }
}

// ── Main dispatch ─────────────────────────────────────────────────────────

bool Executor::execute(const Pipeline& pipeline) {
    if (pipeline.is_empty()) return true;

    // Single command: check built-ins first
    if (pipeline.commands.size() == 1) {
        if (execute_builtin(pipeline.commands[0])) return true;

        // External command: fork and exec
        pid_t pid = fork();
        if (pid == 0) {
            // Child
            execute_single(pipeline.commands[0]);
            exit(1);
        } else if (pid > 0) {
            // Parent waits
            waitpid(pid, nullptr, 0);
        } else {
            std::cerr << "fork: " << strerror(errno) << "\n";
        }
        return true;
    }

    // Pipeline
    execute_pipeline(pipeline);
    return true;
}