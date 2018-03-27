#include "newwebenginenotewindow.h"
#include "ui_newwebenginenotewindow.h"
#include <QtWidgets>
#include "stringparser.cpp"
#include "database.cpp"
#include <fstream>
#include <QCloseEvent>

#ifdef _WIN32
    #define PANDOC_PATH "pandoc.exe"
    #define RM_COMMAND "del"
    #define OS "windows"
#elif __linux__
    #define PANDOC_PATH "pandoc"
    #define RM_COMMAND "rm"
    #define OS "linux"
#else
    #define PANDOC_PATH "/usr/local/bin/pandoc"
    #define RM_COMMAND "rm"
    #define OS "macos"
#endif

NewWebEngineNoteWindow::NewWebEngineNoteWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewWebEngineNoteWindow) {
    ui->setupUi(this);    
}

NewWebEngineNoteWindow::NewWebEngineNoteWindow(QWidget *parent, string filePath) :
    QMainWindow(parent),
    ui(new Ui::NewWebEngineNoteWindow) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->txtInput->setStyleSheet("background-color: black; color:white;");
    ui->previewWindow->hide();
    ui->actionPreview->setChecked(false);
    Database db;
    if (db.fontConfigExists()) {
        QFont font;
        font.fromString(db.getFontConfig());
        ui->txtInput->setFont(font);
    }
    if (filePath != "") {
        fromOpen = true;
        fileName = QString::fromStdString(filePath);
        setWindowTitle(fileName);
        ifstream file(filePath);
        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        file.close();
        ui->txtInput->setPlainText(QString::fromStdString(content));
        if (!db.checkIfExists(fileName)) {
            if (db.checkRowCountEq(10)) {
                db.deleteOldest();
            }
            db.insertNote(fileName);
        }
    }
}

void NewWebEngineNoteWindow::on_txtInput_textChanged() {
    changeCount += 1;
    fileSaved = (fromOpen) ? (changeCount == 1) ? true : false : false;
    string input = ui->txtInput->toPlainText().toUtf8().constData();
    if (input.size() > 0 && ui->actionPreview->isChecked()) {
        setText(input);
    }
}

void NewWebEngineNoteWindow::on_actionSave_triggered() {
    saveFile();
}

void NewWebEngineNoteWindow::on_actionHTML_triggered() {
    saveHtml();
}

void NewWebEngineNoteWindow::on_actionDOCX_triggered() {
    QString qsPath = QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(), tr("docx (*.docx)"));
    QString qsTempPath = qsPath + QString::fromStdString(".md");
    string path = qsPath.toUtf8().constData();
    string tempPath = qsTempPath.toUtf8().constData();
    saveFile(tempPath);
    tempPath = "\"" + tempPath + "\"";
    path = " \"" + path + "\"";
    string command = (string)PANDOC_PATH + " -s " + tempPath + " -o" + path;
    try {
        if (path != "") {
            system(command.c_str());
            if (OS == "windows") {
                qsTempPath = QDir::toNativeSeparators(qsTempPath);
                tempPath = qsTempPath.toUtf8().constData();
                tempPath = "\"" + tempPath + "\"";
            }
            command = (string)RM_COMMAND + " " + tempPath;
            system(command.c_str());
        }
    } catch (exception ex) {
        QMessageBox msgBox;
        msgBox.setText("There was an error! Do you have pandoc installed?");
        msgBox.exec();
    }
}

void NewWebEngineNoteWindow::on_actionChange_Font_triggered() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok);
    Database db;
    db.insertFontConfig(font.toString());
    ui->txtInput->setFont(font);
}

void NewWebEngineNoteWindow::on_actionPreview_changed() {
    if (ui->actionPreview->isChecked()) {
        string input = ui->txtInput->toPlainText().toUtf8().constData();
        if (input.size() > 0) {
            setText(input);
        }
        ui->previewWindow->show();
    } else {
        ui->previewWindow->hide();
    }
}

void NewWebEngineNoteWindow::closeEvent (QCloseEvent *event) {
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
        }
    }
    ((QWidget*)parent())->show();
}

