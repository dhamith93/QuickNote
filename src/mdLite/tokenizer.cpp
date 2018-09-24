#include "token.h"
#include "tokenizer.h"
#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string/regex.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

Tokenizer::Tokenizer() {
    Token t;
    t.tag = "div";
    t.isEmpty = false;
    tree.insert(t);
}

void Tokenizer::tokenize(std::string &input) {
    std::vector<std::string> lines;
    splitInput(lines, input);
    std::string prevToken;
    bool codeStarted = false;
    bool blockquoteStarted = false;
    std::string codeLines = "";
    std::string codeLanguage = "";
    Token token;
    int nesting = 1;
    int emptyLineCount = 0;
    for (auto &line : lines) {
        token.isEmpty = true;
        if (comment(line)) {
            token.text = line;
            token.isComment = true;
            token.isEmpty = false;
            tree.insert(token);
            token = { };
            continue;
        }
        if (!codeStarted) {
            if (header(line)) {
                token = createHeader(line);
                prevToken = "header";
                tree.insert(token);
            } else if (list(line) || oList(line)) {
                std::string tempLine = line;

                int nesting = getNesting(tempLine);

                Token t = createListItem(tempLine);
                t.nesting = nesting;
                t.isListItem = true;

                if (oList(line))
                    t.styles = "display:block;margin:0.5em;margin-left:-16px;";

                if (prevToken != "list") {
                    token = createList();
                    token.subTokens.push_back(t);
                    token.isEmpty = false;
                    token.isList = true;
                } else {
                    if (subList(line)) {
                        insertLastList(token, t);
                    } else {
                        token.subTokens.push_back(t);
                    }
                }
                prevToken = "list";
            } else if (image(line)) {
                token = createImage(line);
                tree.insert(token);
                prevToken = "image";
                token = { };
            } else if (link(line)) {
                token = createLink(line);
                tree.insert(token);
                prevToken = "link";
                token = { };
            } else if (code(line)) {
                codeStarted = true;
                codeLanguage = extractCodeLanguage(line);
            } else if (blockquote(line)) {
                if (!blockquoteStarted) {
                    blockquoteStarted = true;
                    token = createBlockquote(line);
                    nesting = token.nesting;
                } else {
                    Token t;
                    int level = getBlockquoteLevel(line);
                    if (level < nesting) {
                        t = createBlockquote(line);
                        addToNesting(token, t);
                    } else if (level == nesting) {
                        t.isBlockquoteText = true;
                        t.tag = "blockquote-text";
                        t.nesting = level;
                        std::string text = line;
                        trimBlockquote(text, t.nesting);
                        t.text = text;
                        token.subTokens.push_back(t);
                    } else {
                        t = createBlockquote(line);
                        token.subTokens.push_back(t);
                    }
                    nesting = t.nesting;
                }
            } else if (tag(line)) {
                tags.push_back(extractTag(line));
            } else {
                if (prevToken == "list") {
                    tree.insert(token);
                }

                if (blockquoteStarted) {
                    blockquoteStarted = false;
                    tree.insert(token);

                }

                if (line.length() == 0) {
                    if (emptyLineCount >= 1) {
                        prevToken = "p";
                        token.tag = "p";
                        token.text = "";
                        token.subTokens.clear();
                        token.isEmpty = false;
                    }
                    emptyLineCount += 1;
                } else {
                    emptyLineCount = 0;
                    token = createParagraph(line);
                    token.isEmpty = false;
                }

                if (!token.isEmpty) {
                    tree.insert(token);
                }

                token = { };
            }
        } else if (codeStarted) {
            if (code(line)) {
                codeStarted = false;
                token = createCodeBlock(codeLines, codeLanguage);
                codeLines = "";
                codeLanguage = "";
                tree.insert(token);
                token = { };
            } else {
                codeLines += line + "\n";
            }
        }
    }
}

void Tokenizer::splitInput(std::vector<std::string> &lines, std::string &input)  {
    boost::algorithm::split_regex(lines, input, boost::regex("(\n)"));
}

std::string Tokenizer::getHTML() {
    return tree.traversTree();
}

std::vector<std::string> Tokenizer::getTags() {
    return tags;
}

int Tokenizer::getNesting(std::string &line) {
    int count = 0;

    for (auto &c : line) {
        if (c == ' ' || c == '\t') {
            count += 1;
        } else {
            break;
        }
    }

    return count;
}

void Tokenizer::addToNesting(Token &parent, Token &newToken) {
    if ((parent.nesting - 1) == newToken.nesting) {
        parent.subTokens.push_back(newToken);
    } else {
        if (parent.subTokens.size() > 0)
            addToNesting(parent.subTokens.back(), newToken);
    }
}

