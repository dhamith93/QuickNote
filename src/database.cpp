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

Database::Database() {
    if (!Database::createConnection()) {
        qDebug() << "Error: connection error";
    } else {
        QSqlQuery query;
        query.prepare("CREATE TABLE IF NOT EXISTS notes (path TEXT NOT NULL)");
        query.exec();
        query.prepare("CREATE TABLE IF NOT EXISTS tags (note_id INT NOT NULL, tag TEXT NOT NULL)");
        query.exec();
        query.prepare("CREATE TABLE IF NOT EXISTS recent_notes (note_id INT NOT NULL, date_opened TEXT NOT NULL)");
        query.exec();
        query.prepare("CREATE TABLE IF NOT EXISTS config (font TEXT, display_mode TEXT, last_open_path TEXT)");
        query.exec();
   }
}

bool Database::createConnection() {
    // macos: ~/Library/Application Support/<QuickNote>/
    // linux: ~/.local/share/QuickNote
    QString dbPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + "/note.db";
    #ifdef Q_OS_WIN
    dbPath = QDir::currentPath() + "\\note.db"; // windows: PWD
    #endif
    if (!QSqlDatabase::contains()) {
        QSqlDatabase::addDatabase("QSQLITE");
    }
    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(dbPath);
    return db.open();
}

bool Database::insertNote(const QString& notePath) {
    QSqlQuery query;
    query.prepare("INSERT INTO notes (path) VALUES (:path)");
    query.bindValue(":path", notePath);
    if (query.exec())
        return insertRecentNote(getRowId(notePath.toStdString()));
    return false;
}

bool Database::insertNoteWithTags(const QString& notePath, const std::vector<std::string> &tags) {
    QSqlQuery query;    
    query.prepare("INSERT INTO notes (path) VALUES (:path)");
    query.bindValue(":path", notePath);
    query.exec();
    QVariant qv = query.lastInsertId();
    int noteId = qv.toInt();
    insertRecentNote(noteId);
    if (noteId != 0) {
        bool result;
        for (auto& tag : tags)
            result = insertTag(tag, noteId);

        return result;
    }
    return false;
}

bool Database::insertRecentNote(const int noteId) {
    QSqlQuery query;
    query.prepare("INSERT INTO recent_notes (note_id, date_opened) VALUES (:note_id, datetime('now', 'localtime'))");
    query.bindValue(":note_id", noteId);
    return query.exec();
}

bool Database::updateOpenedDate(const QString &notePath) {
    int noteId = getRowId(notePath.toStdString());
    QSqlQuery query;
    query.prepare("UPDATE recent_notes SET date_opened = datetime('now', 'localtime') WHERE note_id = :note_id");
    query.bindValue(":note_id", noteId);
    return query.exec();
}

bool Database::updateTags(const QString& notePath, const std::vector<std::string> &tags) {
    int noteId = getRowId(notePath.toUtf8().constData());    
    if (noteId != 0) {
        bool result;
        for (auto& tag : tags)
            result = insertTag(tag, noteId);

        return result;
    }
    return false;
}

int Database::getRowId(const std::string& path) {
    QSqlQuery query;
    query.prepare("SELECT rowid FROM notes WHERE path = (:path)");
    query.bindValue(":path", QString::fromStdString(path));

    if (query.exec()) {
        if (query.next())
            return query.value(0).toInt();
    }

    return -1;
}

bool Database::insertTag(const std::string& tag, const int& noteId) {
    QSqlQuery query;
    query.prepare("INSERT INTO tags (note_id, tag) VALUES (:note_id, :tag)");
    query.bindValue(":note_id", noteId);
    query.bindValue(":tag", QString::fromStdString(tag));
    return query.exec();
}

bool Database::deleteTags(const int& noteId) {
    QSqlQuery query;
    query.prepare("DELETE FROM tags WHERE note_id = (:note_id)");
    query.bindValue(":note_id", noteId);
    return query.exec();
}

QVector<QString> Database::getNotes() {
    QSqlQuery query("SELECT path FROM notes");
    QVector<QString> notes;
    while (query.next())
        notes.append(query.value(0).toString());

    return notes;
}

QVector<QString> Database::getTags() {
    QSqlQuery query("SELECT tag FROM tags");
    QVector<QString> tags;
    while (query.next())
        tags.append(query.value(0).toString());

    return tags;
}

