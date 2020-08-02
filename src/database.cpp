#include "headers/database.h"
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSql>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QVector>
#include <QVariant>
#include <vector>

#include "src/libs/include/helpers.h"

bool Database::open(const QString &path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    return db.open();
}

bool Database::createNoteDb(const QString& path) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();
    QSqlQuery query;
    bool res = false;
    query.prepare("CREATE TABLE IF NOT EXISTS note (title TEXT NOT NULL, content TEXT NOT NULL, created_date TIMESTAMP DEFAULT (datetime('now','localtime')) NOT NULL, modified_date DEFAULT (datetime('now','localtime')) NOT NULL)");
    res = query.exec();
    query.prepare("CREATE TABLE IF NOT EXISTS tag (note_id INT NOT NULL, tag TEXT NOT NULL)");
    res = query.exec();
    return res;
}


int Database::save(const std::string &output) {
    std::string title = Helpers::buildTitle(output);
    QSqlQuery query;
    query.prepare("INSERT INTO note (title, content) VALUES (:title, :content)");
    query.bindValue(":title", QString::fromStdString(title));
    query.bindValue(":content", QString::fromStdString(output));
    query.exec();
    return query.lastInsertId().toInt();
}

bool Database::save(const int noteId, const std::string &output) {
    std::string title = Helpers::buildTitle(output);
    QSqlQuery query;
    query.prepare("UPDATE note SET title = :title, content = :content, modified_date = datetime('now','localtime') WHERE ROWID = :noteId");
    query.bindValue(":title", QString::fromStdString(title));
    query.bindValue(":content", QString::fromStdString(output));
    query.bindValue(":noteId", noteId);
    return query.exec();
}

bool Database::addTags(int &noteId, std::vector<std::string> &tags) {
    bool res = false;
    for (auto tag : tags) {
        res = addTag(noteId, tag);
    }
    return res;
}

bool Database::addTag(int &noteId, std::string &tag) {
    QSqlQuery query;
    query.prepare("INSERT INTO tag (note_id, tag) VALUES (:noteId, :tag)");
    query.bindValue(":noteId", noteId);
    query.bindValue(":tag", QString::fromStdString(tag));
    return query.exec();
}

QVector<QVector<QString>> Database::getRecents() {
    QSqlQuery query("SELECT ROWID, title FROM note ORDER BY modified_date DESC LIMIT 10");
    QVector<QVector<QString>> notes;
    QVector<QString> note;
    while (query.next()) {
        note.append(query.value(0).toString());
        note.append(query.value(1).toString());
        notes.append(note);
        note.clear();
    }
    return notes;
}

QString Database::getNote(int &noteId) {
    QSqlQuery query;
    query.prepare("SELECT content FROM note WHERE ROWID = :noteId");
    query.bindValue(":noteId", noteId);
    if (!query.exec()) {
         qDebug() << "SQL error: "<< query.lastError().text() << endl;
    }
    while (query.next()) {
        return query.value(0).toString();
    }
    return "";
}

QVector<QVector<QString>> Database::getNotes() {
    QSqlQuery query("SELECT ROWID, title FROM note ORDER BY modified_date DESC");
    QVector<QVector<QString>> notes;
    QVector<QString> note;
    while (query.next()) {
        note.append(query.value(0).toString());
        note.append(query.value(1).toString());
        notes.append(note);
        note.clear();
    }
    return notes;
}

QVector<QString> Database::getTags() {
    QSqlQuery query("SELECT tag FROM tag");
    QVector<QString> tags;
    while (query.next())
        tags.append(query.value(0).toString());

    return tags;
}

QVector<QVector<QString>> Database::getNotesByTag(std::string& tag) {
    QSqlQuery query;
    QVector<QVector<QString>> paths;
    QVector<QString> path;
    if (!tag.empty()) {
        query.prepare("SELECT n.ROWID, n.title FROM tag AS t JOIN note AS n ON n.ROWID = t.note_id WHERE t.tag = :tag");
        query.bindValue(":tag", QString::fromStdString(tag));
        if (query.exec()) {
            while (query.next()) {
                path.append(query.value(0).toString());
                path.append(query.value(1).toString());
                paths.append(path);
                path.clear();
            }
        }
    }
    return paths;
}

bool Database::deleteTags(const int& noteId) {
    QSqlQuery query;
    query.prepare("DELETE FROM tag WHERE note_id = (:note_id)");
    query.bindValue(":note_id", noteId);
    return query.exec();
}
