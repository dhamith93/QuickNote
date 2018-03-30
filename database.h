#ifndef DATABASE_H
#define DATABASE_H
#include <QApplication>
#include <QSqlDatabase>
#include <QSql>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QSqlQuery>
#include <QString>
#include <QStandardPaths>

class Database {
    private:
        QSqlDatabase db;
    public:
        Database();

        bool insertNote(const QString& notePath);

        bool checkIfExists(const QString& notePath);

        bool checkRowCountEq(const int& x);

        bool deleteOldest();

        bool deletePath(const QString& notePath);

        QVector<QString> getRecents();

        bool fontConfigExists();

        bool insertFontConfig(const QString& font);

        QString getFontConfig();

        bool colorConfigExists();

        bool insertColorConfig(const QString& color, const QString& color1,  const QString type);

        QString getColorConfig(const QString type);
};


#endif // DATABASE_H
