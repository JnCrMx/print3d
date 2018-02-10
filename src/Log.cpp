#include "../include/Log.h"

#include <libio.h>

Log::Log(string ident, int facility)
{
    Log::facility_=facility;
    Log::priority_=LOG_INFO;

    strncpy(ident_, ident.c_str(), sizeof(ident_));
    ident_[sizeof(ident_)-1] = '\0';

    openlog(ident_, LOG_PID | LOG_CONS| LOG_NDELAY, facility_);
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int Log::sync()
{
    int len=buffer_.length();
    if (len)
    {
        size_t pos=buffer_.find('\n');
        if(pos != string::npos)
        {
            if(pos!=0)
            {
                replace(buffer_, BOLD, "");
                replace(buffer_, RED, "");
                replace(buffer_, RESET, "");
                replace(buffer_, YELLOW, "");
                replace(buffer_, GREEN, "");

                syslog(priority_, buffer_.c_str());
                buffer_.erase();
                priority_ = LOG_INFO;
            }
            else
            {
                buffer_.erase();
            }
        }
    }
    return 0;
}

int Log::overflow(int c)
{
    if (c != EOF)
    {
        buffer_ += static_cast<char>(c);
    }
    else
    {
        sync();
    }
    return c;
}

std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority)
{
    static_cast<Log *>(os.rdbuf())->priority_ = (int)log_priority;
    return os;
}
