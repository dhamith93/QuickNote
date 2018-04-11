#include "headers/newnotewindow.h"
#include "ui_newnotewindow.h"
#include <QtWidgets>
#include "stringparser.cpp"
#include "headers/cst_string.h"
#include <fstream>
#include <QCloseEvent>

#ifdef _WIN32
    #define PANDOC_PATH "cd C:\\\"Program Files (x86)\"\\Pandoc\\ && pandoc.exe "
    #define WKHTMLTO_PATH "C:\\\"Program Files\"\\wkhtmltopdf\\bin\\wkhtmltopdf.exe "
    #define RM_COMMAND "del"
#elif __linux__
    #define PANDOC_PATH "pandoc"
    #define WKHTMLTO_PATH "wkhtmltopdf"
    #define RM_COMMAND "rm"
#else
    #define PANDOC_PATH "/usr/local/bin/pandoc"
    #define WKHTMLTO_PATH "/usr/local/bin/wkhtmltopdf --lowquality "
    #define RM_COMMAND "rm"
#endif

NewNoteWindow::NewNoteWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewNoteWindow) {
    ui->setupUi(this);
}

NewNoteWindow::NewNoteWindow(QWidget *parent, string filePath) :
    QMainWindow(parent),
    ui(new Ui::NewNoteWindow) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->previewWindow->hide();
    if (db.colorConfigExists()) {
        QString background = db.getColorConfig("background");
        QString color = db.getColorConfig("font");
        ui->txtInput->setStyleSheet("QPlainTextEdit { background-color: " + background + "; color:" + color + "; }");
    } else {
        ui->txtInput->setStyleSheet("QPlainTextEdit { background-color: black; color: white; }");
    }
    if (db.fontConfigExists()) {
        QFont font;
        font.fromString(db.getFontConfig());
        ui->txtInput->setFont(font);
    }
    if (!filePath.empty()) {
        fromOpen = true;        
        fileName = QString::fromStdString(filePath);
        setWindowTitle(fileName);
        ifstream file(filePath);
        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        file.close();
        ui->txtInput->setPlainText(QString::fromStdString(content));
        setText(content);        
        if (!db.recentNoteExists(fileName)) {
            if (db.checkRowCountEq(10)) {
                db.deleteOldest();
            }
            db.insertNote(fileName);
        }
    } else {
        setWindowModified(true); // Sets 'content modified' on macos. It prompts save window on exit
        string input = ui->txtInput->toPlainText().toUtf8().constData();
        if (input.size() > 0) {
            setText(input);
        }
    }
}

void NewNoteWindow::on_txtInput_textChanged() {
    // change count needed to make sure opening files doesn't show up as unsaved work
    // as loading text from file triggers txtInput_textChanged(). So first change is ignored
    changeCount += 1;
    fileSaved = (fromOpen) ? (changeCount == 1) ? true : false : false;
    if (!fileSaved) {
        setWindowModified(true);
    }
    string input = ui->txtInput->toPlainText().toUtf8().constData();
    // Tags. if first line is starting and ending with '%', this split tags and fills tagArr
    if (input.size() > 0 && input.at(0) == '%') {
        CstString CstStr;
        vector<string> strArr = CstStr.split(input, '\n', 2);
        string firstLine = strArr[0];
        int length = firstLine.length();
        if (length > 1) {
            if (firstLine.at(length - 1) == '%') {
                if (tagLine != firstLine) {
                    string str = CstStr.subStrBetween(strArr[0], "%", "%");
                    tagArr = CstStr.split(str, ' ');
                }
                input = (strArr.size() > 1) ? input.substr(length + 1, input.length()) : "";
            }
        }
    }
    if (input.size() > 0 && ui->actionPreview->isChecked()) {
        setText(input);
        setScrollPosition();
    }
}

void NewNoteWindow::on_txtInput_cursorPositionChanged() {
    setScrollPosition();
}

void NewNoteWindow::on_actionSave_triggered() {
    saveFile();
}

void NewNoteWindow::on_actionHTML_triggered() {
    saveHtml();
}

