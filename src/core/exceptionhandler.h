#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include <QString>

#if defined(Q_OS_LINUX)
#include <client/linux/handler/exception_handler.h>
#elif defined(Q_OS_WIN)
#include <client/windows/handler/exception_handler.h>
#endif

class ExceptionHandler: public google_breakpad::ExceptionHandler
{
public:
    ExceptionHandler(const QString &directory);
};

#endif // EXCEPTIONHANDLER_H
