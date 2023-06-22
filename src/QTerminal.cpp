
#include <QRegularExpression.h>

#include "QTerminal.h"


#if defined (Q_OS_WIN32)
# include "QTerminalImpl.h"
#else
# include "unix/QUnixTerminalImpl.h"
#endif

QTerminal*
QTerminal::create(QWidget* xparent)
{
#if defined (Q_OS_WIN32)
    return new QWinTerminalImpl(xparent);
#else
    return new QUnixTerminalImpl(xparent);
#endif
}

QList<QColor>
QTerminal::default_colors(void)
{
    static QList<QColor> colors;

    if (colors.isEmpty())
    {
        colors << QColor(0, 0, 0)
            << QColor(255, 255, 255)
            << QColor(192, 192, 192)
            << QColor(128, 128, 128);
    }

    return colors;
}

QStringList
QTerminal::color_names(void)
{
    static QStringList names;

    if (names.isEmpty())
    {
        names << QObject::tr("foreground")
            << QObject::tr("background")
            << QObject::tr("selection")
            << QObject::tr("cursor");
    }

    return names;
}

// slot for disabling the interrupt action when terminal loses focus
void
QTerminal::set_global_shortcuts(bool focus_out)
{
    if (focus_out)
    {
        _interrupt_action->setShortcut(QKeySequence());
        _nop_action->setShortcut(QKeySequence());
    }
    else
    {
        _interrupt_action->setShortcut(
            QKeySequence(Qt::ControlModifier | Qt::Key_C));
        _nop_action->setShortcut(
            QKeySequence(Qt::ControlModifier | Qt::Key_D));
    }
}

// slot for the terminal's context menu
void
QTerminal::handleCustomContextMenuRequested(const QPoint& at)
{
    QClipboard* cb = QApplication::clipboard();
    QString selected_text = selectedText();
    bool has_selected_text = !selected_text.isEmpty();

    _edit_action->setVisible(false);

#if defined (Q_OS_WIN32)
    // include this when in windows because there is no filter for
    // detecting links and error messages yet
    if (has_selected_text)
    {
        QRegularExpression file("(?:[ \\t]+)(\\S+) at line (\\d+) column (?:\\d+)");
        /*
        int pos = file.indexIn(selected_text);

        if (pos > -1)
        {
            QString file_name = file.cap(1);
            QString line = file.cap(2);

            _edit_action->setVisible(true);
            _edit_action->setText(tr("Edit %1 at line %2")
                .arg(file_name).arg(line));

            QStringList data;
            data << file_name << line;
            _edit_action->setData(data);
        }
        */
    }
#endif

    _paste_action->setEnabled(cb->text().length() > 0);
    _copy_action->setEnabled(has_selected_text);
    _run_selection_action->setEnabled(has_selected_text);

    // Get the actions of any hotspots the filters may have found
    QList<QAction*> actions = get_hotspot_actions(at);
    if (actions.length())
        _contextMenu->addSeparator();
    for (int i = 0; i < actions.length(); i++)
        _contextMenu->addAction(actions.at(i));

    // Finally, show the context menu
    _contextMenu->exec(mapToGlobal(at));

    // Cleaning up, remove actions of the hotspot
    for (int i = 0; i < actions.length(); i++)
        _contextMenu->removeAction(actions.at(i));
}

// slot for running the selected code
void
QTerminal::run_selection()
{
    QStringList commands = selectedText().split(QRegularExpression("[\r\n]"),
        Qt::SkipEmptyParts);
    for (int i = 0; i < commands.size(); i++)
        emit execute_command_in_terminal_signal(commands.at(i));

}

// slot for edit files in error messages
void
QTerminal::edit_file()
{
    QString file = _edit_action->data().toStringList().at(0);
    int line = _edit_action->data().toStringList().at(1).toInt();

    emit edit_mfile_request(file, line);
}

void
QTerminal::notice_settings(const QSettings* settings)
{
    // QSettings pointer is checked before emitting.

    // Set terminal font:
    QFont term_font = QFont();
    term_font.setStyleHint(QFont::TypeWriter);
    //QString default_font = settings->value(global_mono_font.key, global_mono_font.def).toString();
    //term_font.setFamily
    //(settings->value(cs_font.key, default_font).toString());
    term_font.setPointSize(settings->value("terminal/fontSize", 10).toInt());
    setTerminalFont(term_font);

    QFontMetrics metrics(term_font);
    setMinimumSize(metrics.maxWidth() * 16, metrics.height() * 3);

    QString cursorType
        = settings->value("terminal/cursorType", "ibeam").toString();

    bool cursorBlinking;
    if (settings->contains("cursor_blinking"))
        cursorBlinking = settings->value("cursor_blinking", true).toBool();
    else
        cursorBlinking = settings->value("terminal/cursorBlinking", true).toBool();

    if (cursorType == "ibeam")
        setCursorType(QTerminal::IBeamCursor, cursorBlinking);
    else if (cursorType == "block")
        setCursorType(QTerminal::BlockCursor, cursorBlinking);
    else if (cursorType == "underline")
        setCursorType(QTerminal::UnderlineCursor, cursorBlinking);

    bool cursorUseForegroundColor
        = settings->value("terminal/cursorUseForegroundColor", true).toBool();

    QList<QColor> colors = default_colors();

    setForegroundColor
    (settings->value("terminal/color_f",
        QVariant(colors.at(0))).value<QColor>());

    setBackgroundColor
    (settings->value("terminal/color_b",
        QVariant(colors.at(1))).value<QColor>());

    setSelectionColor
    (settings->value("terminal/color_s",
        QVariant(colors.at(2))).value<QColor>());

    setCursorColor
    (cursorUseForegroundColor,
        settings->value("terminal/color_c",
            QVariant(colors.at(3))).value<QColor>());
    setScrollBufferSize(settings->value("terminal/history_buffer", 1000).toInt());

    // check whether Copy shortcut is Ctrl-C
    QKeySequence sc;
    sc = QKeySequence(settings->value("shortcuts/main_edit:copy").toString());

    // if sc is empty, shortcuts are not yet in the settings (take the default)
    if (sc.isEmpty())         // QKeySequence::Copy as second argument in
        sc = QKeySequence::Copy; // settings->value () does not work!

    //  dis- or enable extra interrupt action
    bool extra_ir_action = (sc != QKeySequence(Qt::ControlModifier | Qt::Key_C));
    _interrupt_action->setEnabled(extra_ir_action);
    has_extra_interrupt(extra_ir_action);

    // check whether shortcut Ctrl-D is in use by the main-window
    bool ctrld = settings->value("shortcuts/main_ctrld", false).toBool();
    _nop_action->setEnabled(!ctrld);
}