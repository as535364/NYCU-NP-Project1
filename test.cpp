#include <iostream>
#include <regex>
#include <string>
using namespace std;
int main() {
    std::regex re = std::regex{R"(([^|!]+)(([\||!])(\d+))*)"};
    string s = "removetag test.html !2 ls |1 number";
    const std::regex_token_iterator<std::string::iterator> end_tokens;
    std::regex_token_iterator<std::string::iterator> it(s.begin(), s.end(), re, {1, 3, 4});
    std::vector<std::string> to_vector;
    while (it != end_tokens)
    {
        to_vector.emplace_back(*it++);
    }
    int size = 0;
    for(auto item: to_vector){
        cout << size++ << " " << item << endl;
    }

    // Display the content of the vector
//    std::copy(begin(to_vector),
//              end(to_vector),
//              std::ostream_iterator<std::string>(std::cout, "\n"));
    return 0;
}