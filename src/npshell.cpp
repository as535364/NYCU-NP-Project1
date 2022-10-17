#include <iostream>
#include <string>
#include <vector>
#include "utils.h"

void processCmd(const std::string &lineCmd) {
    std::vector<std::string> cmdArgs = split(lineCmd, ' ');
    if(cmdArgs.size() == 0) {
        return;
    } else if(cmdArgs[0] == "printenv" && cmdArgs.size() == 2) {
        const char *env_p = getenv(cmdArgs[1].c_str());
        if(env_p != nullptr){
            std::cout << env_p << std::endl;
        }
    } else if(cmdArgs[0] == "setenv" && cmdArgs.size() == 3) {
        setenv(cmdArgs[1].c_str(), cmdArgs[2].c_str(), 1);
    } else if(cmdArgs[0] == "exit") {
        exit(EXIT_SUCCESS);
    } else {
        std::cout << "Unknown command: " << '[' << cmdArgs[0] << ']' << std::endl;
    }
}

int main() {
    setenv("PATH","bin:.", 1);
    std::string s;

    std::cout << "% ";
    while(std::getline(std::cin, s)){
        processCmd(s);
        std::cout << "% ";
    }
    return 0;
}
