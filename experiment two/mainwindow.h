#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCharFormat>

class FindDialog;
class ReplaceDialog;
class QLabel;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    // 编辑
    void showFindDialog();
    void showReplaceDialog();
    void updateCopyActions(bool hasSelection);
    void updatePasteAction();

    // 格式
    void changeFont();
    void changeFontColor();
    void changeFontBackground();
    void changeEditorBackground();
    void toggleWordWrap(bool on);

    // 其它
    void showAbout();
    void updateStatusBar();
    void updateWindowTitle();

private:
    bool maybeSave();
    bool loadFile(const QString &filePath);
    bool writeFile(const QString &filePath);
    void setCurrentFile(const QString &filePath);
    void createStatusBarLabels();
    void applyCharFormat(const QTextCharFormat &format);
    QString displayFileName() const;

    Ui::MainWindow *ui;

    QString m_filePath;
    FindDialog *m_findDialog;
    ReplaceDialog *m_replaceDialog;

    QLabel *m_lengthLabel;
    QLabel *m_lineCountLabel;
    QLabel *m_positionLabel;
    QLabel *m_authorLabel;
};

#endif // MAINWINDOW_H
