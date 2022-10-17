#include <iostream>
#include <string>
#include <vector>
#include "utils.h"

void processCmd(const std::string &cmds) {
    std::vector<std::string> cmdArgs = split(cmds, ' ');
//    for(int i = 0; i < cmdArgs.size(); ++i) {
//        std::cout << i << ' ' << cmdArgs[i] << std::endl;
//    }
    if(cmdArgs.size() && cmdArgs[0] == "exit") {
        exit(EXIT_SUCCESS);
    }
}

int main() {
    std::string s;
    while(std::cin >> s){
        processCmd(s);
    }
    return 0;
}
