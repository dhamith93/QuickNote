#include "src/headers/displayconfigdialog.h"
#include "ui_displayconfigdialog.h"
#include "QFont"
#include "QFontDialog"
#include "QColor"
#include "QColorDialog"
#include "headers/config.h"
#include "QLayout"

DisplayConfigDialog::DisplayConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DisplayConfigDialog)
{
    ui->setupUi(this);
    init();
}

DisplayConfigDialog::~DisplayConfigDialog()
{
    delete ui;
}

void DisplayConfigDialog::on_btnReset_clicked() {
    Config::getInstance().setDefaultStyles();
    init();
}

void DisplayConfigDialog::init() {
    QFont font;
    font.fromString(Config::getInstance().get(Config::getInstance().FONT));
    ui->txtFont->setText(font.family());
    ui->txtBgColor->setText(Config::getInstance().get(Config::getInstance().BACKGROUND));
    ui->txtFontColor->setText(Config::getInstance().get(Config::getInstance().FOREGROUND));
    ui->txtBQuotesColor->setText(Config::getInstance().get(Config::getInstance().BLOCKQUOTE));
    ui->txtCodesColor->setText(Config::getInstance().get(Config::getInstance().CODE));
    ui->txtBoldColor->setText(Config::getInstance().get(Config::getInstance().BOLD));
    ui->txtItalicColor->setText(Config::getInstance().get(Config::getInstance().ITALIC));
    ui->txtCommentsColor->setText(Config::getInstance().get(Config::getInstance().COMMENT));
    ui->txtHeadersColor->setText(Config::getInstance().get(Config::getInstance().HEADER));
    ui->txtSThroughColor->setText(Config::getInstance().get(Config::getInstance().STRIKETHROUGH));
    ui->txtSymbolsColor->setText(Config::getInstance().get(Config::getInstance().SYMBOL));
    ui->txtTagsColor->setText(Config::getInstance().get(Config::getInstance().TAG));
}

QColor DisplayConfigDialog::pickColor(QColor &color) {
    QColor newColor = QColorDialog::getColor(color, this, "Pick a color");
    if (!newColor.isValid()) {
        return color;
    }
    return newColor;
}

void DisplayConfigDialog::on_btnFontChange_clicked() {
    bool changed;
    QFont font = QFontDialog::getFont(&changed, ui->txtFont->font());
    if (changed) {
        Config::getInstance().set(Config::getInstance().FONT, font.toString());
        ui->txtFont->setText(font.family());
    }
}

void DisplayConfigDialog::on_btnCommentsColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().COMMENT));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().COMMENT, color.name());
    ui->txtCommentsColor->setText(color.name());
}

void DisplayConfigDialog::on_btnFontColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().FOREGROUND));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().FOREGROUND, color.name());
    ui->txtFontColor->setText(color.name());
}

void DisplayConfigDialog::on_btnBgColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().BACKGROUND));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().BACKGROUND, color.name());
    ui->txtBgColor->setText(color.name());
}

void DisplayConfigDialog::on_btnItalicColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().ITALIC));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().ITALIC, color.name());
    ui->txtItalicColor->setText(color.name());
}

void DisplayConfigDialog::on_btnBoldColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().BOLD));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().BOLD, color.name());
    ui->txtBoldColor->setText(color.name());
}

void DisplayConfigDialog::on_btnSThroughColorColor_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().STRIKETHROUGH));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().STRIKETHROUGH, color.name());
    ui->txtSThroughColor->setText(color.name());
}

void DisplayConfigDialog::on_btnHeadersColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().HEADER));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().HEADER, color.name());
    ui->txtHeadersColor->setText(color.name());
}

void DisplayConfigDialog::on_btnBQuotesColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().BLOCKQUOTE));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().BLOCKQUOTE, color.name());
    ui->txtBQuotesColor->setText(color.name());
}

void DisplayConfigDialog::on_btnCodeColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().CODE));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().CODE, color.name());
    ui->txtCodesColor->setText(color.name());
}

void DisplayConfigDialog::on_btnSymbolsColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().SYMBOL));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().SYMBOL, color.name());
    ui->txtSymbolsColor->setText(color.name());
}

void DisplayConfigDialog::on_btnTagsColorChange_clicked() {
    QColor color(Config::getInstance().get(Config::getInstance().TAG));
    color = pickColor(color);
    Config::getInstance().set(Config::getInstance().TAG, color.name());
    ui->txtTagsColor->setText(color.name());
}


