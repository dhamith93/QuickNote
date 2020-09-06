#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>
#include <QString>

class Helpers {
    public:
        Helpers();
        static std::string buildTitle(const std::string &input);
        static std::vector<std::string> split(const std::string &str, char delimiter);
        static QString getWordCount(QString &content);
        static std::string getFileContent(std::string path);
        static std::vector<std::string> split(std::string &str, char delimiter);
        static bool checkListItem(QString &line);
        static bool checkUnorderedListItem(QString &line);
        static bool checkEmptyListItem(QString &line);
        static int getSpaceCount(QString &line);
        static QString getNextNumber(QString &line);
        static void exportHTML(std::string &htmlSavePath, std::string &content);
        static std::string makeList(std::string type, std::string &selection);
};

#endif // HELPERS_H