void NewNoteWindow::on_actionPDF_triggered() {
    QString qsPath = QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(), tr("PDF (*.pdf)"));
    if (!qsPath.isEmpty()) {
        QString qsTempPath = tempSaveHtml();
        qsPath = QDir::toNativeSeparators(qsPath);
        qsTempPath = QDir::toNativeSeparators(qsTempPath);
        string path = qsPath.toUtf8().constData();
        path = "\"" + path + "\"";
        string tempPath = qsTempPath.toUtf8().constData();
        tempPath = "\"" + tempPath + "\"";
        string command = WKHTMLTO_PATH + tempPath + " " + path;
        system(command.c_str());
    }
}

void NewNoteWindow::on_actionDOCX_triggered() {
    QString qsPath = QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(), tr("docx (*.docx)"));
    if (!qsPath.isEmpty()) {
        QString qsTempPath = qsPath + QString::fromStdString(".md");
        qsTempPath = QDir::toNativeSeparators(qsTempPath);
        qsPath = QDir::toNativeSeparators(qsPath);
        string path = qsPath.toUtf8().constData();
        string tempPath = qsTempPath.toUtf8().constData();
        saveFile(tempPath);
        tempPath = "\"" + tempPath + "\"";
        path = " \"" + path + "\"";
        try {
            string command = (string)PANDOC_PATH + " -s " + tempPath + " -o " + path;
            system(command.c_str());
            command = (string)RM_COMMAND + " " + tempPath;
            system(command.c_str());
        } catch (exception ex) {
            QMessageBox msgBox;
            msgBox.setText("There was an error! Do you have pandoc installed?");
            msgBox.exec();
        }
    }
}

void NewNoteWindow::on_actionChange_Font_triggered() {
    bool changed;
    QFont font = QFontDialog::getFont(&changed);
    if (changed) {
        db.insertFontConfig(font.toString());
        ui->txtInput->setFont(font);
    }
}

void NewNoteWindow::on_actionPreview_changed() {
    if (ui->actionPreview->isChecked()) {
        string input = ui->txtInput->toPlainText().toUtf8().constData();
        if (input.size() > 0) {
            // If there is a tag line at the firts line, it gets remove from the preview
            if (input.at(0) == '%') {
                CstString CstStr;
                vector<string> strArr = CstStr.split(input, '\n', 2);
                string firstLine = strArr[0];
                int length = firstLine.length();
                if (length > 1) {
                    if (firstLine.at(length - 1) == '%') {
                        input = (strArr.size() > 1) ? input.substr(length + 1, input.length()) : "";
                    }
                }
            }
            setText(input);
            setScrollPosition();
        }
        ui->previewWindow->show();
        int w = this->width();
        this->setFixedWidth(w * 2);
    } else {
        ui->previewWindow->hide();
        int w = this->width();
        this->setFixedWidth(w / 2);
    }
    this->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    this->setMinimumSize(0,0);
}

void NewNoteWindow::on_actionPretty_Preview_triggered() {
    QString path = "file:///" + tempSaveHtml();
    QDesktopServices::openUrl(QUrl(path));
}

void NewNoteWindow::on_actionSet_Background_Color_triggered() {
    QColor color = QColorDialog::getColor(Qt::black, this, "Pick a background color",  QColorDialog::DontUseNativeDialog);
    if (color.isValid()) {
        setColor(color, "background");
    }
}

void NewNoteWindow::on_actionSet_Font_Color_triggered() {
    QColor color = QColorDialog::getColor(Qt::white, this, "Pick a font color",  QColorDialog::DontUseNativeDialog);
    if (color.isValid()) {
        setColor(color, "font");
    }
}

void NewNoteWindow::closeEvent (QCloseEvent *event) {
    if (!fileSaved) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(
            this, "",
            tr("Do you want to save the file?\n"),
            QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
            QMessageBox::Yes
        );
        if (resBtn == QMessageBox::Yes) {
            event->ignore();
            saveFile();
            event->accept();            
        } else if (resBtn == QMessageBox::Cancel) {
            event->ignore();
        } else {
            event->accept();
            this->close();
        }
    }
}

