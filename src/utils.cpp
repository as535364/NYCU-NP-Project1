//
// Created by as535364 on 2022/10/17.
//

#include "utils.h"

std::vector<std::string> split(const std::string &str, const char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string tok;

    while (std::getline(ss, tok, delimiter)) {
        result.push_back(tok);
        std::cerr << result.size() << ' ' << tok << std::endl;
    }
    return result;
}