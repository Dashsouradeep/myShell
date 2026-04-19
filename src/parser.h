#pragma once
#include <string>
#include <vector>

struct Command
{
    std::vector<std::string> args;
};

struct Pipeline
{
    std::vector<Command> commands;
    bool is_empty() const {return commands.empty();}
};

class Parser
{
    public:

        static Pipeline parse(const std::string& line);
        static std::vector<std::string> tokenize(const std::string& line);
};