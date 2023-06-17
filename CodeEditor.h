#pragma once
#include <optional>
#include <string>

#include <QMainWindow>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

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
    TextEdit(QWidget* parent = nullptr);
    bool load(const QString& f);

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
    bool maybeSave();
    void setCurrentFileName(const QString& fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat& format);
    void colorChanged(const QColor& c);

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
    QTextEdit* textEdit;
    Highlighter* highlighter;
};
