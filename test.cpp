#include <iostream>
#include <regex>
#include <string>
using namespace std;
int main() {
    std::regex rgx = std::regex{R"(([\||!])(\d+))"};
    string s = "ls | cat | number";
    std::cout << s << std::endl;
    std::regex_token_iterator<std::string::iterator> it(s.begin(),
                                    s.end(),
                                    rgx,
                                    {-1, 1, 2});
    const std::regex_token_iterator<std::string::iterator> end_tokens;
    bool earlyBreak = false;
    for(; it != end_tokens; ++it){
        if(std::next(it, 1) == end_tokens || std::next(it, 2) == end_tokens) { earlyBreak = true; break; }
        std::string cmd = *it++;
        std::string pipe = *it++;
        std::string numPipe = *it;

        size_t num = numPipe.empty() ? 0 : std::stoi(numPipe);
        bool errPipe = pipe == "!";
        std::cout << cmd << ", " << pipe << ", " << numPipe << ", " << std::endl;
    }
    if(it != end_tokens && earlyBreak) {
        cerr << "early break" << endl;
        std::string cmd = *it;
        std::cout << cmd << ", " << std::endl;
    }
    return 0;
}