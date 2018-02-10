#ifndef FREESCULPT_H
#define FREESCULPT_H

#include "Printer.h"

class FreeSculpt : public Printer
{
    public:
        FreeSculpt(int port, int version, ostream* out);
        virtual int connect();
        virtual int print(string filename);
        virtual int test();
        virtual int information();
    protected:
        int version;
    private:
        void wait();
};

#endif // FREESCULPT_H
