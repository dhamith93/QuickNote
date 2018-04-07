#include "headers/cst_string.h"
using namespace std;

CstString::CstString() { }

vector<string> CstString::split(string& str, char d) {
    string token;
    vector<string> lineArray;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, d)) {
        lineArray.push_back(token);
    }

    return lineArray;
}

vector<string>CstString::split(string& str, char d, int n) {
    string token, temp;
    vector<string> lineArray;
    istringstream tokenStream(str);
    int i = 0;
    while (getline(tokenStream, token, d)) {
        if (i != n) {
            lineArray.push_back(token);
            i += 1;
        } else {
            temp += token;
        }
    }
    if (!temp.empty()) {
        lineArray.push_back(temp);
    }

    return lineArray;
}

int CstString::getCharCount(string& line, char d) {
    int count = 0;
    for (auto& c : line) {
        if (c != d) {
            break;
        }
        count += 1;
        if (count == 6 && d == 'h') {
            break;
        }
    }
    return count;
}

string CstString::subStrBetween(string& str, string d1, string d2, int offset) {
    int first = str.find(d1) + offset;
    int last;
    if (d1 != d2) {
        last = (offset == 1) ? str.find(d2) - first : str.find(d2);
    } else {
        last = str.find(d2, 1) - first;
    }
    return str.substr(first, last);
}

string CstString::linkReplace(string& line, string& content, string& link, string& text, int& startPos, int& endPos) {
    string temp = line.substr(startPos - 1, endPos - startPos + 1);
    string textToReplace;
    for (auto& c1 : temp) {
        if (c1 == '[' || c1 == ']' || c1 == '(' || c1 == ')' || c1 == '?') {
            textToReplace += '\\';
        }
        textToReplace += c1;
    }
    content = regex_replace(content, regex(textToReplace), "<a href=\"" + link + "\">" + text + "</a>");
    return content;
}
