#include "../include/Printer.h"

Printer::Printer(int port, ostream* out)
{
    Printer::port=port;
    Printer::out=out;
}

void Printer::output(ostream* out, bool details)
{
	Printer::out=out;
	Printer::details=details;
}

ostream* Printer::output()
{
	return Printer::out;
}

int Printer::stop()
{
    *out << "Printer::stop() should not be called!" << endl;
    return 2;
}

int Printer::setup()
{
    speed_t baud=this->baud;

    struct termios2 tty;
    memset(&tty, 0, sizeof tty);

    if(ioctl(port, TCGETS2, &tty))
    {
        *out << "[" << RED << "fail" << RESET << "]" << endl;
        *out << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"setting up port failed (Does the specified port really exist?)" << endl;
        close(port);
        return 2;
    }
    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= BOTHER;

    tty.c_ispeed = baud;
    tty.c_ospeed = baud;
    if(ioctl(port, TCSETS2, &tty))
    {
        *out << "[" << RED << "fail" << RESET << "]" << endl;
        *out << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"setting up port failed (Does the specified port " << port << " really exist?)" << endl;
        close(port);
        return 2;
    }
    *out << "[" << GREEN << "ok" << RESET << "]" << endl << endl;
    sleep(1);

    *out << "Connecting..."; out->flush();
    if(this->connect()!=0)
    {
        *out << BOLD << "print3d: " << RED << "fatal-error: " << RESET << "connection failed" << endl;
        return 2;
    }

    using namespace abc; //Why?
    sleep(5);
    tcflush(port,TCIOFLUSH);
    using namespace std;
    *out << "[" << GREEN << "ok" << RESET << "]" << endl << endl;

    return 0;
}

int Printer::connect()
{
    *out << "Printer::connect() should not be called!" << endl;
    return 2;
}

int Printer::print(string filename)
{
    *out << "Printer::print(string filename) should not be called!" << endl;
    return 2;
}

int Printer::test()
{
    *out << "Printer::test() should not be called!" << endl;
    return 2;
}

int Printer::information()
{
    *out << "Printer::information() should not be called!" << endl;
    return 2;
}

int Printer::custom_gcode(string gcode)
{
    *out << "Printer::custom_gcode() should not be called!" << endl;
    return 2;
}

ssize_t Printer::send(const char* buf)
{
    string str(buf);
    size_t size=str.size();
    return write(Printer::port, buf, size);
}

string Printer::wait_for(const char* towaitfor)
{
    string str;
    char buf;

    while(true)
    {
        read(Printer::port, &buf, 1);
        str.append(1, buf);

        //*out << buf;

        if(str.find(string(towaitfor))!=string::npos)
        {
            return str;
        }
    }
}
