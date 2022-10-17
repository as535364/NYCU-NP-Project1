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
    std::regex sep_regex = std::regex{R"(\s+\|(\d+))"};
    std::sregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
    std::sregex_token_iterator numIter(s.begin(), s.end(), sep_regex, 1);
    std::sregex_token_iterator end;
    std::vector<int> numPipe;
    std::vector<CommandNumPipe> result;

    for ( ; numIter != end; ++numIter){
        numPipe.push_back(std::stoi(*numIter));
    }
    for (size_t i = 0; iter != end; ++iter, ++i){
        std::string tmp(*iter);
        removeLeadingTrailingSpace(tmp);
        if(i == numPipe.size()){
            result.push_back({tmp, 0});
        } else {
            result.push_back({tmp, numPipe[i]});
        }
    }
    return result;
}