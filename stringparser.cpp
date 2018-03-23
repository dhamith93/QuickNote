#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include "htmlelement.cpp"
using namespace std;

#ifdef _WIN32
    #define NEWLINE '\r\n'
#else
    #define NEWLINE '\n'
#endif

class StringParser {
private:
    string input;
    vector<string> lineArray;
    vector<HtmlElement> elements;
    char lineBreak = NEWLINE;

    void split(string& str) {
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, lineBreak)) {
            lineArray.push_back(token);
        }
    }

    int getCharCount(string& line, char d) {
        int count = 0;
        for (auto& c : line) {
            if (count >= 6) {
                break;
            }
            if (c == d) {
                count += 1;
            } else {
                break;
            }
        }
        return count;
    }

public:
    void parse(string& input) {
        bool uListBegin = false, subListBegin = false, codeBegin = false, blockquoteBegin = false,
                italicized = false, bolded = false, strikedOut = false;
        int count = 0;
        HtmlElement holding{};
        string temp = "", tempQuote = "";
        split(input);

        for (auto& line : lineArray) {
            if (line.length() == 0) {
                continue;
            }
            if (line.length() > 0 && !codeBegin && line.at(0) == '#') {
                uListBegin = false;
                int hCount = getCharCount(line, '#');
                string h = "h" + to_string(hCount);
                string content = line.substr(hCount, line.length());
                elements.push_back(HtmlElement {h, content});
                count += 1;
            } else if (line == "------" | line  == "======") {
                uListBegin = false;
                elements.push_back(HtmlElement {"hr", ""});
                count += 1;
            } else if (line.size() > 2 && line.substr(0, 2) == "* ") {
                if (!uListBegin) {
                    uListBegin = true;
                    elements.push_back(HtmlElement{"ul", ""});
                    count += 1;
                }
                if (uListBegin) {
                    string item = line.substr(2, line.length());
                    elements[count - 1].subElements.push_back(HtmlElement("li", item));
                    subListBegin = false;
                }
            } else if (line.size() > 2 && line.substr(0, 4) == "  * ") {
                if (uListBegin) {
                    string item = line.substr(3, line.length());
                    int length = (int)elements[count - 1].subElements.size();
                    if (!subListBegin) {
                        subListBegin = true;
                        elements[count - 1].subElements[length - 1].subElements.push_back(HtmlElement{"ul", ""});
                    }
                    int subLength = (int)elements[count - 1].subElements[length - 1].subElements.size();
                    elements[count - 1].subElements[length - 1].subElements[subLength - 1].subElements.push_back(HtmlElement("li", item));
                }
            } else if (line == "```") {
                uListBegin = false;
                if (!codeBegin) {
                    codeBegin = true;
                    continue;
                }
                elements.push_back(HtmlElement{"pre", temp});
                count += 1;
                codeBegin = false;
                temp = "";
            } else if (codeBegin) {
                temp += line + '\n';
                continue;
            } else if (line.size() > 2 && line.substr(0, 2) == "> ") {
                uListBegin = false;
                if (!blockquoteBegin) {
                    blockquoteBegin = true;
                }
                tempQuote += line.substr(1, line.length());
            } else {
                uListBegin = false;
                if (blockquoteBegin) {
                    elements.push_back(HtmlElement {"blockquote", tempQuote});
                    count += 1;
                    blockquoteBegin = false;
                    tempQuote = "";
                }
                string content;
                int aCount = 0, tCount = 0;
                for (auto& c : line) {
                    if (c == '*') {
                        aCount += 1;
                    } else if (c == '~') {
                        tCount += 1;
                    } else {
                        if (aCount == 1) {
                            if (italicized) {
                                content += "</em>";
                                italicized = false;
                            } else {
                                content += "<em>";
                                italicized = true;
                            }
                        } else if (aCount == 2) {
                            if (bolded) {
                                content += "</strong>";
                                bolded = false;
                            } else {
                                content += "<strong>";
                                bolded = true;
                            }
                        }
                        if (tCount == 2) {
                            if (strikedOut) {
                                content += "</s>";
                                strikedOut = false;
                            } else {
                                content += "<s>";
                                strikedOut = true;
                            }
                        }
                        tCount = 0;
                        aCount = 0;
                        content += c;
                    }
                }
                elements.push_back(HtmlElement{"p", content});
                content = "";
                count += 1;
            }
            if (blockquoteBegin) {
                elements.push_back(HtmlElement {"blockquote", tempQuote});
                blockquoteBegin = false;
                count += 1;
            }
            if (bolded) {
                elements.push_back(HtmlElement{"span", "</strong>"});
                bolded = false;
                count += 1;
            }
            if (italicized) {
                elements.push_back(HtmlElement{"span", "</em>"});
                italicized = false;
                count += 1;
            }
        }

    }

    vector<HtmlElement> getElements() {
        if (elements.size() > 0) {
            return elements;
        } else {
            throw invalid_argument("No elements in the list");
        }
    }
};
