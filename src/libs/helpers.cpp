#include "src/libs/include/helpers.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <QDebug>
#include <QApplication>
#include <QRegularExpression>
#include <boost/algorithm/string.hpp>

Helpers::Helpers() {

}

std::string Helpers::buildTitle(const std::string &input) {
    std::string title = Helpers::split(input, '\n').at(0);
    boost::trim(title);
    if (input.empty() || title.empty() || (title.length() == 1 && title.at(0) == ' '))
        return "untitled";

    if (title.at(0) != '#') {
        return title;
    } else {
        title = title.substr(1, title.length());
        boost::trim(title);
        return title;
    }
}

std::vector<std::string> Helpers::split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter))
       tokens.push_back(token);

    return tokens;
}

QString Helpers::getWordCount(QString &content) {
    int wordCount = content.split(QRegExp("(\\s|\\n|\\r)+"), QString::SkipEmptyParts).count();
    return QString::number(wordCount);
}

std::string Helpers::getFileContent(std::string path) {
    try {
        std::ifstream ifs(path);
        std::string output((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        ifs.close();
        return output;
    } catch(const std::exception& e) { }

    return "";
}

std::vector<std::string> Helpers::split(std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter))
       tokens.push_back(token);

    return tokens;
}

bool Helpers::checkListItem(QString &line) {
    QRegularExpression regex1("^\\s*\\*\\s");
    QRegularExpression regex2("^\\s*\\d*\\.\\s");
    return (regex1.match(line).hasMatch() || regex2.match(line).hasMatch());
}

bool Helpers::checkUnorderedListItem(QString &line) {
    QRegularExpression regex("^\\s*\\*\\s");
    return (regex.match(line).hasMatch());
}

bool Helpers::checkEmptyListItem(QString &line) {
    QRegularExpression regex1("^\\s*\\*([[:blank:]]){1,}$");
    QRegularExpression regex2("^\\s*\\d*\\.([[:blank:]]){1,}$");
    return (regex1.match(line).hasMatch() || regex2.match(line).hasMatch());
}


int Helpers::getSpaceCount(QString &line) {
    int count = 0;
    for (auto& c : line) {
        if (c == ' ') {
            count += 1;
        } else {
            break;
        }
    }
    return count;
}

QString Helpers::getNextNumber(QString &line) {
    QString number = "";
    QString trimmedLine = line.trimmed();
    for (auto& c : trimmedLine) {
        if (!c.isDigit())
            break;
        number += c;
    }
    if (number != "") {
        try {
            int newNumber = number.toInt() + 1;
            return QString::number(newNumber);
        } catch (std::exception &ex) {
            return "";
        }
    } else {
        return "";
    }
}

void Helpers::exportHTML(std::string &htmlSavePath, std::string &content) {
    std::ofstream out(htmlSavePath);
    std::string headerPath = "html/header.html";
    std::string footerPath = "html/footer.html";

#ifdef Q_OS_DARWIN
    headerPath = QString(QApplication::applicationDirPath() + "/../Resources/header.html").toStdString();
    footerPath = QString(QApplication::applicationDirPath() + "/../Resources/footer.html").toStdString();
#endif

    std::string output = Helpers::getFileContent(headerPath);
    output += content;
    output += getFileContent(footerPath);

    out << output;
    out.close();
}

std::string Helpers::makeList(std::string type, std::string &selection) {
    std::vector<std::string> lines = Helpers::split(selection, '\n');

    if (type == "unordered") {
        for (auto &line : lines) {
            if (!line.empty())
                line = "* " + line;
        }
    } else if (type == "ordered") {
        for (size_t i = 0; i < lines.size(); i++)
            lines.at(i) = std::to_string(i + 1) + ". " + lines.at(i);
    }

    std::string text = "";

    for (auto &line : lines)
        text += line + "\n";

    return text;
}
