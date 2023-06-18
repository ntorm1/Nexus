#pragma once
#include <optional>
#include <string>
#include <vector>

#include <QMainWindow>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class NexusEnv;

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QTextDocument;
QT_END_NAMESPACE

//! [0]
class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument* parent = 0);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

class TextEdit : public QMainWindow
{
    Q_OBJECT
public:
    TextEdit(
        NexusEnv const * nexus_env,
        QWidget* parent = nullptr
    );
    bool load(const QString& f);

    bool maybeSave();

    QString const& get_file_name() const { return this->fileName; }
    QTextDocument* get_document() const { return this->document; }

public slots:
    void fileNew();

protected:
    void closeEvent(QCloseEvent* e) override;

private slots:
    void fileOpen(std::optional<std::string> path = std::nullopt);
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textFamily(const QString& f);
    void textSize(const QString& p);
    void textStyle(int styleIndex);
    void textColor();
    void setChecked(bool checked);

    void currentCharFormatChanged(const QTextCharFormat& format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();

private:
    void setupFileActions();
    void setupEditActions();
    void setCurrentFileName(const QString& fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat& format);
    void colorChanged(const QColor& c);

    NexusEnv const * nexus_env;
    QAction* actionSave;
    QAction* actionToggleCheckState;
    QAction* actionUndo;
    QAction* actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction* actionCut;
    QAction* actionCopy;
    QAction* actionPaste;
#endif

    QString fileName;
    QTextDocument* document;
    QTextEdit* textEdit;
    Highlighter* highlighter;
};
