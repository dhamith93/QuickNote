#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Config {
    public:
        const QString DISPLAY_MODE = "display_mode";
        const QString FONT = "font";
        const QString LAST_OPEN_NOTE = "last_open_note";
        const QString NOTE_DB_PATH = "note_db_path";
        static Config &getInstance();
        void init();
        void set(const QString &key, const QString &value);
        QString get(const QString &key);

    private:
        Config();
        QString path;
        QSettings settings;
        void setDefaultConfig();
};

#endif // CONFIG_H
