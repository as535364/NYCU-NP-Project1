#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <unistd.h>
#include <sys/wait.h>
#include "npshell.h"
#include "utils.h"

// find file exist in path
bool findExist(const std::string &cmd){
    std::string path = getenv("PATH");
    std::vector<std::string> pathList = split(path, ':');
    return std::any_of(pathList.begin(), pathList.end(), [&cmd](const std::string &path){
        return access((path + "/" + cmd).c_str(), X_OK) == 0;
    });
}

// find out where is the pipeFd for output
std::array<int, 2> findPipeFd(std::list<pipeFdItem> &pipeFdList, size_t lineCnt){
    auto it = std::find_if(pipeFdList.begin(), pipeFdList.end(), [lineCnt](const pipeFdItem &item){
        return item.line == lineCnt;
    });
    if(it != pipeFdList.end()){
        std::array<int, 2> pipeFd = {it->pipeFd[0], it->pipeFd[1]};
        pipeFdList.erase(it);
        return pipeFd;
    }
    return {-1, -1};
}

// number pipe: insert pipeFd to proper pos in list
std::array<int, 2> insertPipeFd(std::list<pipeFdItem> &pipeFdList, size_t lineCnt) {
    int pipeFd[2];
    auto it = pipeFdList.begin();
    while(it != pipeFdList.end() && it->line < lineCnt)
        it++;
    if(it -> line != lineCnt) {
        pipe(pipeFd);
        pipeFdList.insert(it, pipeFdItem(pipeFd, lineCnt));
    } else { // if pipeFd already exist, use it to output
        pipeFd[0] = it->pipeFd[0];
        pipeFd[1] = it->pipeFd[1];
    }
    return std::array<int, 2>{pipeFd[0], pipeFd[1]};
}

void forkProcess(const std::vector<std::string> &cmdArg,
                 std::array<int, 2> &pipeInFd, std::array<int, 2> &pipeOutFd, PipeType type,
                 const std::string &fileName="") {
    pid_t childPid;
    while((childPid = fork()) == -1) { usleep(100); }

    if(childPid == 0){ // child process
        char *argv[cmdArg.size() + 1];
        for(size_t i = 0; i < cmdArg.size(); ++i){
            argv[i] = const_cast<char *>(cmdArg[i].c_str());
        }
        argv[cmdArg.size()] = nullptr;

        if(type & PipeType::PIPE_IN){
            dup2(pipeInFd[0], STDIN_FILENO);
            close(pipeInFd[1]);
        }
        if(type & PipeType::PIPE_OUT){
            dup2(pipeOutFd[1], STDOUT_FILENO);
            close(pipeOutFd[0]);
        }
        if(type & PipeType::PIPE_ERR){
            dup2(pipeOutFd[1], STDERR_FILENO);
            // close(pipeOutFd[0]); it is closed when PIPE_OUT
        }
        if(!fileName.empty()){
            freopen(fileName.c_str(), "w", stdout);
        }
        execvp(argv[0], argv);
        exit(0);
    } else { // parent process
        if(type & PipeType::PIPE_IN){
            close(pipeInFd[0]);
            close(pipeInFd[1]);
        }
        if((type & PipeType::PIPE_OUT) == 0)
            waitpid(childPid, nullptr, 0);
    }
}

void processCmd(const std::string &inputCmd, size_t &lineCnt, std::list<pipeFdItem> &pipeFdList) {
    std::vector<CommandNumPipe> lineCmds = splitLineCmd(inputCmd);
    // number piped
    for(auto &lineCmd : lineCmds) {
        lineCnt++;
        std::array<int, 2> pipeInFd = findPipeFd(pipeFdList, lineCnt);
        std::array<int, 2> pipeOutFd = {-1, -1};

        // inline piped
        std::vector<std::string> inlinePipedCmd = split(lineCmd.cmd, '|');
        std::array<int, 2> prevPipe = {-1, -1}, nextPipe = {-1, -1};
        for(auto it = inlinePipedCmd.begin(); it != inlinePipedCmd.end(); ++it){
            std::string cmd = *it;
            std::vector<std::string> cmdArg =  split(cmd, ' ');
            if(cmdArg[0] == "printenv" && cmdArg.size() == 2) {
                const char *env_p = getenv(cmdArg[1].c_str());
                if(env_p != nullptr)
                    std::cout << env_p << std::endl;
            } else if(cmdArg[0] == "setenv" && cmdArg.size() == 3) {
                setenv(cmdArg[1].c_str(), cmdArg[2].c_str(), 1);
            } else if(cmdArg[0] == "exit") {
                exit(EXIT_SUCCESS);
            } else if(findExist(cmdArg[0])){
                PipeType type = PIPE_NONE;
                // need inline pipe
                if (inlinePipedCmd.size() > 1) {
                    if(it == inlinePipedCmd.begin()){
                        type = static_cast<PipeType>(type | PipeType::PIPE_OUT);
                    } else if(it == inlinePipedCmd.end() - 1){
                        type = static_cast<PipeType>(type | PipeType::PIPE_IN);
                    } else {
                        type = static_cast<PipeType>(type | PipeType::PIPE_IN | PipeType::PIPE_OUT);
                    }
                }
                // redirection
                std::string fileName;
                if(cmd.find('>') != std::string::npos){
                    fileName = cmdArg.back();
                    cmdArg.pop_back(); cmdArg.pop_back(); // pop '>' and fil
                }
                // number pipe in
                if(pipeInFd[0] != -1 && pipeInFd[1] != -1){
                    type = static_cast<PipeType>(type | PipeType::PIPE_IN);
                }
                // number pipe out
                if(lineCmd.numPipe != 0 && it == inlinePipedCmd.end() - 1){
                    type = static_cast<PipeType>(type | PipeType::PIPE_OUT);
                }
                if(lineCmd.errPipe && it == inlinePipedCmd.end() - 1){
                    type = static_cast<PipeType>(type | PipeType::PIPE_ERR);
                }

                if(pipeInFd[0] != -1 && pipeInFd[1] != -1 && it == inlinePipedCmd.begin()){
                    prevPipe = pipeInFd; // first command to be piped in
                }
                if(it != inlinePipedCmd.end() - 1 && (type & PipeType::PIPE_OUT)) {
                    pipe(nextPipe.data());
                } else if(lineCmd.numPipe != 0 && (type & PipeType::PIPE_OUT)) {
                    // generate pipe and insert into list in correct pos
                    pipeOutFd = insertPipeFd(pipeFdList, lineCnt + lineCmd.numPipe);
                    nextPipe = pipeOutFd;
                }
                forkProcess(cmdArg, prevPipe, nextPipe, type, fileName);
                close(prevPipe[0]);
                close(prevPipe[1]);
                prevPipe = nextPipe;
            } else {
                close(prevPipe[0]);
                close(prevPipe[1]);
                std::cerr << "Unknown command: " << '[' << cmdArg[0] << "]." << std::endl;
            }
        }
    }
}

int main() {
    signal(SIGCHLD, SIG_IGN);
    setenv("PATH","bin:.", 1);
    std::string s;
    size_t lineCnt = 0;
    std::list<pipeFdItem> pipeFdList;

    std::cout << "% ";
    while(std::getline(std::cin, s)){
        if(!s.empty())
            processCmd(s, lineCnt, pipeFdList);
        std::cout << "% ";
    }
    return 0;
}
