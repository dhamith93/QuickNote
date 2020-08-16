#ifndef DATABASE_H
#define DATABASE_H

#include <QVector>
#include <QString>
#include <QSqlDatabase>

class Database
{
    private:
        static bool createConnection(const QString& path);
    public:
        bool createNoteDb(const QString& path);
        int save(const std::string &output);
        bool save(const int noteId, const std::string &output);
        bool open(const QString &path);
        bool addTags(int &noteId, std::vector<std::string> &tags);
        bool addTag(int &noteId, std::string &tag);
        QVector<QVector<QString>> getRecents();
        QString getNote(int &noteId);
        QVector<QVector<QString>> getNotes();
        bool deleteNote(const int& noteId);
        bool deleteTags(const int& noteId);
        QVector<QString> getTags();
        QVector<QVector<QString>> getNotesByTag(std::string &tag);
        QVector<QVector<QString>> search(QString &key);

};

#endif // DATABASE_H
