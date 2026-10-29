#ifndef APPINFO_H
#define APPINFO_H

#include <QString>

// 关于对话框和状态栏里要显示的个人信息集中放在这里，换人交作业只改这个文件即可
namespace AppInfo {

inline QString appName()        { return QStringLiteral("简易文本编辑器"); }
inline QString developer()      { return QStringLiteral("姓　名"); }
inline QString studentId()      { return QStringLiteral("学　号"); }
inline QString email()          { return QStringLiteral("email@example.com"); }
inline QString version()        { return QStringLiteral("V1.0 build 202610"); }

}

#endif // APPINFO_H
