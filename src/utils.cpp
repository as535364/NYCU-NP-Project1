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
    std::regex rgx = std::regex{R"(([\||!])(\d+))"};
    std::regex_token_iterator<std::string::iterator> it(tmp.begin(),tmp.end(),rgx,{-1, 1, 2});
    const std::regex_token_iterator<std::string::iterator> endTokens;

    bool earlyBreak = false;
    for(; it != endTokens; ++it){
        if(std::next(it, 1) == endTokens) { earlyBreak = true; break; }
        std::string cmd = *it++;
        removeLeadingTrailingSpace(cmd);
        std::string pipe = *it++;
        std::string numPipe = *it;
        size_t num = numPipe.empty() ? 0 : std::stoi(numPipe);
        bool errPipe = pipe == "!";
        result.emplace_back(cmd, num, errPipe);

    }
    if(it != endTokens && earlyBreak) {
        std::string cmd = *it;
        removeLeadingTrailingSpace(cmd);
        result.emplace_back(cmd, 0, false);
    }
    return result;
}