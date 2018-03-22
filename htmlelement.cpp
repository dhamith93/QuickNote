#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

struct HtmlElement {
    string element;
    string content;
    vector<HtmlElement> subElements;

    HtmlElement() {}
    HtmlElement(const string &element, const string &content) : element(element), content(content) { }

    string str(int indent = 1) {
        ostringstream oss;
        string i(indent * 2, ' ');
        oss << i << "<" << element << ">" << endl;
        if (content.size() > 0) {
            if (this->element == "pre") {
                indent = 0;
            }
            oss << string(indent * 3, ' ') << content << endl;
        }
        for (auto& e : subElements) {
           oss << e.str(indent + 1);
        }
        oss << i << "</" << element << ">" << endl;

        return oss.str();
    }
};
