#pragma once
#include <optional>
#include <string>
#include <vector>

#include <QMainWindow>
#include <QSize>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QFontMetrics>

class NexusEnv;
class QTextEditHighlighter;
class LineNumberArea;

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
    QTextCharFormat controlFormat;
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
    void setup_file_actions();
    void setup_edit_actions();
    void setup_tab();
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

    QString         fileName;
    QTextDocument*  document;
    QTextEditHighlighter*     textEdit;
    QWidget*        lineNumberArea;
    Highlighter*    highlighter;
};


class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(QTextEdit* editor);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent* event);

private:
    QTextEdit* codeEditor;
};

class QTextEditHighlighter : public QTextEdit
{
    Q_OBJECT

public:

    explicit QTextEditHighlighter(QWidget* parent = 0);

    int getFirstVisibleBlockId();
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();

signals:


public slots:

    void resizeEvent(QResizeEvent* e);

private slots:

    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(QRectF /*rect_f*/);
    void updateLineNumberArea(int /*slider_pos*/);
    void updateLineNumberArea();

private:

    QWidget* lineNumberArea;

};