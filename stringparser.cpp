#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <regex>
#include "htmlelement.cpp"
using namespace std;

#define NEWLINE '\n'

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

        string getStringBetweenDelimiters(string& str, string d1, string d2, int offset = 1) {
            unsigned first = str.find(d1) + offset;
            unsigned last = (offset == 1) ? str.find(d2) - first : str.find(d2);
            return str.substr (first, last);
        }

        string linkReplace(string& line, string& content, string& link, string& text, int& startPos, int& endPos) {
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

        bool prevElementIsSame(string e, int pos) {
            if (elements.size() == 0) {
                return false;
            }
            return (elements[pos].element == e);
        }

        string popLastElementContent(int& pos) {
            string content = elements[pos].content;
            elements.pop_back();
            return content;
        }

        string popLastSubElementContent(int& pos, int& subPos) {
            string content = elements[pos].subElements[subPos].content;
            elements.pop_back();
            return content;
        }

        string parseInline(string& line) {
            int aCount = 0, tCount = 0, charCount = 0, startPos = 0, endPos = 0;
            string content = "", tempLink, tempText;
            bool isLinkStart = false, isLinkFinished = false, isTextStart = false, isTextFinished = false,
                italicized = false, bolded = false, strikedOut = false, code = false;
            for (auto& c : line) {
                charCount += 1;
                if (c == '*') {
                    aCount += 1;
                } else if (c == '~') {                    
                    tCount += 1;
                } else if (c == '[') {
                    startPos = charCount;
                    isTextStart = true;
                    content += c;
                    continue;
                } else if (c == '(') {
                    isLinkStart = true;
                    content += c;
                } else if (c == ']') {
                    isTextFinished = true;
                    content += c;
                } else if (c == ')') {
                    endPos = charCount;
                    content += c;
                    isLinkFinished = isTextFinished;
                } else if (c == '<') {
                    isLinkStart = true;
                    content += c;
                    startPos = charCount;
                    isLinkFinished = false;
                } else if (c == '>') {
                    content += c;
                    endPos = charCount;
                    isLinkFinished = true;
                } else if (c == '`') {
                    if (code) {
                        code = false;
                        content += "</code>";
                        continue;
                    }
                    code = true;
                    content += "<code>";
                } else {
                    if (code) {
                        content += c;
                        continue;
                    }
                    if (isLinkStart) {
                        if (c != ')' && c != '>') {
                            tempLink += c;
                        }
                    }
                    if (isTextStart && !isLinkStart) {
                        if (c != ']') {
                            tempText += c;
                        }
                    }
                    if (aCount == 1) {
                        aCount = 0;
                        if (italicized) {
                            content += "</em>";
                            italicized = false;
                            content += c;
                            continue;
                        }
                        content += "<em>";
                        italicized = true;
                    }
                    if (aCount == 2) {
                        aCount = 0;
                        if (bolded) {
                            content += "</strong>";
                            bolded = false;
                            content += c;
                            continue;
                        }
                        content += "<strong>";
                        bolded = true;
                    }                    
                    if (tCount == 2) {
                        tCount = 0;
                        if (strikedOut) {
                            content += "</s>";
                            strikedOut = false;
                            content += c;
                            continue;
                        }
                        content += "<s>";
                        strikedOut = true;
                    }
                    content += c;
                    continue;
                }
                // link with [text](link)
                if (isTextStart && isTextFinished && isLinkStart && isLinkFinished) {
                    isTextStart = false;
                    isTextFinished = false;
                    isLinkStart = false;
                    isLinkFinished = false;
                    content = linkReplace(line, content, tempLink, tempText, startPos, endPos);
                    tempText = "";
                    tempLink = "";
                }
                // Link within < >
                if (!isTextFinished && isLinkStart && isLinkFinished) {
                    isLinkStart = false;
                    isLinkFinished = false;
                    content = linkReplace(line, content, tempLink, tempLink, startPos, endPos);
                    tempLink = "";
                }

            }
            if (strikedOut || bolded || italicized) {
                string str = (strikedOut) ? "s" : (bolded) ? "strong" : "em";
                content += "<span></" + str + ">" + "</span>";
            }
            return content;
        }
public:
        void parse(string& input) {
            split(input);
            int count = 0, emptyLineCount = 1; // emptyLineCount is 1 because string split function ignores first empty line
            bool code = false;
            string codeText, codeClass;
            if (lineArray.size() < 1) {
                throw invalid_argument("No elements in the list");
            }
            for (auto& line : lineArray) {
                if (line == "" && !code) {
                    emptyLineCount += 1;
                    if (emptyLineCount >= 2) {
                        elements.push_back(HtmlElement {"br", ""});
                        count += 1;
                    }
                    continue;
                }
                if (!code && (line == "---" || line == "___")) {
                    elements.push_back(HtmlElement {"hr", ""});
                    count += 1;
                    continue;
                }
                if (!code && line.at(0) == '#') {
                    int headerLevel = getCharCount(line, '#');
                    string h = "h" + to_string(headerLevel);
                    string content = line.substr(headerLevel, line.length());
                    elements.push_back(HtmlElement {h, content});
                    count += 1;
                    continue;
                }
                int length = line.length();
                if (code && line != "```") {
                    codeText += "<br>" + line;
                    continue;
                }
                if (length >= 3) {
                    string subStr = line.substr(0, 2);
                    string subStr1 = line.substr(0, 3);
                    if (subStr1 == "```") {
                        if (code) {
                            elements.push_back(HtmlElement{"pre", ""});
                            elements[count].subElements.push_back(HtmlElement{"code", codeText, "class", codeClass});
                            count += 1;
                            code = false;
                            codeText = "";
                            continue;
                        }
                        code = true;
                        codeClass = line.substr(3, line.length() - 1);
                        continue;
                    }
                    if (length >= 4) {
                        if (line.substr(0, 4) == "<!--") {
                            continue;
                        }
                    }
                    if (isdigit(line.at(0)) && line.at(1) == '.') {
                        string item = parseInline(line);
                        elements.push_back(HtmlElement{"ol", ""});
                        elements[count].subElements.push_back(HtmlElement{"li", item, "style", "list-style-type: none;"});
                        count += 1;
                        continue;
                    }                    
                    if (subStr == "* " || subStr == "- ") {
                        string item = line.substr(2, line.length());
                        item = parseInline(item);
                        elements.push_back(HtmlElement{"ul", ""});
                        elements[count].subElements.push_back(HtmlElement{"li", item});
                        count += 1;
                        continue;
                    }
                    if (subStr == "\t*" || subStr == "\t-") {
                        string item = line.substr(2, line.length());
                        item = parseInline(item);
                        elements.push_back(HtmlElement{"ul", ""});
                        elements[count].subElements.push_back(HtmlElement{"ul", ""});
                        elements[count].subElements[0].subElements.push_back(HtmlElement{"li", item});
                        count += 1;
                        continue;
                    }
                    if (line.at(0) == '\t' && isdigit(line.at(1))) {
                        if ((isdigit(line.at(2)) && line.at(3) == '.') || line.at(2) == '.') {
                            string item = line.substr(1, line.length());
                            item = parseInline(item);
                            elements.push_back(HtmlElement{"ol", ""});
                            elements[count].subElements.push_back(HtmlElement{"ol", ""});
                            elements[count].subElements[0].subElements.push_back(HtmlElement{"li", item, "style", "list-style-type: none;"});
                            count += 1;
                            continue;
                        }
                    }
                    if (length >= 7) {
                        subStr1 = line.substr(0, 6);
                        subStr = line.substr(0, 3);
                        if (subStr1 == "    * " || subStr1 == "    - ") {
                            string item = line.substr(5, line.length());
                            item = parseInline(item);
                            elements.push_back(HtmlElement{"ul", ""});
                            elements[count].subElements.push_back(HtmlElement{"ul", ""});
                            elements[count].subElements[0].subElements.push_back(HtmlElement{"li", item});
                            count += 1;
                            continue;
                        }
                        if (isdigit(line.at(4))) {
                            if (line.at(5) == '.' || (isdigit(line.at(4)) && line.at(6) == '.' )) {
                                string item = parseInline(line);
                                elements.push_back(HtmlElement{"ol", ""});
                                elements[count].subElements.push_back(HtmlElement{"ol", ""});
                                elements[count].subElements[0].subElements.push_back(HtmlElement{"li", item, "style", "list-style-type: none;"});
                                count += 1;
                                continue;
                            }
                        }
                    }
                    subStr1 = line.substr(0, 4);
                    if (subStr1 == "> > ") {
                        string quote = line.substr(3, line.length());
                        if (prevElementIsSame("blockquote", count - 1)) {
                            count -= 1;
                            elements[count].subElements.push_back(HtmlElement{"blockquote", quote});
                            count += 1;
                            continue;
                        }
                        elements.push_back(HtmlElement{"blockquote", ""});
                        elements[count].subElements.push_back(HtmlElement{"blockquote", quote});
                        count += 1;
                        continue;
                    }
                    subStr = line.substr(0, 2);
                    if (subStr == "> ") {
                        string quote = line.substr(2, line.length());
                        if (count > 0) {
                            int subCount = elements[count - 1].subElements.size();
                            if (prevElementIsSame("blockquote", count - 1) && subCount == 0) {
                                count -= 1;
                                quote = popLastElementContent(count) + "<br />" + quote;
                            }
                        }
                        elements.push_back(HtmlElement{"blockquote", quote});
                        count += 1;
                        continue;
                    }                    
                    if (subStr == "![" && line.at(line.length() - 1) == ')') {
                        string inlineText = getStringBetweenDelimiters(line, "[", "]");
                        string link = getStringBetweenDelimiters(line, "(", ")");
                        if (link.length() > 0) {
                            elements.push_back(HtmlElement{"p", ""});
                            elements.push_back(HtmlElement{"img", inlineText, "src", link});
                            count += 2;
                        }
                        continue;
                    }                                       
                }
                string content = parseInline(line);
                if (emptyLineCount < 2) {
                    if (prevElementIsSame("p", count - 1)) {
                        count -= 1;
                        content = popLastElementContent(count) + "<br />" + content;
                    }
                }
                elements.push_back(HtmlElement{"p", content});
                count += 1;
                emptyLineCount = 1;
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
