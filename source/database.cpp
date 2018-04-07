#include "headers/database.h"

Database::Database() {
   QStringList paths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
   db = QSqlDatabase::addDatabase("QSQLITE");
   QString path = paths.at(0);
   QDir dir;
   dir.mkdir(paths.at(0));
   path += "/notePathDB.db";
   db.setDatabaseName(path);
   if (!db.open()) {
      qDebug() << "Error: connection error";
   } else {
       QSqlQuery query;
       query.prepare("CREATE TABLE IF NOT EXISTS notes (path TEXT NOT NULL)");
       query.exec();
       query.prepare("CREATE TABLE IF NOT EXISTS tags (note_id INT NOT NULL, tag NOT NULL)");
       query.exec();
       query.prepare("CREATE TABLE IF NOT EXISTS recent_notes (path TEXT NOT NULL)");
       query.exec();
       query.prepare("CREATE TABLE IF NOT EXISTS font_config (font TEXT NOT NULL)");
       query.exec();
       query.prepare("CREATE TABLE IF NOT EXISTS color_config (background TEXT NOT NULL, font TEXT NOT NULL)");
       query.exec();
   }
}

bool Database::insertNote(const QString& notePath) {
    QSqlQuery query;
    query.prepare("INSERT INTO recent_notes (path) VALUES (:path)");
    query.bindValue(":path", notePath);
    return query.exec();
}

bool Database::insertNoteWithTags(const QString& notePath, const std::vector<std::string> &tags) {
    QSqlQuery query;
    bool result;
    query.prepare("INSERT INTO notes (path) VALUES (:path)");
    query.bindValue(":path", notePath);
    query.exec();
    QVariant qv = query.lastInsertId();
    int noteId = qv.toInt();
    if (noteId != 0) {
        for (auto& tag : tags) {
            result = insertTag(tag, noteId);
        }
        return result;
    }
    return false;
}

bool Database::updateTags(const QString& notePath, const std::vector<std::string> &tags) {
    int noteId = getRowId(notePath.toUtf8().constData());
    bool result;
    if (noteId != 0) {
        for (auto& tag : tags) {
            result = insertTag(tag, noteId);
        }
        return result;
    }
    return false;
}

int Database::getRowId(const std::string path) {
    QSqlQuery query;
    query.prepare("SELECT rowid FROM notes WHERE path = (:path)");
    query.bindValue(":path", QString::fromStdString(path));
    query.exec();
    query.next();
    int id = query.value(0).toInt();
    return id;
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

QVector<QString> Database::getTags() {
    QSqlQuery query("SELECT tag FROM tags");
    QVector<QString> tags;
    while (query.next()) {
        tags.append(query.value(0).toString());
    }
    return tags;
}

QVector<QString> Database::getNotesByTag(std::string& tag) {
    QSqlQuery query;
    std::vector<int> ids;
    QVector<QString> paths;
    query.prepare("SELECT note_id FROM tags WHERE tag = :tag");
    query.bindValue(":tag", QString::fromStdString(tag));
    if (query.exec()) {
        while (query.next()) {
            ids.push_back(query.value(0).toInt());
        }
    }
    for (auto& id : ids) {
        query.prepare("SELECT path FROM notes WHERE rowid = :id");
        query.bindValue(":id", QString::fromStdString(std::to_string(id)));
        if (query.exec()) {
            query.next();
            paths.append(query.value(0).toString());
        }
    }
    return paths;
}

bool Database::recentNoteExists(const QString& notePath) {
    QSqlQuery query;
    query.prepare("SELECT path FROM recent_notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    if (query.exec()) {
       return query.next();
    }
    return false;
}

bool Database::taggedNoteExists(const QString& notePath) {
    QSqlQuery query;
    query.prepare("SELECT path FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    if (query.exec()) {
       return query.next();
    }
    return false;
}

bool Database::checkRowCountEq(const int& x) {
    QSqlQuery query("SELECT COUNT(*) FROM recent_notes");
    query.first();
    int count = query.value(0).toInt();
    return (count == x);
}

bool Database::deleteOldest() {
    QSqlQuery query("SELECT rowid FROM recent_notes ORDER BY rowid ASC LIMIT 1");
    query.first();
    int id = query.value(0).toInt();
    query.prepare("DELETE FROM recent_notes WHERE rowid = (:id)");
    query.bindValue(":id", id);
    return query.exec();
}

bool Database::deletePath(const QString& notePath) {
    QSqlQuery query;
    query.prepare("DELETE FROM recent_notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    return query.exec();
}

bool Database::deleteTaggedPath(const QString& notePath) {
    int noteId = getRowId(notePath.toUtf8().constData());
    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    query.exec();
    return deleteTags(noteId);
}

QVector<QString> Database::getRecents() {
    QSqlQuery query("SELECT path FROM recent_notes ORDER BY rowid DESC LIMIT 10");
    QVector<QString> paths;
    while (query.next()) {
        paths.append(query.value(0).toString());
    }
    return paths;
}

bool Database::fontConfigExists() {
    QSqlQuery query("SELECT COUNT(*) FROM font_config");
    query.first();
    int count = query.value(0).toInt();
    return (count > 0);
}

bool Database::insertFontConfig(const QString& font) {
    QSqlQuery query;
    if (fontConfigExists()) {
        query.prepare("UPDATE font_config SET font = :font WHERE rowid = 1");
    } else {
        query.prepare("INSERT INTO font_config (font) VALUES (:font)");
    }
    query.bindValue(":font", font);
    return query.exec();
}

QString Database::getFontConfig() {
    QSqlQuery query("SELECT font FROM font_config WHERE rowid = 1");
    query.next();
    return query.value(0).toString();
}

bool Database::colorConfigExists() {
    QSqlQuery query("SELECT COUNT(*) FROM color_config");
    query.first();
    int count = query.value(0).toInt();
    return (count > 0);
}

bool Database::insertColorConfig(const QString& color, const QString& color1,  const QString type) {
    QSqlQuery query(db);
    if (colorConfigExists()) {
        if (type == "background") {
            query.prepare("UPDATE color_config SET background = :color WHERE rowid = 1");
        } else {
            query.prepare("UPDATE color_config SET font = :color WHERE rowid = 1");
        }
    } else {
        if (type == "background") {
            query.prepare("INSERT INTO color_config (background, font) VALUES (:color, :color1)");
        } else {
            query.prepare("INSERT INTO color_config (font, background) VALUES (:color, :color1)");
        }
        query.bindValue(":color1", color1);
    }
    query.bindValue(":color", color);
    return query.exec();
}

QString Database::getColorConfig(const QString type) {
    QSqlQuery query;
    if (type == "background") {
        query.prepare("SELECT background FROM color_config WHERE rowid = 1");
    } else {
        query.prepare("SELECT font FROM color_config WHERE rowid = 1");
    }
    query.exec();
    query.next();
    return query.value(0).toString();
}
