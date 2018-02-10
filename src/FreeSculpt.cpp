#include "../include/FreeSculpt.h"

FreeSculpt::FreeSculpt(int port, int version, ostream* out) : Printer::Printer(port, out)
{
    Printer::baud=115200;
}

void FreeSculpt::wait()
{
    wait_for("ok");
    wait_for("\n");
}

int FreeSculpt::connect()
{
    send("N0 M110 *3\n"); wait();
    send("N1 G90 *49\n"); wait();
    send("N2 G92 X0 Y0 Z0 E0 *46\n"); wait();
    send("N3 M115 *5\n"); wait();

    return 0;
}

int FreeSculpt::test()
{
    return 2;
}

int FreeSculpt::print(string filename)
{
    return 2;
}

int FreeSculpt::information()
{
    cout << "FreeSculpt::information() not implemented yet!" << endl;
    return 2;
}
