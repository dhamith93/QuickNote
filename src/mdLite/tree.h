#if !defined(TREE_H)
#define TREE_H

#include "token.h"

class Tree {
    private:
        Token root;
        void addToNesting(Token &token, Token &root, int &nesting, int count);
        void printToken(Token &token, const std::string &s, std::string &output);
    public:
        Tree();
        Token *last;
        std::string traversTree();
        void insert(Token token);
        void insert(Token token, int nesting);
};


#endif // TREE_H
