#if !defined(TOKEN_H)
#define TOKEN_H

#include <string>
#include <map>
#include <vector>

struct Token {
        std::string tokenType;
        std::string text;
        //std::string classes[];
        std::string htmlClass = "";
        std::string id;
        std::string styles;
        std::vector<Token> subTokens;
        std::string src;
        std::string href;
        std::string title;
        std::string tag;
        int nesting = 0;
        bool isHeader = false;
        bool isList = false;
        bool isListItem = false;
        bool isParaghraph = false;
        bool isStrong = false;
        bool isEm = false;
        bool isInlineCode = false;
        bool isBlockquote = false;
        bool isBlockquoteText = false;
        bool isCode = false;
        bool isPre = false;
        bool isLink = false;
        bool isImage = false;
        bool isBreak = false;
        bool isEmpty = true;
};

#endif // TOKEN_H
