#ifndef STRING_H
#define STRING_H
#include <vector>
#include <sstream>
#include <regex>

class CstString {
    public:
        CstString();
        std::vector<std::string> split(std::string& str, char d);
        std::vector<std::string> split(std::string& str, char d, int n);
        int getCharCount(std::string& line, char d);
        std::string subStrBetween(std::string& str, std::string d1, std::string d2, int offset = 1);
        std::string linkReplace(std::string& line, std::string& content, std::string& link, std::string& text, int& startPos, int& endPos);
};

#endif // STRING_H