void NewNoteWindow::setText(string &content) {
    StringParser parser;
    parser.parse(content);
    vector<HtmlElement> elements = parser.getElements();
    string htmlText = "<!DOCTYPE html>\n<html><head></head>\n<body><style>.markdown-body {box-sizing: border-box;max-width: 980px;"
                      "margin: 0 auto;}</style><article class=\"markdown-body\">\n";
    for (auto& element : elements) {
        htmlText += element.str();
    }
    htmlText += "</article></body>\n</html>";
    QString qstr = QString::fromStdString(htmlText);
    ui->previewWindow->setHtml(qstr);
}

void NewNoteWindow::setScrollPosition() {
    QTextCursor  cursor = ui->txtInput->textCursor();
    ui->txtInput->setTextCursor( cursor );
    int currPos = cursor.blockNumber();
    int docHeight = ui->previewWindow->document()->size().height();
    int lineCount = ui->txtInput->blockCount();
    ui->previewWindow->verticalScrollBar()->setValue(currPos * (docHeight / lineCount));
}

void NewNoteWindow::saveFile() {
    if (!fromOpen) {
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath(), tr("Markdown (*.md)"));
    }
    try {        
        if (!fileName.isEmpty()) {
            ofstream out(fileName.toUtf8().constData());
            string output = ui->txtInput->toPlainText().toUtf8().constData();
            out << output;
            out.close();
            fileSaved = true;
            fromOpen = true;
            setWindowModified(false);
            setWindowTitle(fileName);
            if (!db.recentNoteExists(fileName)) {
                if (db.checkRowCountEq(10)) {
                    db.deleteOldest();
                }
                db.insertNote(fileName);
            }
            if (!db.taggedNoteExists(fileName)) {
                if (tagArr.size() > 0) {
                    db.insertNoteWithTags(fileName, tagArr);
                }
            } else {
                db.deleteTags(db.getRowId(fileName.toUtf8().constData()));
                if (tagArr.size() > 0) {
                    db.updateTags(fileName, tagArr);
                }
            }
        }
    } catch (exception ex) {
        QMessageBox msgBox;
        msgBox.setText("There was an error! please try again.");
        msgBox.exec();
    }
}

void NewNoteWindow::saveFile(string path) {
    ofstream out(path);
    string output = ui->txtInput->toPlainText().toUtf8().constData();
    out << output;
    out.close();
}

