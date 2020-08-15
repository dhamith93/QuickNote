#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

class Config {
    public:
        const QString DISPLAY_MODE = "display_mode";
        const QString FONT = "font";
        const QString LAST_OPEN_NOTE = "last_open_note";
        const QString NOTE_DB_PATH = "note_db_path";
        const QString BLOCKQUOTE = "blockquote";
        const QString ITALIC = "italic";
        const QString BOLD = "bold";
        const QString STRIKETHROUGH = "strikethrough";
        const QString TAG = "tag";
        const QString CODE = "code";
        const QString HEADER = "header";
        const QString COMMENT = "comment";
        const QString SYMBOL = "symbol";
        const QString FOREGROUND_LIGHT = "foreground_light";
        const QString FOREGROUND = "foreground";
        const QString BACKGROUND_LIGHT = "background_light";
        const QString BACKGROUND = "background";
        static Config &getInstance();
        void init();
        void setDefaultStyles();
        void set(const QString &key, const QString &value);
        QString get(const QString &key);

    private:
        Config();
        QString path;
        QSettings settings;
        void setDefaultConfig();
};

#endif // CONFIG_H
