#include "../include/AnycubicI3Mega.h"

void AnycubicI3Mega::wait()
{
    wait_for("ok");
    wait_for("\n");
}

int AnycubicI3Mega::stop()
{
	if(details) {*out << "Stopping..."; out->flush();}
	printing=false;
	send("M2\n"); wait();
	send("M117\n");
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
    else {*out << "ok" << endl;}

    return 0;
}

int AnycubicI3Mega::connect()
{
    //send("N0 M110 *3\n"); wait();
    wait_for("start");

    return 0;
}

int AnycubicI3Mega::test()
{
    if(details) {*out << "Homing x axis..."; out->flush();}
    send("G28 X0\n"); wait();
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
    else {*out << "ok" << endl;}

    if(details) {*out << "Homing y axis..."; out->flush();}
    send("G28 Y0\n"); wait();
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
    else {*out << "ok" << endl;}

    if(details) {*out << "Homing z axis..."; out->flush();}
    send("G28 Z0\n"); wait();
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
    else {*out << "ok" << endl;}

    return 0;
}

int AnycubicI3Mega::print(string filename)
{
    ifstream input(filename.c_str());
    if(!input)
    {
        if(details) {*out << endl;
        *out << BOLD << "print3d: " << RED << "error: " << RESET << "printing failed: file not found: " << filename << endl;}
        else {*out << "err " << "file not found: " << filename << endl;}
    }
    int count = std::count(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>(), '\n');
    input.close();
    input.open(filename.c_str());

    if(details) {*out << "Printing..." << endl;}

    int step=1;
    printing=true;
    while(input)
    {
    	if(!printing)
    	{
    	    if(details) {*out << "Printing..." << "[" << GREEN << "fail" << RESET << "]" << endl;}
    	    else {*out << "err " << "canceled" << endl;}
    	    input.close();
    	    return 2;
    	}

        if(step<=count)
        {
            string line;
            getline(input, line);
            string line2=line;

            send(line2.append("\n").c_str());

            if(details) {*out << "Step " << step << "/" << count << ": \"" << line << "\"..."; out->flush();}
            else {*out << step << " " << count << endl;}

            wait();

            step++;
            if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
        }
        else
        {
        	break;
        }
    }
    printing=false;
    if(details) {*out << "Printing..." << "[" << GREEN << "ok" << RESET << "]" << endl;}
    else {*out << "ok" << endl;}

    input.close();

    return 0;
}

int AnycubicI3Mega::information()
{
    if(details) {*out << "Getting firmware version and capabilities..."; out->flush();}
    send("M115\n");
    string firmware=wait_for("ok"); wait_for("\n");
    firmware=firmware.substr(0, firmware.size()-3);
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
    *out << firmware << endl;

    return 0;
}

int AnycubicI3Mega::custom_gcode(string gcode)
{
    if(details) {*out << "Sending gcode: \"" << gcode << "\"..."; out->flush();}
    send(gcode.append("\n").c_str());
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;
    *out << "Receiving response..."; out->flush();}
    string response=wait_for("ok"); string respone2 = wait_for("\n");
    if(details) {*out << "[" << GREEN << "ok" << RESET << "]" << endl;}
    *out << response << respone2.substr(0, respone2.size()-1) << endl;

    return 0;
}
