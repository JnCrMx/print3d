#ifndef PRINTER_H
#define PRINTER_H

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <algorithm>
#include <stropts.h>
#include <asm/termios.h>
#include <sys/types.h>
#include <cstring>

#include "Colors.h"

namespace abc //Why?
{
#include <termios.h>
}

using namespace std;

class Printer
{
    public:
        Printer(int port, ostream* out);
        int baud;
        virtual int setup();
        virtual int connect();
        virtual int print(string filename);
        virtual int test();
        virtual int information();
        virtual int custom_gcode(string gcode);
        virtual int stop();
        void output(ostream* out, bool details);
        ostream* output();
    protected:
        int port;
        ostream* out;
        bool details=true;
        virtual ssize_t send(const char* buf);
        virtual string wait_for(const char* towaitfor);
    private:
};

#endif // PRINTER_H
