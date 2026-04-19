#include <iostream>
#include <string>
#include <vector>
#include "../src/parser.h"

// ── tiny test helpers ──────────────────────────────────────────────────────

int passed = 0, failed = 0;

void check(const std::string& test_name, bool condition) {
    if (condition) {
        std::cout << "  [PASS] " << test_name << "\n";
        passed++;
    } else {
        std::cout << "  [FAIL] " << test_name << "\n";
        failed++;
    }
}

void section(const std::string& name) {
    std::cout << "\n── " << name << " ──\n";
}

// ── tests ──────────────────────────────────────────────────────────────────

void test_empty_input() {
    section("Empty / blank input");

    auto p = Parser::parse("");
    check("empty string → empty pipeline", p.is_empty());

    p = Parser::parse("   ");
    check("only spaces → empty pipeline", p.is_empty());

    p = Parser::parse("\t\t");
    check("only tabs → empty pipeline", p.is_empty());
}

void test_single_command() {
    section("Single command, no pipes");

    auto p = Parser::parse("ls");
    check("'ls' → 1 command", p.commands.size() == 1);
    check("'ls' → args[0] == 'ls'", p.commands[0].args[0] == "ls");

    p = Parser::parse("ls -la");
    check("'ls -la' → 1 command", p.commands.size() == 1);
    check("'ls -la' → 2 args", p.commands[0].args.size() == 2);
    check("'ls -la' → args[1] == '-la'", p.commands[0].args[1] == "-la");

    p = Parser::parse("echo hello world");
    check("'echo hello world' → 3 args", p.commands[0].args.size() == 3);
    check("'echo hello world' → args[2] == 'world'", p.commands[0].args[2] == "world");
}

void test_extra_whitespace() {
    section("Extra whitespace (should be ignored)");

    auto p = Parser::parse("  ls   -la  ");
    check("leading/trailing spaces handled", p.commands.size() == 1);
    check("args still correct with extra spaces", p.commands[0].args.size() == 2);
}

void test_pipe() {
    section("Pipes");

    auto p = Parser::parse("ls | grep cpp");
    check("'ls | grep cpp' → 2 commands", p.commands.size() == 2);
    check("left side: args[0] == 'ls'",   p.commands[0].args[0] == "ls");
    check("right side: args[0] == 'grep'", p.commands[1].args[0] == "grep");
    check("right side: args[1] == 'cpp'",  p.commands[1].args[1] == "cpp");

    p = Parser::parse("cat file.txt | grep foo | wc -l");
    check("3-stage pipeline → 3 commands", p.commands.size() == 3);
    check("cmd[2] is 'wc'", p.commands[2].args[0] == "wc");
}

void test_edge_cases() {
    section("Edge cases");

    // Pipe with no right-hand side — up to you how you want to handle this
    auto p = Parser::parse("ls |");
    check("trailing pipe → 1 command (no empty command added)",
          p.commands.size() == 1);

    // Just a pipe
    p = Parser::parse("|");
    check("bare '|' → empty pipeline", p.is_empty());
}

// ── main ───────────────────────────────────────────────────────────────────

int main() {
    std::cout << "Parser tests\n";
    std::cout << "============\n";

    test_empty_input();
    test_single_command();
    test_extra_whitespace();
    test_pipe();
    test_edge_cases();

    std::cout << "\n============\n";
    std::cout << "Results: " << passed << " passed, " << failed << " failed\n";

    return failed > 0 ? 1 : 0;  // non-zero exit if any test failed
}