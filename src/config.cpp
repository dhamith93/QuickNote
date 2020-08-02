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

void Config::set(const QString &key, const QString &value) {
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue(key, value);
}

QString Config::get(const QString &key) {
    QSettings settings(path, QSettings::IniFormat);
    return settings.value(key, "").toString();
}