void NewWebEngineNoteWindow::setText(string &content) {
    StringParser parser;
    parser.parse(content);
    vector<HtmlElement> elements = parser.getElements();
    string style = ".markdown-body hr::after,.markdown-body::after{clear:both}.markdown-body{-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%;color:#24292e;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Helvetica,Arial,sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\";font-size:16px;line-height:1.5;word-wrap:break-word}.markdown-body .pl-c{color:#6a737d}.markdown-body .pl-c1,.markdown-body .pl-s .pl-v{color:#005cc5}.markdown-body .pl-e,.markdown-body .pl-en{color:#6f42c1}.markdown-body .pl-s .pl-s1,.markdown-body .pl-smi{color:#24292e}.markdown-body .pl-ent{color:#22863a}.markdown-body .pl-k{color:#d73a49}.markdown-body .pl-pds,.markdown-body .pl-s,.markdown-body .pl-s .pl-pse .pl-s1,.markdown-body .pl-sr,.markdown-body .pl-sr .pl-cce,.markdown-body .pl-sr .pl-sra,.markdown-body .pl-sr .pl-sre{color:#032f62}.markdown-body .pl-smw,.markdown-body .pl-v{color:#e36209}.markdown-body .pl-bu{color:#b31d28}.markdown-body .pl-ii{color:#fafbfc;background-color:#b31d28}.markdown-body .pl-c2{color:#fafbfc;background-color:#d73a49}.markdown-body .pl-c2::before{content:\"^M\"}.markdown-body .pl-sr .pl-cce{font-weight:700;color:#22863a}.markdown-body .pl-ml{color:#735c0f}.markdown-body .pl-mh,.markdown-body .pl-mh .pl-en,.markdown-body .pl-ms{font-weight:700;color:#005cc5}.markdown-body .pl-mi{font-style:italic;color:#24292e}.markdown-body .pl-mb{font-weight:700;color:#24292e}.markdown-body .pl-md{color:#b31d28;background-color:#ffeef0}.markdown-body .pl-mi1{color:#22863a;background-color:#f0fff4}.markdown-body .pl-mc{color:#e36209;background-color:#ffebda}.markdown-body .pl-mi2{color:#f6f8fa;background-color:#005cc5}.markdown-body .pl-mdr{font-weight:700;color:#6f42c1}.markdown-body .pl-ba{color:#586069}.markdown-body .pl-sg{color:#959da5}.markdown-body .pl-corl{text-decoration:underline;color:#032f62}.markdown-body .octicon{display:inline-block;fill:currentColor;vertical-align:text-bottom}.markdown-body hr::after,.markdown-body hr::before,.markdown-body::after,.markdown-body::before{display:table;content:\"\"}.markdown-body a{background-color:transparent;color:#0366d6;text-decoration:none}.markdown-body a:active,.markdown-body a:hover{outline-width:0}.markdown-body h1{margin:.67em 0}.markdown-body img{border-style:none}.markdown-body hr{box-sizing:content-box}.markdown-body input{font:inherit;margin:0;overflow:visible;font-family:inherit;font-size:inherit;line-height:inherit}.markdown-body dl dt,.markdown-body strong,.markdown-body table th{font-weight:600}.markdown-body code,.markdown-body pre{font-family:SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace}.markdown-body [type=checkbox]{box-sizing:border-box;padding:0}.markdown-body *{box-sizing:border-box}.markdown-body a:hover{text-decoration:underline}.markdown-body td,.markdown-body th{padding:0}.markdown-body blockquote{margin:0}.markdown-body ol ol,.markdown-body ul ol{list-style-type:lower-roman}.markdown-body ol ol ol,.markdown-body ol ul ol,.markdown-body ul ol ol,.markdown-body ul ul ol{list-style-type:lower-alpha}.markdown-body dd{margin-left:0}.markdown-body .pl-0{padding-left:0!important}.markdown-body .pl-1{padding-left:4px!important}.markdown-body .pl-2{padding-left:8px!important}.markdown-body .pl-3{padding-left:16px!important}.markdown-body .pl-4{padding-left:24px!important}.markdown-body .pl-5{padding-left:32px!important}.markdown-body .pl-6{padding-left:40px!important}.markdown-body>:first-child{margin-top:0!important}.markdown-body>:last-child{margin-bottom:0!important}.markdown-body a:not([href]){color:inherit;text-decoration:none}.markdown-body .anchor{float:left;padding-right:4px;margin-left:-20px;line-height:1}.markdown-body .anchor:focus{outline:0}.markdown-body blockquote,.markdown-body dl,.markdown-body ol,.markdown-body p,.markdown-body pre,.markdown-body table,.markdown-body ul{margin-top:0;margin-bottom:16px}.markdown-body hr{overflow:hidden;background:#e1e4e8;height:.25em;padding:0;margin:24px 0;border:0}.markdown-body blockquote{padding:0 1em;color:#6a737d;border-left:.25em solid #dfe2e5}.markdown-body h1,.markdown-body h2{padding-bottom:.3em;border-bottom:1px solid #eaecef}.markdown-body blockquote>:first-child{margin-top:0}.markdown-body blockquote>:last-child{margin-bottom:0}.markdown-body h1,.markdown-body h2,.markdown-body h3,.markdown-body h4,.markdown-body h5,.markdown-body h6{margin-top:24px;margin-bottom:16px;font-weight:600;line-height:1.25}.markdown-body h1 .octicon-link,.markdown-body h2 .octicon-link,.markdown-body h3 .octicon-link,.markdown-body h4 .octicon-link,.markdown-body h5 .octicon-link,.markdown-body h6 .octicon-link{color:#1b1f23;vertical-align:middle;visibility:hidden}.markdown-body h1:hover .anchor,.markdown-body h2:hover .anchor,.markdown-body h3:hover .anchor,.markdown-body h4:hover .anchor,.markdown-body h5:hover .anchor,.markdown-body h6:hover .anchor{text-decoration:none}.markdown-body h1:hover .anchor .octicon-link,.markdown-body h2:hover .anchor .octicon-link,.markdown-body h3:hover .anchor .octicon-link,.markdown-body h4:hover .anchor .octicon-link,.markdown-body h5:hover .anchor .octicon-link,.markdown-body h6:hover .anchor .octicon-link{visibility:visible}.markdown-body h1{font-size:2em}.markdown-body h2{font-size:1.5em}.markdown-body h3{font-size:1.25em}.markdown-body h4{font-size:1em}.markdown-body h5{font-size:.875em}.markdown-body h6{font-size:.85em;color:#6a737d}.markdown-body ol,.markdown-body ul{padding-left:2em}.markdown-body ol ol,.markdown-body ol ul,.markdown-body ul ol,.markdown-body ul ul{margin-top:0;margin-bottom:0}.markdown-body li{word-wrap:break-all}.markdown-body li>p{margin-top:16px}.markdown-body li+li{margin-top:.25em}.markdown-body dl{padding:0}.markdown-body dl dt{padding:0;margin-top:16px;font-size:1em;font-style:italic}.markdown-body dl dd{padding:0 16px;margin-bottom:16px}.markdown-body table{border-spacing:0;border-collapse:collapse;display:block;width:100%;overflow:auto}.markdown-body table td,.markdown-body table th{padding:6px 13px;border:1px solid #dfe2e5}.markdown-body table tr{background-color:#fff;border-top:1px solid #c6cbd1}.markdown-body table tr:nth-child(2n){background-color:#f6f8fa}.markdown-body img{max-width:100%;box-sizing:content-box;background-color:#fff}.markdown-body img[align=right]{padding-left:20px}.markdown-body img[align=left]{padding-right:20px}.markdown-body code{padding:.2em .4em;margin:0;font-size:85%;background-color:rgba(27,31,35,.05);border-radius:3px}.markdown-body pre{word-wrap:normal}.markdown-body pre>code{padding:0;margin:0;font-size:100%;word-break:normal;white-space:pre;background:0 0;border:0}.markdown-body .highlight{margin-bottom:16px}.markdown-body .highlight pre{margin-bottom:0;word-break:normal}.markdown-body .highlight pre,.markdown-body pre{padding:16px;overflow:auto;font-size:85%;line-height:1.45;background-color:#f6f8fa;border-radius:3px}.markdown-body pre code{display:inline;max-width:auto;padding:0;margin:0;overflow:visible;line-height:inherit;word-wrap:normal;background-color:transparent;border:0}.markdown-body .full-commit .btn-outline:not(:disabled):hover{color:#005cc5;border-color:#005cc5}.markdown-body kbd{display:inline-block;padding:3px 5px;font:11px SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace;line-height:10px;color:#444d56;vertical-align:middle;background-color:#fafbfc;border:1px solid #d1d5da;border-bottom-color:#c6cbd1;border-radius:3px;box-shadow:inset 0 -1px 0 #c6cbd1}.markdown-body :checked+.radio-label{position:relative;z-index:1;border-color:#0366d6}.markdown-body .task-list-item{list-style-type:none}.markdown-body .task-list-item+.task-list-item{margin-top:3px}.markdown-body .task-list-item input{margin:0 .2em .25em -1.6em;vertical-align:middle}.markdown-body hr{border-bottom-color:#eee}";
    string htmlText = "<!DOCTYPE html>\n<html><head></head>\n<body><style>.markdown-body {box-sizing: border-box;max-width: 980px;margin: 0 auto;padding: 45px;} " + style + " </style><link rel=\"stylesheet\" href=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css\"><article class=\"markdown-body\">\n";
    for (auto& element : elements) {
        htmlText += element.str();
    }
    htmlText += "</article><script src=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js\"></script><script>hljs.initHighlightingOnLoad();</script></body>\n</html>";
    QString qstr = QString::fromStdString(htmlText);
    ui->previewWindow->setHtml(qstr);
}

