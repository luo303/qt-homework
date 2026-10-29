#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "appinfo.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QMessageBox>
#include <QSaveFile>
#include <QStatusBar>
#include <QTextDocument>

namespace {
// 超过这个大小的文件当文本打开会让界面卡很久，直接拦掉
const qint64 kMaxFileSize = 10 * 1024 * 1024;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_findDialog(nullptr)
    , m_replaceDialog(nullptr)
{
    ui->setupUi(this);
    createStatusBarLabels();

    // ---------- 文件 ----------
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // ---------- 编辑 ----------
    connect(ui->actionUndo, &QAction::triggered, ui->editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->editor, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->editor, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->editor, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->editor, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->editor, &QPlainTextEdit::selectAll);
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // ---------- 格式 ----------
    connect(ui->actionWordWrap, &QAction::toggled, this, &MainWindow::toggleWordWrap);
    connect(ui->actionLineNumber, &QAction::toggled, ui->editor, &CodeEditor::setLineNumberVisible);
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::changeFontColor);
    connect(ui->actionFontBackground, &QAction::triggered, this, &MainWindow::changeFontBackground);
    connect(ui->actionEditorBackground, &QAction::triggered, this, &MainWindow::changeEditorBackground);

    // ---------- 查看 ----------
    connect(ui->actionToolBar, &QAction::toggled, ui->mainToolBar, &QToolBar::setVisible);
    connect(ui->actionStatusBar, &QAction::toggled, ui->statusbar, &QStatusBar::setVisible);

    // ---------- 帮助 ----------
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);

    // ---------- 动作使能状态跟随编辑状态 ----------
    connect(ui->editor, &QPlainTextEdit::undoAvailable, ui->actionUndo, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::redoAvailable, ui->actionRedo, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::copyAvailable, this, &MainWindow::updateCopyActions);
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updatePasteAction);

    // ---------- 状态栏与标题 ----------
    connect(ui->editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);

    // 刚启动时没有任何内容，撤销/剪切/复制都应该是灰的
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    updatePasteAction();

    setCurrentFile(QString());
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createStatusBarLabels()
{
    m_lengthLabel = new QLabel(this);
    m_lineCountLabel = new QLabel(this);
    m_positionLabel = new QLabel(this);
    m_authorLabel = new QLabel(AppInfo::developer(), this);

    // 按照老师给的参考界面，统计信息放在状态栏右侧，作者名放在最右边
    statusBar()->addPermanentWidget(m_lengthLabel);
    statusBar()->addPermanentWidget(m_lineCountLabel);
    statusBar()->addPermanentWidget(m_positionLabel);
    statusBar()->addPermanentWidget(m_authorLabel);

    // 右下角那个拖拽三角会压住作者名，这里关掉
    statusBar()->setSizeGripEnabled(false);
}

QString MainWindow::displayFileName() const
{
    if (m_filePath.isEmpty())
        return tr("新建文本文件");

    return QFileInfo(m_filePath).fileName();
}

void MainWindow::updateWindowTitle()
{
    const bool modified = ui->editor->document()->isModified();

    // 参考截图里的标题格式：有未保存修改时文件名前面带一个星号
    setWindowTitle(QString("%1%2 - %3")
                       .arg(modified ? QStringLiteral("* ") : QString(),
                            displayFileName(),
                            AppInfo::appName()));
}

void MainWindow::setCurrentFile(const QString &filePath)
{
    m_filePath = filePath;
    ui->editor->document()->setModified(false);
    updateWindowTitle();
}

// ------------------------------------------------------------------
// 文件操作
// ------------------------------------------------------------------

bool MainWindow::maybeSave()
{
    if (!ui->editor->document()->isModified())
        return true;

    const QMessageBox::StandardButton result = QMessageBox::warning(
        this, AppInfo::appName(),
        tr("「%1」的修改尚未保存，是否保存？").arg(displayFileName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);

    if (result == QMessageBox::Save)
        return saveFile();      // 保存失败或者另存为被取消，就不要再往下走
    if (result == QMessageBox::Discard)
        return true;

    return false;
}

void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    ui->editor->clear();
    setCurrentFile(QString());
    updateStatusBar();
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("打开文件"), QString(),
        tr("文本文件 (*.txt *.cpp *.h *.md);;所有文件 (*.*)"));
    if (filePath.isEmpty())
        return;

    if (!loadFile(filePath))
        return;

    setCurrentFile(filePath);
    statusBar()->showMessage(tr("已打开 %1").arg(QDir::toNativeSeparators(filePath)), 3000);
}

