#if !defined(TOKENIZER_H)
#define TOKENIZER_H

#include "token.h"
#include "tree.h"

class Tokenizer {
    private:
        Tree tree;
        std::vector<std::string> tags;

        bool header(std::string &str);
        bool list(std::string str);
        bool subList(std::string str);
        bool oList(std::string str);
        bool link(std::string &str);
        bool image(std::string &str);
        bool blockquote(std::string &str);
        bool code(std::string &str);
        bool tag(std::string &str);
        Token createHeader(std::string &str);
        Token createList();
        Token createListItem(std::string &str);
        Token createBlockquote(std::string &str);
        Token createImage(std::string &str);
        Token createLink(std::string &str);
        Token createParagraph(std::string &str);
        Token createCodeBlock(std::string &str, std::string &lanugage);
        std::string extractBetweenParentheses(std::string &str);
        std::string extractBetweenBrackets(std::string &str);
        std::string extractCodeLanguage(std::string &str);
        std::string extractTag(std::string &str);
        void replaceWithStrong(std::string &str);
        void replaceWithEm(std::string &str);
        void replaceWithStrikethrough(std::string &str);
        void replaceWithCode(std::string &str);
        void replaceWithLink(std::string &str);
        int getBlockquoteLevel(std::string &str);
        void addToNesting(Token &parent, Token &newToken);
        void trimHeader(std::string &str, int &headerCount);
        void trimBlockquote(std::string &str, int &level);
        int getHeaderCount(std::string &str);
        void insertLastList(Token &token, Token &newToken);
        int getNesting(std::string &line);
        void trimListItem(std::string &line);
        void splitInput(std::vector<std::string> &lines, std::string &input);

    public:
        Tokenizer();
        void tokenize(std::string &input);
        std::vector<std::string> getTags();
        std::string getHTML();

};


#endif // TOKENIZER_H