std::string Tokenizer::extractBetweenParentheses(std::string &str) {
    std::string output = "";
    std::string p = "\\(([^\\)]+)\\)";
    const boost::regex pattern(p);
    boost::smatch result;
    if (boost::regex_search(str, result, pattern)) {
        std::string submatch(result[1].first, result[1].second);
        return submatch;
    }

    return output;
}

std::string Tokenizer::extractBetweenBrackets(std::string &str) {
    std::string output = "";
    std::string p = "\\[([\\w\\s^\\)]+)\\]";
    const boost::regex pattern(p);
    boost::smatch result;
    if (boost::regex_search(str, result, pattern)) {
        std::string submatch(result[1].first, result[1].second);
        return submatch;
    }

    return output;
}

bool Tokenizer::comment(std::string &str) {
    std::string pattern = "^<!--(.){0,}-->";
    const boost::regex regex(pattern);

    return (
        boost::regex_search(str, regex, boost::match_partial)
    );
}

/**
 * START: List related functions
 */

bool Tokenizer::list(std::string str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));

    return (
        str.length() > 2 &&
        ((str.at(0) == '*' && str.at(1) == ' '))
    );
}

bool Tokenizer::subList(std::string str) {
    return (getNesting(str) > 3);
}

bool Tokenizer::oList(std::string str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));

    std::string pattern = "^\\d+\\. \\w";
    const boost::regex regex(pattern);

    return (
        str.length() > 2 && boost::regex_search(str, regex, boost::match_partial)
    );
}

void Tokenizer::insertLastList(Token &token, Token &newToken) {
    if (token.subTokens.back().nesting == newToken.nesting) {
        token.subTokens.push_back(newToken);
    } else {
        if (token.subTokens.back().subTokens.size() == 0) {
            Token list = createList();
            list.subTokens.push_back(newToken);
            token.subTokens.back().subTokens.push_back(list);
        } else {
            insertLastList(token.subTokens.back().subTokens.back(), newToken);
        }
    }
}

Token Tokenizer::createList() {
    Token token;
    token.isList = true;
    token.tag = "ul";
    return token;
}

Token Tokenizer::createListItem(std::string &str) {
    Token token;
    token.isListItem = true;
    token.tag = "li";
    trimListItem(str);
    token.text = str;
    return token;
}

void Tokenizer::trimListItem(std::string &line) {
    int spaces = getNesting(line);
    int end = line.length();
    line = line.substr(spaces, end);

    if (!oList(line)) {
        if (spaces + 2 < end)
            line = line.substr(2, end);
    }
}

/**
 * START: Header related functions
 */

bool Tokenizer::header(std::string &str) {
    std::string pattern = "^#{1,6}\\s\\w";
    const boost::regex regex(pattern);

    return (
        boost::regex_search(str, regex, boost::match_partial)
    );
}

int Tokenizer::getHeaderCount(std::string &str) {
    int count = 0;
    for (auto& s : str) {
        if (s != '#')
            break;
        count += 1;
    }

    return count;
}

void Tokenizer::trimHeader(std::string &str, int &headerCount) {
    str = str.substr(headerCount + 1, str.length() - 1);
}

Token Tokenizer::createHeader(std::string &str) {
    int headerCount = getHeaderCount(str);
    Token token;
    if (headerCount > 0 && headerCount <= 6) {
        token.tag = "h" + std::to_string(headerCount);
        trimHeader(str, headerCount);
        token.text = str;
    }
    return token;
}

/**
 * START: Image related function
 */

bool Tokenizer::image(std::string &str) {
    std::string pattern = "!\\[\\w{0,}\\]\\((\\w|.){1,}\\)";
    const boost::regex regex(pattern);

    return (
        boost::regex_search(str, regex, boost::match_partial)
    );
}

Token Tokenizer::createImage(std::string &str) {
    Token token;
    token.tag = "img";
    token.text = extractBetweenBrackets(str);
    token.src = extractBetweenParentheses(str);
    token.isImage = true;
    return token;
}

/**
 * START: Link related function
 */

bool Tokenizer::link(std::string &str) {
    std::string pattern = "^(\\s)?\\[.{0,}\\]\\((https?:\\/\\/(?:www\\.|(?!www))[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\\.[^\\s]{2,}|www\\.[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\\.[^\\s]{2,}|https?:\\/\\/(?:www\\.|(?!www))[a-zA-Z0-9]\\.[^\\s]{2,}|www\\.[a-zA-Z0-9]\\.[^\\s]{2,})\\)";
    const boost::regex regex(pattern);

    return (
        boost::regex_search(str, regex, boost::match_partial)
    );
}

Token Tokenizer::createLink(std::string &str) {
    Token token;
    token.tag = "a";
    token.text = extractBetweenBrackets(str);
    token.href = extractBetweenParentheses(str);
    if (token.text == "")
        token.text = token.href;
    token.isLink = true;
    return token;
}

/**
 * START: Code related function
 */