bool MainWindow::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("无法打开文件「%1」。\n%2")
                                 .arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return false;
    }

    if (file.size() > kMaxFileSize) {
        file.close();
        QMessageBox::warning(this, tr("打开文件"),
                             tr("「%1」超过 10 MB，本程序暂不支持作为文本打开。")
                                 .arg(QDir::toNativeSeparators(filePath)));
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    // 二进制文件中间一般有 \0，按文本打开只会显示一堆乱码，这里先判断一下
    if (data.contains('\0')) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("「%1」看起来不是文本文件。")
                                 .arg(QDir::toNativeSeparators(filePath)));
        return false;
    }

    // 先按 UTF-8 读，读出来是乱码（有替换字符）再退回本地编码
    QString text = QString::fromUtf8(data);
    if (text.contains(QChar::ReplacementCharacter))
        text = QString::fromLocal8Bit(data);

    ui->editor->setPlainText(text);
    ui->editor->document()->setModified(false);

    return true;
}

bool MainWindow::saveFile()
{
    if (m_filePath.isEmpty())
        return saveFileAs();

    return writeFile(m_filePath);
}

bool MainWindow::saveFileAs()
{
    const QString startPath = m_filePath.isEmpty()
                                  ? QDir::homePath() + QStringLiteral("/新建文本文件.txt")
                                  : m_filePath;

    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("另存为"), startPath,
        tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (filePath.isEmpty())
        return false;

    return writeFile(filePath);
}

bool MainWindow::writeFile(const QString &filePath)
{
    // QSaveFile 先写临时文件再整体替换，写到一半出错也不会把原文件弄坏
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法写入文件「%1」。\n%2")
                                 .arg(QDir::toNativeSeparators(filePath), file.errorString()));
        return false;
    }

    const QByteArray data = ui->editor->toPlainText().toUtf8();
    if (file.write(data) != data.size()) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("写入文件时出错：\n%1").arg(file.errorString()));
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("保存文件失败：\n%1").arg(file.errorString()));
        return false;
    }

    setCurrentFile(filePath);
    statusBar()->showMessage(tr("已保存 %1").arg(QDir::toNativeSeparators(filePath)), 3000);
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ------------------------------------------------------------------
// 编辑操作
// ------------------------------------------------------------------

void MainWindow::updateCopyActions(bool hasSelection)
{
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
}

void MainWindow::updatePasteAction()
{
    // 剪贴板里没有文本时“粘贴”不该能点
    ui->actionPaste->setEnabled(ui->editor->canPaste());
}

void MainWindow::showFindDialog()
{
    if (m_findDialog == nullptr)
        m_findDialog = new FindDialog(ui->editor, this);

    // 选中了文字再按 Ctrl+F 时，直接带过去当查找目标，省得重新输入
    const QString selected = ui->editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        m_findDialog->setSearchText(selected);

    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (m_replaceDialog == nullptr)
        m_replaceDialog = new ReplaceDialog(ui->editor, this);

    const QString selected = ui->editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        m_replaceDialog->setSearchText(selected);

    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

void MainWindow::showAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

// ------------------------------------------------------------------
// 格式设置
// ------------------------------------------------------------------

void MainWindow::applyCharFormat(const QTextCharFormat &format)
{
    QTextCursor cursor = ui->editor->textCursor();
    cursor.mergeCharFormat(format);

    // 没有选中内容时，把带格式的光标交还给编辑区，
    // 接下来输入的文字才会用上新的颜色 / 背景色
    if (!cursor.hasSelection())
        ui->editor->setTextCursor(cursor);
}

void MainWindow::changeFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->editor->font(), this, tr("字体"));
    if (!ok)
        return;

    // QPlainTextEdit 会把字体同时设到文档默认字体上，已有文字也会跟着变
    ui->editor->setFont(font);
}

void MainWindow::changeFontColor()
{
    // QPlainTextEdit 没有 textColor()，当前颜色要从光标的字符格式里取
    const QColor current = ui->editor->textCursor().charFormat().foreground().color();
    const QColor color = QColorDialog::getColor(current.isValid() ? current : Qt::black,
                                                this, tr("字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setForeground(color);
    applyCharFormat(format);
}

void MainWindow::changeFontBackground()
{
    const QColor color = QColorDialog::getColor(Qt::white, this, tr("字体背景色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setBackground(color);
    applyCharFormat(format);
}

void MainWindow::changeEditorBackground()
{
    const QColor color = QColorDialog::getColor(ui->editor->palette().color(QPalette::Base),
                                                this, tr("编辑器背景色"));
    if (!color.isValid())
        return;

    QPalette palette = ui->editor->palette();
    palette.setColor(QPalette::Base, color);
    ui->editor->setPalette(palette);
}

void MainWindow::toggleWordWrap(bool on)
{
    ui->editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                                   : QPlainTextEdit::NoWrap);
}

// ------------------------------------------------------------------
// 状态栏
// ------------------------------------------------------------------

void MainWindow::updateStatusBar()
{
    const QTextCursor cursor = ui->editor->textCursor();

    m_lengthLabel->setText(tr("length: %1").arg(ui->editor->toPlainText().length()));
    m_lineCountLabel->setText(tr("lines: %1").arg(ui->editor->blockCount()));
    m_positionLabel->setText(tr("Ln: %1   Col: %2")
                                 .arg(cursor.blockNumber() + 1)
                                 .arg(cursor.positionInBlock() + 1));
}
