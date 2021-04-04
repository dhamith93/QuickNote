#if !defined(TOKENIZER_H)
#define TOKENIZER_H

#include "token.h"
#include "tree.h"

class Tokenizer {
    private:
        Tree tree;
        std::vector<std::string> tags;

        bool comment(std::string &str);
        bool header(std::string &str);
        bool list(std::string str);
        bool subList(const std::string &str);
        bool oList(std::string str);
        bool link(std::string &str);
        bool image(std::string &str);
        bool blockquote(std::string &str);
        bool code(std::string &str);
        bool tag(std::string &str);
        bool tableRow(std::string &str);
        Token createHeader(std::string &str);
        Token createList();
        Token createListItem(std::string &str);
        Token createBlockquote(const std::string &str);
        Token createImage(std::string &str);
        Token createLink(std::string &str);
        Token createParagraph(std::string &str);
        Token createCodeBlock(const std::string &str, const std::string &lanugage);
        Token createTableRow(const std::string &str, std::vector<int> &options, bool isHeader = false);
        Token createTable(std::vector<std::string> &lines);
        std::string extractBetweenParentheses(std::string &str);
        std::string extractBetweenBrackets(std::string &str);
        std::string extractCodeLanguage(std::string &str);
        std::string extractTag(std::string &str);
        void replaceWithStrong(std::string &str);
        void replaceWithEm(std::string &str);
        void replaceWithStrikethrough(std::string &str);
        void replaceWithCode(std::string &str);
        void replaceWithLink(std::string &str);
        int getBlockquoteLevel(const std::string &str);
        void addToNesting(Token &parent, Token &newToken);
        void trimHeader(std::string &str, const int &headerCount);
        void trimBlockquote(std::string &str, const int &level);
        int getHeaderCount(const std::string &str);
        void insertLastList(Token &token, Token &newToken);
        int getNesting(const std::string &line);
        void trimListItem(std::string &line);
        std::vector<int> parseTableOptions(const std::string &line);
        void splitInput(std::vector<std::string> &lines, std::string &input);
        void encode(std::string &data);

    public:
        Tokenizer();
        void tokenize(std::string &input);
        std::vector<std::string> getTags();
        std::string getHTML();

};


#endif // TOKENIZER_H