void NewNoteWindow::saveHtml() {
    string style = ".markdown-body hr::after,.markdown-body::after{clear:both}.markdown-body{-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%;color:#24292e;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Helvetica,Arial,sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\";font-size:16px;line-height:1.5;word-wrap:break-word}.markdown-body .pl-c{color:#6a737d}.markdown-body .pl-c1,.markdown-body .pl-s .pl-v{color:#005cc5}.markdown-body .pl-e,.markdown-body .pl-en{color:#6f42c1}.markdown-body .pl-s .pl-s1,.markdown-body .pl-smi{color:#24292e}.markdown-body .pl-ent{color:#22863a}.markdown-body .pl-k{color:#d73a49}.markdown-body .pl-pds,.markdown-body .pl-s,.markdown-body .pl-s .pl-pse .pl-s1,.markdown-body .pl-sr,.markdown-body .pl-sr .pl-cce,.markdown-body .pl-sr .pl-sra,.markdown-body .pl-sr .pl-sre{color:#032f62}.markdown-body .pl-smw,.markdown-body .pl-v{color:#e36209}.markdown-body .pl-bu{color:#b31d28}.markdown-body .pl-ii{color:#fafbfc;background-color:#b31d28}.markdown-body .pl-c2{color:#fafbfc;background-color:#d73a49}.markdown-body .pl-c2::before{content:\"^M\"}.markdown-body .pl-sr .pl-cce{font-weight:700;color:#22863a}.markdown-body .pl-ml{color:#735c0f}.markdown-body .pl-mh,.markdown-body .pl-mh .pl-en,.markdown-body .pl-ms{font-weight:700;color:#005cc5}.markdown-body .pl-mi{font-style:italic;color:#24292e}.markdown-body .pl-mb{font-weight:700;color:#24292e}.markdown-body .pl-md{color:#b31d28;background-color:#ffeef0}.markdown-body .pl-mi1{color:#22863a;background-color:#f0fff4}.markdown-body .pl-mc{color:#e36209;background-color:#ffebda}.markdown-body .pl-mi2{color:#f6f8fa;background-color:#005cc5}.markdown-body .pl-mdr{font-weight:700;color:#6f42c1}.markdown-body .pl-ba{color:#586069}.markdown-body .pl-sg{color:#959da5}.markdown-body .pl-corl{text-decoration:underline;color:#032f62}.markdown-body .octicon{display:inline-block;fill:currentColor;vertical-align:text-bottom}.markdown-body hr::after,.markdown-body hr::before,.markdown-body::after,.markdown-body::before{display:table;content:\"\"}.markdown-body a{background-color:transparent;color:#0366d6;text-decoration:none}.markdown-body a:active,.markdown-body a:hover{outline-width:0}.markdown-body h1{margin:.67em 0}.markdown-body img{border-style:none}.markdown-body hr{box-sizing:content-box}.markdown-body input{font:inherit;margin:0;overflow:visible;font-family:inherit;font-size:inherit;line-height:inherit}.markdown-body dl dt,.markdown-body strong,.markdown-body table th{font-weight:600}.markdown-body code,.markdown-body pre{font-family:SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace}.markdown-body [type=checkbox]{box-sizing:border-box;padding:0}.markdown-body *{box-sizing:border-box}.markdown-body a:hover{text-decoration:underline}.markdown-body td,.markdown-body th{padding:0}.markdown-body blockquote{margin:0}.markdown-body ol ol,.markdown-body ul ol{list-style-type:lower-roman}.markdown-body ol ol ol,.markdown-body ol ul ol,.markdown-body ul ol ol,.markdown-body ul ul ol{list-style-type:lower-alpha}.markdown-body dd{margin-left:0}.markdown-body .pl-0{padding-left:0!important}.markdown-body .pl-1{padding-left:4px!important}.markdown-body .pl-2{padding-left:8px!important}.markdown-body .pl-3{padding-left:16px!important}.markdown-body .pl-4{padding-left:24px!important}.markdown-body .pl-5{padding-left:32px!important}.markdown-body .pl-6{padding-left:40px!important}.markdown-body>:first-child{margin-top:0!important}.markdown-body>:last-child{margin-bottom:0!important}.markdown-body a:not([href]){color:inherit;text-decoration:none}.markdown-body .anchor{float:left;padding-right:4px;margin-left:-20px;line-height:1}.markdown-body .anchor:focus{outline:0}.markdown-body blockquote,.markdown-body dl,.markdown-body ol,.markdown-body p,.markdown-body pre,.markdown-body table,.markdown-body ul{margin-top:0;margin-bottom:16px}.markdown-body hr{overflow:hidden;background:#e1e4e8;height:.25em;padding:0;margin:24px 0;border:0}.markdown-body blockquote{padding:0 1em;color:#6a737d;border-left:.25em solid #dfe2e5}.markdown-body h1,.markdown-body h2{padding-bottom:.3em;border-bottom:1px solid #eaecef}.markdown-body blockquote>:first-child{margin-top:0}.markdown-body blockquote>:last-child{margin-bottom:0}.markdown-body h1,.markdown-body h2,.markdown-body h3,.markdown-body h4,.markdown-body h5,.markdown-body h6{margin-top:24px;margin-bottom:16px;font-weight:600;line-height:1.25}.markdown-body h1 .octicon-link,.markdown-body h2 .octicon-link,.markdown-body h3 .octicon-link,.markdown-body h4 .octicon-link,.markdown-body h5 .octicon-link,.markdown-body h6 .octicon-link{color:#1b1f23;vertical-align:middle;visibility:hidden}.markdown-body h1:hover .anchor,.markdown-body h2:hover .anchor,.markdown-body h3:hover .anchor,.markdown-body h4:hover .anchor,.markdown-body h5:hover .anchor,.markdown-body h6:hover .anchor{text-decoration:none}.markdown-body h1:hover .anchor .octicon-link,.markdown-body h2:hover .anchor .octicon-link,.markdown-body h3:hover .anchor .octicon-link,.markdown-body h4:hover .anchor .octicon-link,.markdown-body h5:hover .anchor .octicon-link,.markdown-body h6:hover .anchor .octicon-link{visibility:visible}.markdown-body h1{font-size:2em}.markdown-body h2{font-size:1.5em}.markdown-body h3{font-size:1.25em}.markdown-body h4{font-size:1em}.markdown-body h5{font-size:.875em}.markdown-body h6{font-size:.85em;color:#6a737d}.markdown-body ol,.markdown-body ul{padding-left:2em}.markdown-body ol ol,.markdown-body ol ul,.markdown-body ul ol,.markdown-body ul ul{margin-top:0;margin-bottom:0}.markdown-body li{word-wrap:break-all}.markdown-body li>p{margin-top:16px}.markdown-body li+li{margin-top:.25em}.markdown-body dl{padding:0}.markdown-body dl dt{padding:0;margin-top:16px;font-size:1em;font-style:italic}.markdown-body dl dd{padding:0 16px;margin-bottom:16px}.markdown-body table{border-spacing:0;border-collapse:collapse;display:block;width:100%;overflow:auto}.markdown-body table td,.markdown-body table th{padding:6px 13px;border:1px solid #dfe2e5}.markdown-body table tr{background-color:#fff;border-top:1px solid #c6cbd1}.markdown-body table tr:nth-child(2n){background-color:#f6f8fa}.markdown-body img{max-width:100%;box-sizing:content-box;background-color:#fff}.markdown-body img[align=right]{padding-left:20px}.markdown-body img[align=left]{padding-right:20px}.markdown-body code{padding:.2em .4em;margin:0;font-size:85%;background-color:rgba(27,31,35,.05);border-radius:3px}.markdown-body pre{word-wrap:normal}.markdown-body pre>code{padding:0;margin:0;font-size:100%;word-break:normal;white-space:pre;background:0 0;border:0}.markdown-body .highlight{margin-bottom:16px}.markdown-body .highlight pre{margin-bottom:0;word-break:normal}.markdown-body .highlight pre,.markdown-body pre{padding:16px;overflow:auto;font-size:85%;line-height:1.45;background-color:#f6f8fa;border-radius:3px}.markdown-body pre code{display:inline;max-width:auto;padding:0;margin:0;overflow:visible;line-height:inherit;word-wrap:normal;background-color:transparent;border:0}.markdown-body .full-commit .btn-outline:not(:disabled):hover{color:#005cc5;border-color:#005cc5}.markdown-body kbd{display:inline-block;padding:3px 5px;font:11px SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace;line-height:10px;color:#444d56;vertical-align:middle;background-color:#fafbfc;border:1px solid #d1d5da;border-bottom-color:#c6cbd1;border-radius:3px;box-shadow:inset 0 -1px 0 #c6cbd1}.markdown-body :checked+.radio-label{position:relative;z-index:1;border-color:#0366d6}.markdown-body .task-list-item{list-style-type:none}.markdown-body .task-list-item+.task-list-item{margin-top:3px}.markdown-body .task-list-item input{margin:0 .2em .25em -1.6em;vertical-align:middle}.markdown-body hr{border-bottom-color:#eee}";
    QString exportName = QFileDialog::getSaveFileName(this, tr("Export HTML"), QDir::homePath(), tr("HTML (*.html *.htm)"));
    if (!exportName.isEmpty()) {
        string content = ui->txtInput->toPlainText().toUtf8().constData();
        if (!content.empty()) {
            if (content.at(0) == '%') {
                CstString CstStr;
                vector<string> strArr = CstStr.split(content, '\n', 2);
                string firstLine = strArr[0];
                int length = firstLine.length();
                if (length > 1) {
                    if (firstLine.at(length - 1) == '%') {
                        content = (strArr.size() > 1) ? content.substr(length + 1, content.length()) : "";
                    }
                }
            }
            StringParser parser;
            parser.parse(content);
            vector<HtmlElement> elements = parser.getElements();
            string htmlText = "<!DOCTYPE html>\n<html><head></head>\n<body><style>.markdown-body {box-sizing: border-box;max-width: 980px;margin: 0 auto;} " + style + " </style><link rel=\"stylesheet\" href=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css\"><article class=\"markdown-body\">\n";
            for (auto&& element : elements) {
                htmlText += element.str();
            }
            htmlText += "</article><script src=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js\"></script><script>hljs.initHighlightingOnLoad();</script></body></html>";
            try {
                ofstream out(exportName.toUtf8().constData());
                out << htmlText;
                out.close();
                QMessageBox msgBox;
                msgBox.setText("HTML file exported successfully!");
                msgBox.exec();
            } catch (exception ex) {
                QMessageBox msgBox;
                msgBox.setText("There was an error! please try again.");
                msgBox.exec();
            }
        }
    }
}

