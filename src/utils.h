//
// Created by as535364 on 2022/10/17.
//

#ifndef NP_PROJECT_UTILS_H
#define NP_PROJECT_UTILS_H
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

struct CommandNumPipe {
    CommandNumPipe(std::string cmd, size_t numPipe) : cmd(cmd), numPipe(numPipe) {}
    std::string cmd;
    size_t numPipe;
};

void removeLeadingTrailingSpace(std::string &str);
std::vector<std::string> split(const std::string &s, char delim);
std::vector<CommandNumPipe> splitLineCmd(const std::string &s);

#endif //NP_PROJECT_UTILS_H
