#pragma once
#include "parser.h"

class Executor
{
    public:
        static bool execute(const Pipeline& pipeline);

    private:
        static bool execute_builtin(const Command& cmd);
        static void execute_external(const Command& cmd);
        static void execute_pipeline(const Pipeline& pipeline);
};