QString NewNoteWindow::tempSaveHtml() {
    string content = ui->txtInput->toPlainText().toUtf8().constData();
    if (!content.empty()) {
        QString path = QDir::tempPath() + "/temp_save.html";
        path = QDir::toNativeSeparators(path);
        string style = ".markdown-body hr::after,.markdown-body::after{clear:both}.markdown-body{-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%;color:#24292e;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Helvetica,Arial,sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\";font-size:16px;line-height:1.5;word-wrap:break-word}.markdown-body .pl-c{color:#6a737d}.markdown-body .pl-c1,.markdown-body .pl-s .pl-v{color:#005cc5}.markdown-body .pl-e,.markdown-body .pl-en{color:#6f42c1}.markdown-body .pl-s .pl-s1,.markdown-body .pl-smi{color:#24292e}.markdown-body .pl-ent{color:#22863a}.markdown-body .pl-k{color:#d73a49}.markdown-body .pl-pds,.markdown-body .pl-s,.markdown-body .pl-s .pl-pse .pl-s1,.markdown-body .pl-sr,.markdown-body .pl-sr .pl-cce,.markdown-body .pl-sr .pl-sra,.markdown-body .pl-sr .pl-sre{color:#032f62}.markdown-body .pl-smw,.markdown-body .pl-v{color:#e36209}.markdown-body .pl-bu{color:#b31d28}.markdown-body .pl-ii{color:#fafbfc;background-color:#b31d28}.markdown-body .pl-c2{color:#fafbfc;background-color:#d73a49}.markdown-body .pl-c2::before{content:\"^M\"}.markdown-body .pl-sr .pl-cce{font-weight:700;color:#22863a}.markdown-body .pl-ml{color:#735c0f}.markdown-body .pl-mh,.markdown-body .pl-mh .pl-en,.markdown-body .pl-ms{font-weight:700;color:#005cc5}.markdown-body .pl-mi{font-style:italic;color:#24292e}.markdown-body .pl-mb{font-weight:700;color:#24292e}.markdown-body .pl-md{color:#b31d28;background-color:#ffeef0}.markdown-body .pl-mi1{color:#22863a;background-color:#f0fff4}.markdown-body .pl-mc{color:#e36209;background-color:#ffebda}.markdown-body .pl-mi2{color:#f6f8fa;background-color:#005cc5}.markdown-body .pl-mdr{font-weight:700;color:#6f42c1}.markdown-body .pl-ba{color:#586069}.markdown-body .pl-sg{color:#959da5}.markdown-body .pl-corl{text-decoration:underline;color:#032f62}.markdown-body .octicon{display:inline-block;fill:currentColor;vertical-align:text-bottom}.markdown-body hr::after,.markdown-body hr::before,.markdown-body::after,.markdown-body::before{display:table;content:\"\"}.markdown-body a{background-color:transparent;color:#0366d6;text-decoration:none}.markdown-body a:active,.markdown-body a:hover{outline-width:0}.markdown-body h1{margin:.67em 0}.markdown-body img{border-style:none}.markdown-body hr{box-sizing:content-box}.markdown-body input{font:inherit;margin:0;overflow:visible;font-family:inherit;font-size:inherit;line-height:inherit}.markdown-body dl dt,.markdown-body strong,.markdown-body table th{font-weight:600}.markdown-body code,.markdown-body pre{font-family:SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace}.markdown-body [type=checkbox]{box-sizing:border-box;padding:0}.markdown-body *{box-sizing:border-box}.markdown-body a:hover{text-decoration:underline}.markdown-body td,.markdown-body th{padding:0}.markdown-body blockquote{margin:0}.markdown-body ol ol,.markdown-body ul ol{list-style-type:lower-roman}.markdown-body ol ol ol,.markdown-body ol ul ol,.markdown-body ul ol ol,.markdown-body ul ul ol{list-style-type:lower-alpha}.markdown-body dd{margin-left:0}.markdown-body .pl-0{padding-left:0!important}.markdown-body .pl-1{padding-left:4px!important}.markdown-body .pl-2{padding-left:8px!important}.markdown-body .pl-3{padding-left:16px!important}.markdown-body .pl-4{padding-left:24px!important}.markdown-body .pl-5{padding-left:32px!important}.markdown-body .pl-6{padding-left:40px!important}.markdown-body>:first-child{margin-top:0!important}.markdown-body>:last-child{margin-bottom:0!important}.markdown-body a:not([href]){color:inherit;text-decoration:none}.markdown-body .anchor{float:left;padding-right:4px;margin-left:-20px;line-height:1}.markdown-body .anchor:focus{outline:0}.markdown-body blockquote,.markdown-body dl,.markdown-body ol,.markdown-body p,.markdown-body pre,.markdown-body table,.markdown-body ul{margin-top:0;margin-bottom:16px}.markdown-body hr{overflow:hidden;background:#e1e4e8;height:.25em;padding:0;margin:24px 0;border:0}.markdown-body blockquote{padding:0 1em;color:#6a737d;border-left:.25em solid #dfe2e5}.markdown-body h1,.markdown-body h2{padding-bottom:.3em;border-bottom:1px solid #eaecef}.markdown-body blockquote>:first-child{margin-top:0}.markdown-body blockquote>:last-child{margin-bottom:0}.markdown-body h1,.markdown-body h2,.markdown-body h3,.markdown-body h4,.markdown-body h5,.markdown-body h6{margin-top:24px;margin-bottom:16px;font-weight:600;line-height:1.25}.markdown-body h1 .octicon-link,.markdown-body h2 .octicon-link,.markdown-body h3 .octicon-link,.markdown-body h4 .octicon-link,.markdown-body h5 .octicon-link,.markdown-body h6 .octicon-link{color:#1b1f23;vertical-align:middle;visibility:hidden}.markdown-body h1:hover .anchor,.markdown-body h2:hover .anchor,.markdown-body h3:hover .anchor,.markdown-body h4:hover .anchor,.markdown-body h5:hover .anchor,.markdown-body h6:hover .anchor{text-decoration:none}.markdown-body h1:hover .anchor .octicon-link,.markdown-body h2:hover .anchor .octicon-link,.markdown-body h3:hover .anchor .octicon-link,.markdown-body h4:hover .anchor .octicon-link,.markdown-body h5:hover .anchor .octicon-link,.markdown-body h6:hover .anchor .octicon-link{visibility:visible}.markdown-body h1{font-size:2em}.markdown-body h2{font-size:1.5em}.markdown-body h3{font-size:1.25em}.markdown-body h4{font-size:1em}.markdown-body h5{font-size:.875em}.markdown-body h6{font-size:.85em;color:#6a737d}.markdown-body ol,.markdown-body ul{padding-left:2em}.markdown-body ol ol,.markdown-body ol ul,.markdown-body ul ol,.markdown-body ul ul{margin-top:0;margin-bottom:0}.markdown-body li{word-wrap:break-all}.markdown-body li>p{margin-top:16px}.markdown-body li+li{margin-top:.25em}.markdown-body dl{padding:0}.markdown-body dl dt{padding:0;margin-top:16px;font-size:1em;font-style:italic}.markdown-body dl dd{padding:0 16px;margin-bottom:16px}.markdown-body table{border-spacing:0;border-collapse:collapse;display:block;width:100%;overflow:auto}.markdown-body table td,.markdown-body table th{padding:6px 13px;border:1px solid #dfe2e5}.markdown-body table tr{background-color:#fff;border-top:1px solid #c6cbd1}.markdown-body table tr:nth-child(2n){background-color:#f6f8fa}.markdown-body img{max-width:100%;box-sizing:content-box;background-color:#fff}.markdown-body img[align=right]{padding-left:20px}.markdown-body img[align=left]{padding-right:20px}.markdown-body code{padding:.2em .4em;margin:0;font-size:85%;background-color:rgba(27,31,35,.05);border-radius:3px}.markdown-body pre{word-wrap:normal}.markdown-body pre>code{padding:0;margin:0;font-size:100%;word-break:normal;white-space:pre;background:0 0;border:0}.markdown-body .highlight{margin-bottom:16px}.markdown-body .highlight pre{margin-bottom:0;word-break:normal}.markdown-body .highlight pre,.markdown-body pre{padding:16px;overflow:auto;font-size:85%;line-height:1.45;background-color:#f6f8fa;border-radius:3px}.markdown-body pre code{display:inline;max-width:auto;padding:0;margin:0;overflow:visible;line-height:inherit;word-wrap:normal;background-color:transparent;border:0}.markdown-body .full-commit .btn-outline:not(:disabled):hover{color:#005cc5;border-color:#005cc5}.markdown-body kbd{display:inline-block;padding:3px 5px;font:11px SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace;line-height:10px;color:#444d56;vertical-align:middle;background-color:#fafbfc;border:1px solid #d1d5da;border-bottom-color:#c6cbd1;border-radius:3px;box-shadow:inset 0 -1px 0 #c6cbd1}.markdown-body :checked+.radio-label{position:relative;z-index:1;border-color:#0366d6}.markdown-body .task-list-item{list-style-type:none}.markdown-body .task-list-item+.task-list-item{margin-top:3px}.markdown-body .task-list-item input{margin:0 .2em .25em -1.6em;vertical-align:middle}.markdown-body hr{border-bottom-color:#eee}";
        if (content.at(0) == '%') {
            CstString CstStr;
            vector<string> strArr = CstStr.split(content, '\n', 2);
            string firstLine = strArr[0];
            int length = firstLine.length();
            if (length > 1) {
                if (firstLine.at(length - 1) == '%') {
                    content = (strArr.size() > 1) ? content.substr(length + 1, content.length()) : "";
                }
            }
        }
        StringParser parser;
        parser.parse(content);
        vector<HtmlElement> elements = parser.getElements();
        string htmlText = "<!DOCTYPE html>\n<html><head></head>\n<body><style>.markdown-body {box-sizing: border-box;max-width: 980px;margin: 0 auto;} " + style + " </style><link rel=\"stylesheet\" href=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css\"><article class=\"markdown-body\">\n";
        for (auto&& element : elements) {
            htmlText += element.str();
        }
        htmlText += "</article><script src=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js\"></script><script>hljs.initHighlightingOnLoad();</script></body></html>";
        ofstream out(path.toUtf8().constData());
        out << htmlText;
        out.close();
        return path;
    }
    return "";
}

void NewNoteWindow::setColor(QColor color1, QString type) {
    QPalette palette = ui->txtInput->palette();
    QColor color2;
    QString style;
    if (type == "background") {
        color2 = palette.color(QPalette::WindowText);
        style = "QPlainTextEdit { background-color: " + color1.name(QColor::HexRgb) + "; color: " + color2.name(QColor::HexRgb) + "; }";
    } else {
        color2 = palette.color(QPalette::Window);
        style = "QPlainTextEdit { background-color: " + color2.name(QColor::HexRgb) + "; color: " + color1.name(QColor::HexRgb) + "; }";
    }
    ui->txtInput->setStyleSheet(style);
    db.insertColorConfig(color1.name(QColor::HexRgb), color2.name(QColor::HexRgb), type);
}

NewNoteWindow::~NewNoteWindow() {
    delete ui;
}
