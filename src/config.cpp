#include "src/headers/config.h"
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

Config::Config() {
    path = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).at(0) + "/config.ini";
    QFileInfo file(path);
    if (!file.exists()) {
        setDefaultConfig();
        setDefaultStyles();
    }
}

Config& Config::getInstance() {
    static Config config;
    return config;
}

void Config::init() { }

void Config::setDefaultConfig() {
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue(FONT, "");
    settings.setValue(DISPLAY_MODE, "");
    settings.setValue(LAST_OPEN_NOTE, "");
    settings.setValue(NOTE_DB_PATH, "");
    settings.sync();
}

void Config::setDefaultStyles() {
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue(FOREGROUND, "#FFFFFF");
    settings.setValue(FOREGROUND_LIGHT, "#454545");
    settings.setValue(BACKGROUND, "#252525");
    settings.setValue(BACKGROUND_LIGHT, "#FAFAFA");
    settings.setValue(BLOCKQUOTE, "#16CBF3");
    settings.setValue(ITALIC, "#FF6666");
    settings.setValue(BOLD, "#FA7272");
    settings.setValue(STRIKETHROUGH, "#C55F5F");
    settings.setValue(TAG, "#6E4EFF");
    settings.setValue(CODE, "#FF6B33");
    settings.setValue(HEADER, "#2A96F5");
    settings.setValue(COMMENT, "#0DFFC8");
    settings.setValue(SYMBOL, "#437BCE");
}

void Config::set(const QString &key, const QString &value) {
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue(key, value);
}

QString Config::get(const QString &key) {
    QSettings settings(path, QSettings::IniFormat);
    return settings.value(key, "").toString();
}