void NewWebEngineNoteWindow::saveFile() {
    if (!fromOpen) {
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::homePath(), tr("Markdown (*.md)"));
    }
    try {
        ofstream out(fileName.toUtf8().constData());
        string output = ui->txtInput->toPlainText().toUtf8().constData();
        out << output;
        out.close();
        if (!fileName.isEmpty()) {
            fileSaved = true;
            fromOpen = true;
            setWindowTitle(fileName);
            Database db;
            if (!db.checkIfExists(fileName)) {
                if (db.checkRowCountEq(10)) {
                    db.deleteOldest();
                }
                db.insertNote(fileName);
            }
        }
    } catch (exception ex) {
        QMessageBox msgBox;
        msgBox.setText("There was an error! please try again.");
        msgBox.exec();
    }
}

void NewWebEngineNoteWindow::saveFile(string path) {
    ofstream out(path);
    string output = ui->txtInput->toPlainText().toUtf8().constData();
    out << output;
    out.close();
}

void NewWebEngineNoteWindow::saveHtml() {
    string style = ".markdown-body hr::after,.markdown-body::after{clear:both}.markdown-body{-ms-text-size-adjust:100%;-webkit-text-size-adjust:100%;color:#24292e;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Helvetica,Arial,sans-serif,\"Apple Color Emoji\",\"Segoe UI Emoji\",\"Segoe UI Symbol\";font-size:16px;line-height:1.5;word-wrap:break-word}.markdown-body .pl-c{color:#6a737d}.markdown-body .pl-c1,.markdown-body .pl-s .pl-v{color:#005cc5}.markdown-body .pl-e,.markdown-body .pl-en{color:#6f42c1}.markdown-body .pl-s .pl-s1,.markdown-body .pl-smi{color:#24292e}.markdown-body .pl-ent{color:#22863a}.markdown-body .pl-k{color:#d73a49}.markdown-body .pl-pds,.markdown-body .pl-s,.markdown-body .pl-s .pl-pse .pl-s1,.markdown-body .pl-sr,.markdown-body .pl-sr .pl-cce,.markdown-body .pl-sr .pl-sra,.markdown-body .pl-sr .pl-sre{color:#032f62}.markdown-body .pl-smw,.markdown-body .pl-v{color:#e36209}.markdown-body .pl-bu{color:#b31d28}.markdown-body .pl-ii{color:#fafbfc;background-color:#b31d28}.markdown-body .pl-c2{color:#fafbfc;background-color:#d73a49}.markdown-body .pl-c2::before{content:\"^M\"}.markdown-body .pl-sr .pl-cce{font-weight:700;color:#22863a}.markdown-body .pl-ml{color:#735c0f}.markdown-body .pl-mh,.markdown-body .pl-mh .pl-en,.markdown-body .pl-ms{font-weight:700;color:#005cc5}.markdown-body .pl-mi{font-style:italic;color:#24292e}.markdown-body .pl-mb{font-weight:700;color:#24292e}.markdown-body .pl-md{color:#b31d28;background-color:#ffeef0}.markdown-body .pl-mi1{color:#22863a;background-color:#f0fff4}.markdown-body .pl-mc{color:#e36209;background-color:#ffebda}.markdown-body .pl-mi2{color:#f6f8fa;background-color:#005cc5}.markdown-body .pl-mdr{font-weight:700;color:#6f42c1}.markdown-body .pl-ba{color:#586069}.markdown-body .pl-sg{color:#959da5}.markdown-body .pl-corl{text-decoration:underline;color:#032f62}.markdown-body .octicon{display:inline-block;fill:currentColor;vertical-align:text-bottom}.markdown-body hr::after,.markdown-body hr::before,.markdown-body::after,.markdown-body::before{display:table;content:\"\"}.markdown-body a{background-color:transparent;color:#0366d6;text-decoration:none}.markdown-body a:active,.markdown-body a:hover{outline-width:0}.markdown-body h1{margin:.67em 0}.markdown-body img{border-style:none}.markdown-body hr{box-sizing:content-box}.markdown-body input{font:inherit;margin:0;overflow:visible;font-family:inherit;font-size:inherit;line-height:inherit}.markdown-body dl dt,.markdown-body strong,.markdown-body table th{font-weight:600}.markdown-body code,.markdown-body pre{font-family:SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace}.markdown-body [type=checkbox]{box-sizing:border-box;padding:0}.markdown-body *{box-sizing:border-box}.markdown-body a:hover{text-decoration:underline}.markdown-body td,.markdown-body th{padding:0}.markdown-body blockquote{margin:0}.markdown-body ol ol,.markdown-body ul ol{list-style-type:lower-roman}.markdown-body ol ol ol,.markdown-body ol ul ol,.markdown-body ul ol ol,.markdown-body ul ul ol{list-style-type:lower-alpha}.markdown-body dd{margin-left:0}.markdown-body .pl-0{padding-left:0!important}.markdown-body .pl-1{padding-left:4px!important}.markdown-body .pl-2{padding-left:8px!important}.markdown-body .pl-3{padding-left:16px!important}.markdown-body .pl-4{padding-left:24px!important}.markdown-body .pl-5{padding-left:32px!important}.markdown-body .pl-6{padding-left:40px!important}.markdown-body>:first-child{margin-top:0!important}.markdown-body>:last-child{margin-bottom:0!important}.markdown-body a:not([href]){color:inherit;text-decoration:none}.markdown-body .anchor{float:left;padding-right:4px;margin-left:-20px;line-height:1}.markdown-body .anchor:focus{outline:0}.markdown-body blockquote,.markdown-body dl,.markdown-body ol,.markdown-body p,.markdown-body pre,.markdown-body table,.markdown-body ul{margin-top:0;margin-bottom:16px}.markdown-body hr{overflow:hidden;background:#e1e4e8;height:.25em;padding:0;margin:24px 0;border:0}.markdown-body blockquote{padding:0 1em;color:#6a737d;border-left:.25em solid #dfe2e5}.markdown-body h1,.markdown-body h2{padding-bottom:.3em;border-bottom:1px solid #eaecef}.markdown-body blockquote>:first-child{margin-top:0}.markdown-body blockquote>:last-child{margin-bottom:0}.markdown-body h1,.markdown-body h2,.markdown-body h3,.markdown-body h4,.markdown-body h5,.markdown-body h6{margin-top:24px;margin-bottom:16px;font-weight:600;line-height:1.25}.markdown-body h1 .octicon-link,.markdown-body h2 .octicon-link,.markdown-body h3 .octicon-link,.markdown-body h4 .octicon-link,.markdown-body h5 .octicon-link,.markdown-body h6 .octicon-link{color:#1b1f23;vertical-align:middle;visibility:hidden}.markdown-body h1:hover .anchor,.markdown-body h2:hover .anchor,.markdown-body h3:hover .anchor,.markdown-body h4:hover .anchor,.markdown-body h5:hover .anchor,.markdown-body h6:hover .anchor{text-decoration:none}.markdown-body h1:hover .anchor .octicon-link,.markdown-body h2:hover .anchor .octicon-link,.markdown-body h3:hover .anchor .octicon-link,.markdown-body h4:hover .anchor .octicon-link,.markdown-body h5:hover .anchor .octicon-link,.markdown-body h6:hover .anchor .octicon-link{visibility:visible}.markdown-body h1{font-size:2em}.markdown-body h2{font-size:1.5em}.markdown-body h3{font-size:1.25em}.markdown-body h4{font-size:1em}.markdown-body h5{font-size:.875em}.markdown-body h6{font-size:.85em;color:#6a737d}.markdown-body ol,.markdown-body ul{padding-left:2em}.markdown-body ol ol,.markdown-body ol ul,.markdown-body ul ol,.markdown-body ul ul{margin-top:0;margin-bottom:0}.markdown-body li{word-wrap:break-all}.markdown-body li>p{margin-top:16px}.markdown-body li+li{margin-top:.25em}.markdown-body dl{padding:0}.markdown-body dl dt{padding:0;margin-top:16px;font-size:1em;font-style:italic}.markdown-body dl dd{padding:0 16px;margin-bottom:16px}.markdown-body table{border-spacing:0;border-collapse:collapse;display:block;width:100%;overflow:auto}.markdown-body table td,.markdown-body table th{padding:6px 13px;border:1px solid #dfe2e5}.markdown-body table tr{background-color:#fff;border-top:1px solid #c6cbd1}.markdown-body table tr:nth-child(2n){background-color:#f6f8fa}.markdown-body img{max-width:100%;box-sizing:content-box;background-color:#fff}.markdown-body img[align=right]{padding-left:20px}.markdown-body img[align=left]{padding-right:20px}.markdown-body code{padding:.2em .4em;margin:0;font-size:85%;background-color:rgba(27,31,35,.05);border-radius:3px}.markdown-body pre{word-wrap:normal}.markdown-body pre>code{padding:0;margin:0;font-size:100%;word-break:normal;white-space:pre;background:0 0;border:0}.markdown-body .highlight{margin-bottom:16px}.markdown-body .highlight pre{margin-bottom:0;word-break:normal}.markdown-body .highlight pre,.markdown-body pre{padding:16px;overflow:auto;font-size:85%;line-height:1.45;background-color:#f6f8fa;border-radius:3px}.markdown-body pre code{display:inline;max-width:auto;padding:0;margin:0;overflow:visible;line-height:inherit;word-wrap:normal;background-color:transparent;border:0}.markdown-body .full-commit .btn-outline:not(:disabled):hover{color:#005cc5;border-color:#005cc5}.markdown-body kbd{display:inline-block;padding:3px 5px;font:11px SFMono-Regular,Consolas,\"Liberation Mono\",Menlo,Courier,monospace;line-height:10px;color:#444d56;vertical-align:middle;background-color:#fafbfc;border:1px solid #d1d5da;border-bottom-color:#c6cbd1;border-radius:3px;box-shadow:inset 0 -1px 0 #c6cbd1}.markdown-body :checked+.radio-label{position:relative;z-index:1;border-color:#0366d6}.markdown-body .task-list-item{list-style-type:none}.markdown-body .task-list-item+.task-list-item{margin-top:3px}.markdown-body .task-list-item input{margin:0 .2em .25em -1.6em;vertical-align:middle}.markdown-body hr{border-bottom-color:#eee}";
    QString exportName = QFileDialog::getSaveFileName(this, tr("Export HTML"), QDir::homePath(), tr("HTML (*.html *.htm)"));
    if (exportName.size() > 0) {
        string content = ui->txtInput->toPlainText().toUtf8().constData();
        StringParser parser;
        parser.parse(content);
        vector<HtmlElement> elements = parser.getElements();
        string htmlText = "<!DOCTYPE html>\n<html><head></head>\n<body><style>.markdown-body {box-sizing: border-box;max-width: 980px;margin: 0 auto;padding: 45px;} " + style + " </style><link rel=\"stylesheet\" href=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/styles/default.min.css\"><article class=\"markdown-body\">\n";
        for (auto&& element : elements) {
            htmlText += element.str();
        }
        htmlText += "</article><script src=\"http://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.12.0/highlight.min.js\"></script><script>hljs.initHighlightingOnLoad();</script></body></html>";
        try {
            ofstream out(exportName.toUtf8().constData());
            string output = ui->txtInput->toPlainText().toUtf8().constData();
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

NewWebEngineNoteWindow::~NewWebEngineNoteWindow() {
    delete ui;
}