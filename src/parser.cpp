#include "parser.h"
#include <sstream>
using namespace std;

vector<string> Parser::tokenize(const string& line)
{
    vector<string> tokens;
    istringstream stream(line);
    string token;

    while(stream>>token)
    tokens.push_back(token);

    return tokens;
};

Pipeline Parser::parse(const string& line)
{
    Pipeline pipeline;
    auto tokens = Parser::tokenize(line);

    if(tokens.empty())
    return pipeline;

    Command current_cmd;
    for(const auto& token :tokens)
    {
        if(token == "|")
        {
            if(!current_cmd.args.empty())
            {
                pipeline.commands.push_back(current_cmd);
                current_cmd= Command{};
            }
        }
        else
        {
            current_cmd.args.push_back(token);
        }
    }
    if(!current_cmd.args.empty())
    {
        pipeline.commands.push_back(current_cmd);
    }
    return pipeline;

};