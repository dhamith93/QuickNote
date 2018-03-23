#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

struct HtmlElement {
    string element;
    string content;
    string attrb = "";
    string attrbData;
    vector<HtmlElement> subElements;

    HtmlElement() {}
    HtmlElement(const string &element, const string &content) : element(element), content(content) { }
    HtmlElement(const string &element, const string &content, const string &attrb, const string &attrbData)
        : element(element), content(content), attrb(attrb), attrbData(attrbData) { }

    string str(int indent = 1) {
        ostringstream oss;
        string i(indent * 2, ' ');
        if (attrb.length() > 0) {
            oss << i << "< " << element << " " << attrb << "=\"" << attrbData << "\" " << ">" << endl;
        } else {
            oss << i << "<" << element << ">" << endl;
        }
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
