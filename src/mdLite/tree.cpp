#include "token.h"
#include "tree.h"
#include <vector>

Tree::Tree() {
    root.isEmpty = true;
    last = &root;
}

void Tree::insert(Token token) {
    if (root.isEmpty) {
        root = token;
    } else {
        root.subTokens.push_back(token);
        last = &root.subTokens.back();
    }
}

void Tree::insert(Token token, int nesting) {
    if (!root.isEmpty) {
        addToNesting(token, root, nesting, 2);
    }
}

void Tree::addToNesting(Token &token, Token &r, int &nesting, int count) {
    if (count > 10 || nesting == (count - 1)) {
        r.subTokens.push_back(token);
        return;
    } else {
        addToNesting(token, r.subTokens.back(), nesting, count += 1);
    }
}

std::string Tree::traversTree() {
    std::string output;
    printToken(root, "", output);
    return output;
}

void Tree::printToken(Token &token, std::string s, std::string &output) {

    if (!token.tag.empty() || token.isComment) {
        if (token.isComment) {
            output += token.text + "\n";
        } else if (token.isImage) {
            output += s + "<" + token.tag + " src=\"" + token.src + "\" title=\"" + token.text + "\" />\n";
        } else if (token.isLink) {
            output += s + "<" + token.tag + " href=" + token.href + ">";
            output += token.text;
            output += "</" + token.tag + ">\n";
        } else if (token.isBlockquoteText) {
            output += token.text;
        } else if (token.isCode || token.isPre) {
            std::string classes = (!token.htmlClass.empty()) ? " class=\"" + token.htmlClass + "\" " : "";
            output += "<" + token.tag + classes + ">\n";
            output += token.text + "\n";
            if (!token.subTokens.empty()) {
                for (int i = 0; i < token.subTokens.size(); i++) {
                    printToken(token.subTokens.at(i), "", output);
                }
            }
            output += "</" + token.tag + ">\n";
        } else {
            std::string styles = (!token.styles.empty()) ? " style=\"" + token.styles + "\"" : "";
            std::string classes = (!token.htmlClass.empty()) ? " class=\"" + token.htmlClass + "\" " : "";
            output += s + "<" + token.tag + classes + styles + ">\n";

            output += s + "    " + token.text + "\n";

            if (!token.subTokens.empty()) {
                for (int i = 0; i < token.subTokens.size(); i++) {
                    printToken(token.subTokens.at(i), s + "    ", output);
                }
            }
            if (!token.isBreak)
                output += s + "</" + token.tag + ">\n";
        }
    }

}
