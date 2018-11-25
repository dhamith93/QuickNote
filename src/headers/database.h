#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>
#include <QString>
#include <QSqlDatabase>

class Database
{
    private:
        static bool createConnection();
    public:
        Database();
        bool insertNote(const QString& notePath);
        bool insertNoteWithTags(const QString& notePath, const std::vector<std::string>& tags);
        bool insertRecentNote(const int noteId);
        bool updateOpenedDate(const QString& notePath);
        bool updateTags(const QString& notePath, const std::vector<std::string>& tags);
        int getRowId(const std::string& path);
        bool insertTag(const std::string& tag, const int& noteId);
        bool deleteTags(const int& noteId);
        QVector<QString> getNotes();
        QVector<QString> getTags();
        QVector<QString> getNotesByTag(std::string& tag);
        bool noteExists(const QString& notePath);
        bool recentNoteExists(const QString& notePath);
        bool taggedNoteExists(const QString& notePath);
        bool deletePath(const QString& notePath);
        bool deleteTaggedPath(const QString& notePath);
        QVector<QString> getRecents();
        bool fontConfigExists();
        bool insertFontConfig(const QString& font);
        QString getFontConfig();
        bool displayModeExists();
        bool insertDisplayMode(const QString& font);
        QString getDisplayMode();
        bool lastOpenPathExists();
        bool insertLastOpenPath(const QString& path);
        QString getLastOpenPath();
};

#endif // DATABASE_H
