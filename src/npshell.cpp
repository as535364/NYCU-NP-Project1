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
    auto it = std::find_if(pipeFdList.begin(), pipeFdList.end(), [&lineCnt](const pipeFdItem &item){
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
std::array<int, 2> insertPipeFd(std::list<pipeFdItem> &pipeFdList, size_t lineCnt, size_t numPipe) {
    int pipeFd[2];
    auto it = pipeFdList.begin();
    while(it != pipeFdList.end() && it->line < lineCnt + numPipe)
        it++;
    if(it -> line != lineCnt + numPipe) {
        pipe(pipeFd);
        pipeFdList.insert(it, pipeFdItem(pipeFd, lineCnt + numPipe));
    } else { // if pipeFd already exist, use it to output
//        std::cerr << "\tfound: " << it->pipeFd[0] << ' ' << it->pipeFd[1] << std::endl;
        pipeFd[0] = it->pipeFd[0];
        pipeFd[1] = it->pipeFd[1];
    }
//    std::cerr << "\tpipeFdList: ";
//    for(pipeFdItem &item : pipeFdList){
//        std::cerr << item.line << ' ' << item.pipeFd[0] << ' ' << item.pipeFd[1]  << ", ";
//    }
//    std::cerr << std::endl;
    return std::array<int, 2>{pipeFd[0], pipeFd[1]};
}

void forkProcess(const std::vector<std::string> &cmdArg,
                 std::array<int, 2> &pipeInFd, std::array<int, 2> &pipeOutFd, PipeType type){
    pid_t childPid;
    while((childPid = fork()) == -1) { usleep(100); }

    if(childPid == 0){ // child process
        char *argv[cmdArg.size() + 1];
        for(size_t i = 0; i < cmdArg.size(); ++i){
            argv[i] = const_cast<char *>(cmdArg[i].c_str());
        }
        argv[cmdArg.size()] = nullptr;

//        std::cerr << "\ttype: " << type << std::endl;
        if(type & PipeType::PIPE_IN){
//            std::cerr << "\tPIPE_IN: \"" << argv[0] << "\" used " << pipeInFd[0] << std::endl;
            dup2(pipeInFd[0], STDIN_FILENO);
            close(pipeInFd[1]);
        }
        if(type & PipeType::PIPE_OUT){
//            std::cerr << "\tPIPE_OUT: \"" << argv[0] << "\" used " << pipeOutFd[1] << std::endl;
            dup2(pipeOutFd[1], STDOUT_FILENO);
            close(pipeOutFd[0]);
        }
        if(type & PipeType::PIPE_ERR){
//            std::cerr << "\tPIPE_ERR: \"" << argv[0] << "\" used " << pipeOutFd[1] << std::endl;
            dup2(pipeOutFd[1], STDERR_FILENO);
            // close(pipeOutFd[0]); it is closed when PIPE_OUT
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
//        std::cerr << lineCnt << " lineCmd: " << lineCmd.cmd << " |" << lineCmd.numPipe << std::endl;
        std::array<int, 2> pipeInFd = findPipeFd(pipeFdList, lineCnt);
        std::array<int, 2> pipeOutFd = {-1, -1};

        // inline piped
        std::vector<std::string> inlinePipedCmd = split(lineCmd.cmd, '|');
        std::array<int, 2> prevPipe = {-1, -1}, nextPipe = {-1, -1};
        for(auto it = inlinePipedCmd.begin(); it != inlinePipedCmd.end(); ++it){
            std::string cmd = *it;
            std::vector<std::string> cmdArg =  split(cmd, ' ');
//            std::cerr << "\tcmd: ";
//            for(const auto& s:cmdArg)
//                std::cerr << s << ' ';
//            std::cerr << std::endl;
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
//                        std::cerr << "\tFirst: " << cmd << " " << std::endl;
                        type = static_cast<PipeType>(type | PipeType::PIPE_OUT);
                    } else if(it == inlinePipedCmd.end() - 1){
//                        std::cerr << "\tLast: " << cmd << " " << std::endl;
                        type = static_cast<PipeType>(type | PipeType::PIPE_IN);
                    } else {
//                        std::cerr << "\tMiddle: " << cmd << " " << std::endl;
                        type = static_cast<PipeType>(type | PipeType::PIPE_IN | PipeType::PIPE_OUT);
                    }
                }
                // number pipe in
                if(pipeInFd[0] != -1 && pipeInFd[1] != -1){
//                    std::cerr << "\tOuter PIPE_IN: " << std::endl;
                    type = static_cast<PipeType>(type | PipeType::PIPE_IN);
                }
                // number pipe out
                if(lineCmd.numPipe){
                    // generate pipe and insert into list in correct pos
                    pipeOutFd = insertPipeFd(pipeFdList, lineCnt, lineCmd.numPipe);
                    type = static_cast<PipeType>(type | PipeType::PIPE_OUT);
                }
                if(lineCmd.errPipe){
                    type = static_cast<PipeType>(type | PipeType::PIPE_ERR);
                }

                if(pipeInFd[0] != -1 && pipeInFd[1] != -1){
                    prevPipe = pipeInFd;
                }
                if(!lineCmd.numPipe && (type & PipeType::PIPE_OUT)) {
                    pipe(nextPipe.data());
                } else if(lineCmd.numPipe && (type & PipeType::PIPE_OUT)) {
                    nextPipe = pipeOutFd;
                }

                forkProcess(cmdArg, prevPipe, nextPipe, type);
                prevPipe = nextPipe;
            } else {
                std::cerr << "Unknown command: " << '[' << cmdArg[0] << "]." << std::endl;
            }
        }
//        std::cerr << inlinePipedCmd.size() << std::endl;
    }
}

int main() {
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
