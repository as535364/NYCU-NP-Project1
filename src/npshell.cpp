#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include "utils.h"

// find file exist in path
bool findExist(const std::string &cmd){
    std::string path = getenv("PATH");
    std::vector<std::string> pathList = split(path, ':');
    return std::any_of(pathList.begin(), pathList.end(), [&cmd](const std::string &path){
        return access((path + "/" + cmd).c_str(), X_OK) == 0;
    });
}

void forkProcess(const std::string &cmdArg){
    pid_t chilePid = fork();
    while(chilePid == -1){
        chilePid = fork();
    }
    if(chilePid == 0){
        std::vector<std::string> args = split(cmdArg, ' ');
        char *argv[args.size() + 1];
        for(size_t i = 0; i < args.size(); ++i){
            argv[i] = const_cast<char *>(args[i].c_str());
        }
        argv[args.size()] = nullptr;
        execvp(argv[0], argv);
        exit(0);
    } else {
        waitpid(chilePid, nullptr, 0);
    }
}

void processCmd(const std::string &inputCmd) {
    std::vector<CommandNumPipe> lineCmds = splitLineCmd(inputCmd);
    // number piped
    for(auto &lineCmd : lineCmds) {
//        std::cerr << "lineCmd: " << lineCmd.cmd << ' ' << lineCmd.numPipe << std::endl;
        // inline piped
        for(const auto& cmd : split(lineCmd.cmd, '|')) {
//            std::cerr << "cmd: " << cmd << std::endl;
            std::vector<std::string> cmdArg =  split(cmd, ' ');
            if(cmdArg[0] == "printenv" && cmdArg.size() == 2) {
                const char *env_p = getenv(cmdArg[1].c_str());
                if(env_p != nullptr){
                    std::cout << env_p << std::endl;
                }
            } else if(cmdArg[0] == "setenv" && cmdArg.size() == 3) {
                setenv(cmdArg[1].c_str(), cmdArg[2].c_str(), 1);
            } else if(cmdArg[0] == "exit") {
                exit(EXIT_SUCCESS);
            } else if(findExist(cmdArg[0])){
                forkProcess(cmd);
            } else {
                std::cerr << "Unknown command: " << '[' << cmdArg[0] << ']' << std::endl;
            }
        }
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
