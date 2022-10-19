//
// Created by as535364 on 2022/10/17.
//

#include "utils.h"

void removeLeadingTrailingSpace(std::string &str) {
    while(!str.empty() && std::isspace(*str.begin()))
        str.erase(str.begin());

    while(!str.empty() && std::isspace(*str.rbegin()))
        str.erase(str.length()-1);
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (std::getline (ss, item, delim)) {
        removeLeadingTrailingSpace(item);
        result.push_back(item);
    }
    return result;
}

std::vector<CommandNumPipe> splitLineCmd(const std::string &s) {
    std::string tmp(s);
    std::vector<CommandNumPipe> result;
    std::regex re = std::regex{R"(([^|!]+)(([\||!])(\d+))*)"};
    const std::regex_token_iterator<std::string::iterator> end_tokens;
    std::regex_token_iterator<std::string::iterator> it(tmp.begin(), tmp.end(), re, {1, 3, 4});

    for(; it != end_tokens; ++it){
        std::string cmd = *it++;
        std::string pipe = *it++;
        std::string numPipe = *it;
        size_t num = numPipe.empty() ? 0 : std::stoi(numPipe);
        bool errPipe = pipe == "!";
        removeLeadingTrailingSpace(cmd);
        if(!cmd.empty())result.emplace_back(cmd, num, errPipe);
    }

    return result;
}