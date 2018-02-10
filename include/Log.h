#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <syslog.h>
#include <cstring>

#include "Colors.h"

using namespace std;

enum LogPriority {
    kLogEmerg   = LOG_EMERG,   // system is unusable
    kLogAlert   = LOG_ALERT,   // action must be taken immediately
    kLogCrit    = LOG_CRIT,    // critical conditions
    kLogErr     = LOG_ERR,     // error conditions
    kLogWarning = LOG_WARNING, // warning conditions
    kLogNotice  = LOG_NOTICE,  // normal, but significant, condition
    kLogInfo    = LOG_INFO,    // informational message
    kLogDebug   = LOG_DEBUG    // debug-level message
};

ostream& operator<< (ostream& os, const LogPriority& log_priority);

class Log : public basic_streambuf<char, std::char_traits<char> >
{
    public:
        explicit Log(string ident, int facility);
    protected:
        int sync();
        int overflow(int c);
    private:
        friend ostream& operator<< (ostream& os, const LogPriority& log_priority);
        string buffer_;
        int facility_;
        int priority_;
        char ident_[50];
};

#endif // LOG_H
