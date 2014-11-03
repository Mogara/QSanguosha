#include "exceptionhandler.h"

#include <QProcess>
#include <QCoreApplication>
#include <QFile>

#if defined(Q_OS_LINUX)
static bool DumpCallback(const google_breakpad::MinidumpDescriptor &md,void *, bool succeeded)
{
    if (succeeded && QFile::exists("QSanSMTPClient.exe")){
        QProcess *process = new QProcess(qApp);
        QStringList args;
        args << QString(md.path());
        process->start("QSanSMTPClient", args);
    }
    return succeeded;
}

#elif defined(Q_OS_WIN)
static bool DumpCallback(const wchar_t *, const wchar_t *id, void *, EXCEPTION_POINTERS *, MDRawAssertionInfo *, bool succeeded)
{
    if (succeeded && QFile::exists("QSanSMTPClient.exe")){
        char ID[16000];
        memset(ID, 0, sizeof(ID));
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
        wcstombs(ID, id, wcslen(id));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        QProcess *process = new QProcess(qApp);
        QStringList args;
        args << QString(ID) + ".dmp";
        process->start("QSanSMTPClient", args);
    }
    return succeeded;
}
#endif

ExceptionHandler::ExceptionHandler(const QString &directory)
#if defined(Q_OS_LINUX)
    : google_breakpad::ExceptionHandler(google_breakpad::MinidumpDescriptor(directory.toStdString()), NULL, DumpCallback, NULL, true, -1)
#elif defined(Q_OS_WIN)
    : google_breakpad::ExceptionHandler(directory.toStdWString(), NULL, DumpCallback, NULL, google_breakpad::ExceptionHandler::HANDLER_ALL)
#endif
{
}
