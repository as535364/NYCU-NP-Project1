//
// Created by as535364 on 2022/10/17.
//

#include "utils.h"

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (std::getline (ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}