bool Tokenizer::code(std::string &str) {
    return (str.length() >= 3 && str.substr(0, 3) == "```");
}

std::string Tokenizer::extractCodeLanguage(std::string &str) {
    if (str.length() > 3)
        return str.substr(3, str.length());
    return "";
}

Token Tokenizer::createCodeBlock(std::string &str, std::string &lanugage) {
    Token pre;
    pre.tag = "pre";
    pre.isPre = true;
    Token code;
    code.tag = "code";
    code.isCode = true;
    code.text = str;
    if (lanugage != "")
        code.htmlClass = lanugage;
    pre.subTokens.push_back(code);
    return pre;
}

/**
 * START: Blockquote related function
 */

bool Tokenizer::blockquote(std::string &str) {
    std::string pattern = "^(\\s*>){1,}.";
    const boost::regex regex(pattern);

    return (
        boost::regex_search(str, regex, boost::match_partial)
    );
}

int Tokenizer::getBlockquoteLevel(std::string &str) {
    int count = -1;
    for (auto& c : str) {
        if (c != '>')
            break;
        count += 1;
    }

    return (count > 0) ? count : 0;
}

void Tokenizer::trimBlockquote(std::string &str, int &level) {
    str = str.substr(level + 1, str.length());
}

Token Tokenizer::createBlockquote(std::string &str) {
    Token blockquote;
    blockquote.tag = "blockquote";
    blockquote.nesting = 0;
    Token t;
    t.nesting = 0;
    int level = getBlockquoteLevel(str);
    std::string quote = str;
    trimBlockquote(quote, level);
    if (level > 1) {
        Token text;
        text.text = quote;
        text.nesting = level;
        text.isBlockquoteText = true;
        text.tag = "blockquote-text";

        Token *last = &blockquote;
        for (int i = level - 1; i > 0; i--) {
            Token t1;
            t1.tag = "blockquote";
            t1.nesting = i;
            if (blockquote.subTokens.empty()) {
                blockquote.subTokens.push_back(t1);
                last = &blockquote.subTokens.back();
            } else {
                last->subTokens.push_back(t1);
                last = &last->subTokens.back();
            }
        }
        last->subTokens.push_back(text);

    } else {
        t.text = quote;
        t.nesting = 0;
        t.isBlockquoteText = true;
        t.tag = "blockquote-text";
        blockquote.subTokens.push_back(t);
    }
    return blockquote;
}

/**
 * START: Tag related function
 */

bool Tokenizer::tag(std::string &str) {
    std::string pattern = "^#{1}\\w";
    const boost::regex regex(pattern);

    return (
        boost::regex_search(str, regex, boost::match_partial)
    );
}

std::string Tokenizer::extractTag(std::string &str) {
    std::string output = "";
    std::string p = "^#{1}(\\w+)";
    const boost::regex pattern(p);
    boost::smatch result;
    if (boost::regex_search(str, result, pattern)) {
        std::string submatch(result[1].first, result[1].second);
        return submatch;
    }
    return output;
}

/**
 * START: Paragraph related function
 */

void Tokenizer::replaceWithStrong(std::string &str) {
    std::string p = "(\\*\\*)(.*?)(\\*\\*)";
    boost::regex pattern(p);
    str = boost::regex_replace(str, pattern, "<strong>$2</strong>");
}

void Tokenizer::replaceWithEm(std::string &str) {
    std::string p = "(\\*)(.*?)(\\*)";
    boost::regex pattern(p);
    str = boost::regex_replace(str, pattern, "<em>$2</em>");
}

void Tokenizer::replaceWithStrikethrough(std::string &str) {
    std::string p = "(\\~\\~)(.*?)(\\~\\~)";
    boost::regex pattern(p);
    str = boost::regex_replace(str, pattern, "<strike>$2</strike>");
}

void Tokenizer::replaceWithCode(std::string &str) {
    std::string p = "(`)(.*?)(`)";
    boost::regex pattern(p);
    str = boost::regex_replace(str, pattern, "<code>$2</code>");
}

void Tokenizer::replaceWithLink(std::string &str) {
    std::string p = "\\[(.*?)\\]\\((https?:\\/\\/(?:www\\.|(?!www))[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\\.[^\\s]{2,}|www\\.[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\\.[^\\s]{2,}|https?:\\/\\/(?:www\\.|(?!www))[a-zA-Z0-9]\\.[^\\s]{2,}|www\\.[a-zA-Z0-9]\\.[^\\s]{2,})\\)";
    boost::regex pattern(p);
    str = boost::regex_replace(str, pattern, "<a href=\"$2\">$1</a>");
}

Token Tokenizer::createParagraph(std::string &str) {
    replaceWithStrong(str);
    replaceWithEm(str);
    replaceWithStrikethrough(str);
    replaceWithCode(str);
    replaceWithLink(str);

    Token token;
    token.tag = "p";
    token.text = str;

    return token;
}
