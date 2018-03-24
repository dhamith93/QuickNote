#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <regex>
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

    string getStringBetweenDelimiters(string& str, string d1, string d2, int offset = 1) {
        unsigned first = str.find(d1) + offset;
        unsigned last = (offset == 1) ? str.find(d2) - first : str.find(d2);
        return str.substr (first, last);
    }

    string parseString(string& line) {
        string content;
        bool italicized = false, bolded = false, strikedOut = false;
        int aCount = 0, tCount = 0, charCount = 0;
        for (auto& c : line) {
            charCount += 1;
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

        return content;
    }

public:
    void parse(string& input) {
        bool uListBegin = false, subListBegin = false, codeBegin = false, blockquoteBegin = false,
                italicized = false, bolded = false, strikedOut = false, textStart = false, linkStart = false,
                textComplete = false, linkComplete = false, prevIsPara = false, prevIsQuote = false;
        int count = 0, lineCount = 0;
        string temp = "", tempQuote = "", tempText = "", tempLink = "";
        split(input);

        for (auto& line : lineArray) {
            if (line.length() == 0) {
                lineCount += 1;
                if (lineCount > 1) {
                    if (!prevIsQuote) {
                        elements.push_back(HtmlElement{"p", "&nbsp;"});
                        count += 1;
                    }
                    prevIsQuote = false;
                }
                continue;
            }
            if (line.length() > 0 && !codeBegin && line.at(0) == '#') {
                uListBegin = false;
                int hCount = getCharCount(line, '#');
                string h = "h" + to_string(hCount);
                string content = line.substr(hCount, line.length());
                elements.push_back(HtmlElement {h, content});
                count += 1;
                prevIsPara = false;
                prevIsQuote = false;
            } else if (line == "---") {
                uListBegin = false;
                elements.push_back(HtmlElement {"hr", ""});
                count += 1;
                prevIsPara = false;
            } else if (line.size() > 2 && line.substr(0, 2) == "* ") {
                if (!uListBegin) {
                    uListBegin = true;
                    elements.push_back(HtmlElement{"ul", ""});
                    count += 1;
                }
                if (uListBegin) {
                    string item = line.substr(2, line.length());
                    item = parseString(item);
                    elements[count - 1].subElements.push_back(HtmlElement("li", item));
                    subListBegin = false;
                }
                prevIsPara = false;
                prevIsQuote = false;
            } else if (line.size() > 2 && line.substr(0, 4) == "  * ") {
                if (uListBegin) {
                    string item = line.substr(3, line.length());
                    item = parseString(item);
                    int length = (int)elements[count - 1].subElements.size();
                    if (!subListBegin) {
                        subListBegin = true;
                        elements[count - 1].subElements[length - 1].subElements.push_back(HtmlElement{"ul", ""});
                    }
                    int subLength = (int)elements[count - 1].subElements[length - 1].subElements.size();
                    elements[count - 1].subElements[length - 1].subElements[subLength - 1].subElements.push_back(HtmlElement("li", item));
                }
                prevIsPara = false;
                prevIsQuote = false;
            } else if (line.size() > 2 && line.substr(0, 2) == "![" && line.at(line.length() - 1) == ')') {
                string inlineText = getStringBetweenDelimiters(line, "[", "]");
                string link = getStringBetweenDelimiters(line, "(", ")");

                if (link.length() > 0) {
                    elements.push_back(HtmlElement{"p", ""});
                    elements.push_back(HtmlElement{"img", inlineText, "src", link});
                    count += 2;
                }
                prevIsPara = false;
            } else if (line.size() > 2 && line.at(0) == '[' && line.at(line.length() - 1) == ')') {
                string inlineText = getStringBetweenDelimiters(line, "[", "]");
                string link = getStringBetweenDelimiters(line, "(", ")");

                if (inlineText.length() > 0 && link.length() > 0) {
                    elements.push_back(HtmlElement{"p", ""});
                    elements.push_back(HtmlElement{"a", inlineText, "href", link});
                    count += 1;
                }
                prevIsPara = false;
                prevIsQuote = false;
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
                prevIsPara = false;
                prevIsQuote = false;
            } else if (codeBegin) {
                temp += line + '\n';
                continue;
            } else if (line.size() > 2 && line.substr(0, 2) == "> ") {
                uListBegin = false;
                if (!blockquoteBegin) {
                    blockquoteBegin = true;
                }
                tempQuote += line.substr(1, line.length());
                prevIsPara = false;
            } else {
                uListBegin = false;
                if (blockquoteBegin) {
                    elements.push_back(HtmlElement {"blockquote", tempQuote});
                    count += 1;
                    blockquoteBegin = false;
                    tempQuote = "";
                }
                string content;
                int aCount = 0, tCount = 0, charCount = 0, startPos = 0, endPos = 0;
                for (auto& c : line) {
                    charCount += 1;
                    if (c == '*') {
                        aCount += 1;
                    } else if (c == '~') {
                        tCount += 1;
                    } else if (c == '[') {
                        startPos = charCount;
                        textStart = true;
                        content += c;
                        continue;
                    } else if (c == '(') {
                        linkStart = true;
                        content += c;
                        continue;
                    } else if (c == ']') {
                        textComplete = true;
                        content += c;
                    } else if (c == ')') {
                        endPos = charCount;
                        content += c;
                        linkComplete = textComplete;
                    } else if (c == '<') {
                        linkStart = true;
                        content += c;
                        startPos = charCount;
                        linkComplete = false;
                        continue;
                    } else if (c == '>') {
                        content += c;
                        endPos = charCount;
                        linkComplete = true;
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
                    if (linkStart) {
                        if (c != ')' && c != '>') {
                            tempLink += c;
                        }
                    }
                    if (textStart && !linkStart) {
                        if (c != ']') {
                            tempText += c;
                        }
                    }
                    // link with [text](link)
                    if (textStart && textComplete && linkStart && linkComplete) {
                        textStart = false;
                        textComplete = false;
                        linkStart = false;
                        linkComplete = false;

                        string textToReplaceTemp = line.substr(startPos - 1, endPos - startPos + 1);
                        string textToReplace;
                        for (auto& c1 : textToReplaceTemp) {
                            if (c1 == '[' || c1 == ']' || c1 == '(' || c1 == ')') {
                                textToReplace += '\\';
                            }
                            textToReplace += c1;
                        }
                        try {
                            content = regex_replace(content, regex(textToReplace), "<a href=\"" + tempLink + "\">" + tempText + "</a>");
                        } catch(exception ex) {

                        }
                        tempText = "";
                        tempLink = "";
                    }
                    // Link within < >
                    if (!textComplete && linkStart && linkComplete) {
                        linkStart = false;
                        linkComplete = false;
                        string textToReplaceTemp = line.substr(startPos - 1, endPos - startPos + 1);
                        string textToReplace;
                        for (auto& c1 : textToReplaceTemp) {
                            if (c1 == '?') {
                                textToReplace += '\\';
                            }
                            textToReplace += c1;
                        }
                        try {
                            content = regex_replace(content, regex(textToReplace), "<a href=\"" + tempLink + "\">" + tempLink + "</a>");
                        } catch (exception ex) {

                        }
                        tempLink = "";
                    }
                }
                if (prevIsPara && (lineCount == 0)) {
                    content = elements[count - 1].content + "<br />" + content;
                    elements.pop_back();
                    count -= 1;
                }
                elements.push_back(HtmlElement{"p", content});
                prevIsPara = true;
                prevIsQuote = false;
                content = "";
                count += 1;
            }
            if (blockquoteBegin) {
                if (prevIsQuote && (lineCount == 0)) {
                    tempQuote = elements[count - 1].content + "<br />" + tempQuote;
                    elements.pop_back();
                    count -= 1;
                }
                elements.push_back(HtmlElement {"blockquote", tempQuote});
                blockquoteBegin = false;
                count += 1;
                prevIsQuote = true;
                tempQuote = "";
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
            lineCount = 0;
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
