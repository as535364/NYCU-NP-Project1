//
// Created by as535364 on 2022/10/19.
//

#ifndef NP_PROJECT_1_NPSHELL_H
#define NP_PROJECT_1_NPSHELL_H

struct pipeFdItem {
    pipeFdItem(int pipeFd[2], size_t lineCnt) : line(lineCnt) {
        this->pipeFd[0] = pipeFd[0];
        this->pipeFd[1] = pipeFd[1];
    }
    int pipeFd[2];
    size_t line;
};

enum PipeType {
    PIPE_IN = 1,
    PIPE_OUT = 2,
    PIPE_ERR = 4,
    PIPE_NONE = 0
};

#endif //NP_PROJECT_1_NPSHELL_H
