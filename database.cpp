#include "database.h"

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
       query.prepare("CREATE TABLE IF NOT EXISTS font_config (font TEXT NOT NULL)");
       query.exec();
       query.prepare("CREATE TABLE IF NOT EXISTS color_config (background TEXT NOT NULL, font TEXT NOT NULL)");
       query.exec();
   }
}

bool Database::insertNote(const QString& notePath) {
    QSqlQuery query;
    query.prepare("INSERT INTO notes (path) VALUES (:path)");
    query.bindValue(":path", notePath);
    return query.exec();
}

bool Database::checkIfExists(const QString& notePath) {
    QSqlQuery query;
    query.prepare("SELECT path FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    if (query.exec()) {
       return query.next();
    }
    return false;
}

bool Database::checkRowCountEq(const int& x) {
    QSqlQuery query("SELECT COUNT(*) FROM notes");
    query.first();
    int count = query.value(0).toInt();
    return (count == x);
}

bool Database::deleteOldest() {
    QSqlQuery query("SELECT rowid FROM notes ORDER BY rowid ASC LIMIT 1");
    query.first();
    int id = query.value(0).toInt();
    query.prepare("DELETE FROM notes WHERE rowid = (:id)");
    query.bindValue(":id", id);
    return query.exec();
}

bool Database::deletePath(const QString& notePath) {
    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE path = (:path)");
    query.bindValue(":path", notePath);
    return query.exec();
}

QVector<QString> Database::getRecents() {
    QSqlQuery query("SELECT path FROM notes ORDER BY rowid DESC LIMIT 10");
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
