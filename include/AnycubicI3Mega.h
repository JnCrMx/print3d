#ifndef ANYCUBICI3MEGA_H
#define ANYCUBICI3MEGA_H

#include "Printer.h"

class AnycubicI3Mega : public Printer
{
    public:
		AnycubicI3Mega(int port, ostream* out) : Printer::Printer(port, out) { Printer::baud=250000; AnycubicI3Mega::printing=false;};
        virtual int connect();
        virtual int print(string filename);
        virtual int test();
        virtual int information();
        virtual int custom_gcode(string gcode);
        virtual int stop();
    protected:
    private:
        bool printing;
        void wait();
};

#endif // ANYCUBICI3MEGA_H
