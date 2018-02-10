#ifndef GEEETECHPRUSAI3X_H
#define GEEETECHPRUSAI3X_H

#include "Printer.h"

class GeeetechPrusaI3X : public Printer
{
    public:
        GeeetechPrusaI3X(int port, ostream* out) : Printer::Printer(port, out) { Printer::baud=250000; GeeetechPrusaI3X::printing=false;};
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

#endif // GEEETECHPRUSAI3X_H