QVector<QString> Database::getNotesByTag(std::string& tag) {
    QSqlQuery query;
    std::vector<int> ids;
    QVector<QString> paths;
    if (!tag.empty()) {
        query.prepare("SELECT note_id FROM tags WHERE tag = :tag");
        query.bindValue(":tag", QString::fromStdString(tag));
        if (query.exec()) {
            while (query.next())
                ids.push_back(query.value(0).toInt());
        }
        for (auto& id : ids) {
            query.prepare("SELECT path FROM notes WHERE rowid = :id");
            query.bindValue(":id", QString::fromStdString(std::to_string(id)));
            if (query.exec()) {
                query.next();
                paths.append(query.value(0).toString());
            }
        }
    }
    return paths;
}

bool Database::noteExists(const QString& notePath) {
    QSqlQuery query;
    query.prepare("SELECT path FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);

    if (query.exec())
       return query.next();

    return false;
}

bool Database::recentNoteExists(const QString& notePath) {
    int rowId = getRowId(notePath.toStdString());

    QSqlQuery query;
    query.prepare("SELECT note_id FROM recent_notes WHERE note_id = (:note_id)");
    query.bindValue(":note_id", rowId);

    if (query.exec())
       return query.next();

    return false;
}

bool Database::taggedNoteExists(const QString& notePath) {
    QSqlQuery query;
    query.prepare("SELECT path FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    if (query.exec())
       return query.next();

    return false;
}

bool Database::deletePath(const QString& notePath) {
    QSqlQuery query;
    int noteId = getRowId(notePath.toStdString());
    query.prepare("DELETE FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);

    if (query.exec()) {
        query.prepare("DELETE FROM recent_notes WHERE note_id = (:note_id)");
        query.bindValue(":note_id", noteId);
        return query.exec();
    }

    return false;
}

bool Database::deleteTaggedPath(const QString& notePath) {
    int noteId = getRowId(notePath.toStdString());

    if (deletePath(notePath))
        return deleteTags(noteId);

    return false;
}

QVector<QString> Database::getRecents() {
    QSqlQuery query("SELECT path FROM recent_notes INNER JOIN notes ON notes.rowid = recent_notes.note_id ORDER BY date_opened DESC LIMIT 10");
    QVector<QString> paths;
    while (query.next())
        paths.append(query.value(0).toString());

    return paths;
}

bool Database::fontConfigExists() {
    QSqlQuery query("SELECT COUNT(*) FROM config WHERE font != \"\"");
    query.first();
    int count = 0;
    count = query.value(0).toInt();
    return (count > 0);
}

bool Database::insertFontConfig(const QString& font) {
    QSqlQuery query;
    if (fontConfigExists()) {
        query.prepare("UPDATE config SET font = :font WHERE rowid = 1");
    } else {
        query.prepare("INSERT INTO config (font) VALUES (:font)");
    }
    query.bindValue(":font", font);
    return query.exec();
}

QString Database::getFontConfig() {
    QSqlQuery query("SELECT font FROM config WHERE rowid = 1");
    query.next();
    return query.value(0).toString();
}

bool Database::displayModeExists() {
    QSqlQuery query("SELECT COUNT(*) FROM config WHERE display_mode != \"\"");
    query.first();
    int count = 0;
    count = query.value(0).toInt();
    return (count > 0);
}

bool Database::insertDisplayMode(const QString& mode) {
    QSqlQuery query;
    if (displayModeExists()) {
        query.prepare("UPDATE config SET display_mode = :mode WHERE rowid = 1");
    } else {
        query.prepare("INSERT INTO config (display_mode) VALUES (:mode)");
    }
    query.bindValue(":mode", mode);
    return query.exec();
}

QString Database::getDisplayMode() {
    QSqlQuery query("SELECT display_mode FROM config WHERE rowid = 1");
    query.next();
    return query.value(0).toString();
}

bool Database::lastOpenPathExists() {
    QSqlQuery query("SELECT COUNT(*) FROM config WHERE last_open_path != \"\"");
    query.first();
    int count = 0;
    count = query.value(0).toInt();
    return (count > 0);
}

bool Database::insertLastOpenPath(const QString& path) {
    QSqlQuery query;
    if (lastOpenPathExists()) {
        query.prepare("UPDATE config SET last_open_path = :path WHERE rowid = 1");
    } else {
        query.prepare("INSERT INTO config (last_open_path) VALUES (:path)");
    }
    query.bindValue(":path", path);
    return query.exec();
}

QString Database::getLastOpenPath() {
    QSqlQuery query("SELECT last_open_path FROM config WHERE rowid = 1");
    query.next();
    return query.value(0).toString();
}
