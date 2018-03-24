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
#include <iostream>
using namespace std;

class Database {
private:
    QSqlDatabase db;
public:
    Database(const QString& path) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(path);

       if (!db.open()) {
          qDebug() << "Error: connection error";
       }
    }

    bool insertNote(const QString& notePath) {
        QSqlDatabase::drivers();
        QSqlQuery query;
        query.prepare("INSERT INTO notes (path) VALUES (:path)");
        query.bindValue(":path", notePath);
        if (query.exec()) {
            return true;
        } else {
            qDebug() << "insert error: "
                            << query.lastError();
            return false;
        }
    }

    bool checkIfExists(const QString& notePath) {
        QSqlQuery query;
        query.prepare("SELECT path FROM notes WHERE path = (:path)");
        query.bindValue(":path", notePath);
        if (query.exec()) {
           return query.next();
        }

        return false;
    }

    bool checkRowCountEq(const int& x) {
        QSqlQuery query("SELECT COUNT(*) FROM notes");
        query.first();
        int count = query.value(0).toInt();
        cout << count << endl;
        return (count == x);
    }

    bool deleteOldest() {
        QSqlQuery query("SELECT rowid FROM notes ORDER BY rowid ASC LIMIT 1");
        query.first();
        int id = query.value(0).toInt();
        query.prepare("DELETE FROM notes WHERE rowid = (:id)");
        query.bindValue(":id", id);
        return query.exec();
    }

    bool deletePath(const QString& notePath) {
        QSqlQuery query;
        query.prepare("DELETE FROM notes WHERE path = (:path)");
        query.bindValue(":path", notePath);
        return query.exec();
    }

    QVector<QString> getRecents() {
        QSqlQuery query("SELECT path FROM notes ORDER BY rowid DESC LIMIT 10");
        QVector<QString> paths;
        while (query.next()) {

            paths.append(query.value(0).toString());
        }
        return paths;
    }
};
