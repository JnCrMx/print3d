#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <linux/limits.h>
#include "include/Printer.h"
#include "include/FreeSculpt.h"
#include "include/GeeetechPrusaI3X.h"
#include "include/AnycubicI3Mega.h"
#include "include/Colors.h"
#include "include/Log.h"

#define ACTION_MAX 1

using namespace std;

Printer* find_printer(string model, int USB);
int start_daemon(string port, string model);
int stop_daemon();
int check_daemon();
pid_t find_daemon();
int pass_action(char action, string argument, pid_t pid);

int usb_handle;
int fifo_handle;

int action;
string actionArg;

Printer* printer;

typedef void (*sighandler_t)(int);
static sighandler_t handle_signal (int sig_nr, sighandler_t signalhandler)
{
   struct sigaction neu_sig, alt_sig;
   neu_sig.sa_handler = signalhandler;
   sigemptyset (&neu_sig.sa_mask);
   neu_sig.sa_flags = SA_RESTART;
   if (sigaction (sig_nr, &neu_sig, &alt_sig) < 0)
      return SIG_ERR;
   return alt_sig.sa_handler;
}

static void user_signal(int signr)
{
    if(signr==SIGUSR1)
    {
        //syslog(LOG_INFO, "Starting reading from /var/print3d.pipe");

        int len=PATH_MAX + ACTION_MAX;

        char* buffer=new char[len];
        read(fifo_handle, buffer, len);

        //syslog(LOG_INFO, "Stopped reading from /var/print3d.pipe");

        char option=buffer[0]; buffer++;
        string argument(buffer);

        cout << option << endl;
        cout << argument << endl;

        switch(option)
        {
			case 'I':
				action|=1;
				break;
			case 'T':
				action|=2;
				break;
			case 'G':
				action|=4;
				break;
			case 'P':
				action|=8;
				break;
			case 'S':
				action|=16;
				break;
			default:
				break;
        }
        actionArg = argument;

        if((action&16)==16)
        {
        	printer->stop();
        }
    }
    else if(signr==SIGKILL)
    {
        syslog(LOG_INFO, "print3d daemon died");
        unlink("/var/print3d.pid");

        close(usb_handle);
        closelog();

        raise(SIGKILL);
    }
    else if(signr==SIGTERM)
    {
        syslog(LOG_INFO, "print3d daemon stopped");
        unlink("/var/print3d.pid");

        close(usb_handle);
        closelog();

        raise(SIGKILL);
    }
}

int main(int argc, char* argv[])
{
    string port("/dev/ttyUSB0");
    action=0;
    int daemonAction=0;
    string actionArgP;
    string* actionArgG = new string[256];
    string model("nil");
    int custom_count=0;

    for(int i=1;i<argc;i++)
    {
        string arg(argv[i]);
        if(arg=="-p")
        {
            i++;
            if(i>=argc)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting port: [-p port]" << endl;
                return 2;
            }
            port=string(argv[i]);
            if(port.find("-")==0)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting port: [-p port]" << endl;
                return 2;
            }
        }
        else if(arg=="-m")
        {
            i++;
            if(i>=argc)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting printermodel: [-m printermodel]" << endl;

                cout << BOLD << "    models:" << RESET << endl;
                cout << "    - FreeSculptEXT1" << endl;
                cout << "    - GeeetechPrusaI3X" << endl;
                cout << "    - AnycubicI3Mega" << endl;

                return 2;
            }
            model=string(argv[i]);
            if(model.find("-")==0)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting printermodel: [-m printermodel]" << endl;

                cout << BOLD << "    models:" << RESET << endl;
                cout << "    - FreeSculptEXT1" << endl;
                cout << "    - GeeetechPrusaI3X" << endl;
                cout << "    - AnycubicI3Mega" << endl;

                return 2;
            }
        }
        else if(arg=="-T")
        {
            action|=2;
        }
        else if(arg=="-P")
        {
            action|=8;
            i++;
            if(i>=argc)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting file: [-P file]" << endl;
                return 2;
            }
            actionArgP=string(argv[i]);
            if(actionArgP.find("-")==0)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting file: [-P file]" << endl;
                return 2;
            }
        }
        else if(arg=="-I")
        {
            action|=1;
        }
        else if(arg=="-G")
        {
            if(custom_count>255)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"there is a maximum of 256 custom gcode actions" << endl;
                return 2;
            }

            action|=4;
            i++;
            if(i>=argc)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting string: [-G gcode]" << endl;
                return 2;
            }
            actionArgG[custom_count]=string(argv[i]);
            if(actionArgG[custom_count].find("-")==0)
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"expecting file: [-G gcode]" << endl;
                return 2;
            }
            custom_count++;
        }
        else if(arg=="-S")
        {
        	action|=16;
        }
        else if(arg=="--help")
        {
            cout << "Usage: print-3d [options]" << endl;
            cout << "Options:" << endl;
            cout << "  " << "--help" << "\t\t\t" << "Display this information" << endl;
            cout << "  " << "--start-daemon" << "\t\t" << "Start printer daemon" << endl;
            cout << "  " << "--stop-daemon" << "\t\t\t" << "Stop printer daemon" << endl;
            cout << "  " << "--restart-daemon" << "\t\t" << "Restart printer daemon" << endl;
            cout << "  " << "-p <port>" << "\t\t\t" << "Use <port> for connection" << endl;
            cout << "  " << "-m <printermodel>" << "\t\t" << "Use <printermodel>" << endl;
            cout << "  " << "-T" << "\t\t\t\t" << "Start test" << endl;
            cout << "  " << "-P <file>" << "\t\t\t" << "Start printing gcode file <file>" << endl;
            cout << "  " << "-I" << "\t\t\t\t" << "Start fetching information" << endl;
            cout << "  " << "-G <gcode>" << "\t\t\t" << "Send <gcode>" << endl;
            cout << "  " << "-S" << "\t\t\t\t" << "Stop" << endl;
            cout << endl;
            cout << "Multiple options -p -m will overwrite the old one" << endl;
            cout << "Actions -T -I -P may be called only once" << endl;
            cout << "Action -G may be called up to 256 times" << endl;
            cout << "Multiple actions are possible, they will called in order" << endl;
            cout << endl;

            return 0;
        }
        else if(arg=="--start-daemon")
        {
            daemonAction|=1;
        }
        else if(arg=="--stop-daemon")
        {
            daemonAction|=2;
        }
        else if(arg=="--restart-daemon")
        {
            daemonAction|=4;
        }
        else
        {
            cout << BOLD << "print3d: " << YELLOW << "warning: " << RESET <<"unknown argument " << arg << ": ignoring" << endl;
        }
    }

    if(daemonAction!=0)
    {
        if((daemonAction&1)==1)
        {
            int running=check_daemon();
            if(running==0)
            {
                start_daemon(port, model);
            }
            else
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET << "daemon already running. stop it with \"print3d --stop-daemon\"" << endl;
            }
        }
        else if((daemonAction&2)==2)
        {
            int running=check_daemon();
            if(running==1)
            {
                stop_daemon();
            }
            else
            {
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET << "daemon not running. start it with \"print3d --start-daemon\"" << endl;
            }
        }
        else if((daemonAction&4)==4)
        {
            int running=check_daemon();
            if(running==1)
            {
                stop_daemon();
            }
            start_daemon(port, model);
        }
    }
    else
    {
        if(action==0)
        {
            cout << BOLD << "print3d: " << RED << "error: " << RESET << "no action specified: terminating; use --help for more information" << endl;
            return 1;
        }

        if(check_daemon()==1)
        {
        	//TODO: Mark
            cout << "Passing actions to active daemon..." << endl;

            pid_t pid=find_daemon();
            if(pid==-1)
            {
                cout << "[" << GREEN << "fail" << RESET << "]" << endl;
                return 2;
            }

            fifo_handle=open("/var/print3d.pipe", O_RDWR | O_NONBLOCK);
            if(fifo_handle == -1)
            {
                cout << "[" << GREEN << "fail" << RESET << "]" << endl;
                return 2;
            }

            if((action&1)==1) //Information
            {
            	if(pass_action('I', "", pid)!=0)
            	{
            		cout << BOLD << "print3d: " << RED << "error: " << RESET << "passing action -I failed" << endl;
            	}
            }
            if((action&2)==2) //Test
            {
            	if(pass_action('T', "", pid)!=0)
            	{
            		cout << BOLD << "print3d: " << RED << "error: " << RESET << "passing action -T failed" << endl;
            	}
            }
            if((action&4)==4) //Custom gcode
            {
            	for(int i=0;i<custom_count;i++)
            	{
            		if(pass_action('G', actionArgG[i], pid)!=0)
            		{
            			cout << BOLD << "print3d: " << RED << "error: " << RESET << "passing action -G failed" << endl;
            		}
            	}
            }
            if((action&8)==8) //Print
            {
            	string dir(get_current_dir_name());
            	dir=dir.append("/").append(actionArgP);
            	if(pass_action('P', dir, pid)!=0)
            	{
            		cout << BOLD << "print3d: " << RED << "error: " << RESET << "passing action -P failed" << endl;
            	}
            }
            if((action&16)==16) //Stop
            {
            	if(pass_action('S', "", pid)!=0)
            	{
            		cout << BOLD << "print3d: " << RED << "error: " << RESET << "passing action -S failed" << endl;
            	}
            }

            cout << "Passing actions to active daemon..." << "[" << GREEN << "ok" << RESET << "]" << endl;
        }
        else
        {
            cout << "Setting up port..."; cout.flush();
            usb_handle = open (port.c_str(), O_RDWR | O_NOCTTY);

            printer=find_printer(model, usb_handle);
            if(!printer)
            {
                cout << "[" << RED << "fail" << RESET << "]" << endl;
                cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"printermodel not set or unknown. Use: [-m printermodel] or [-m] to see all models" << endl;
                close(usb_handle);
                return 2;
            }

            printer->setup();
            sleep(1);

            if((action&1)==1) //Information
            {
                cout << "Starting fetching information:" << endl;
                int status = printer->information();

                cout << endl;
                if(status!=0)
                {
                    cout << BOLD << "print3d: " << RED << "error: " << RESET << "fetching information failed" << endl;
                }

                sleep(1);
            }
            if((action&2)==2) //Test
            {
                cout << "Starting test:" << endl;
                int status = printer->test();

                cout << endl;
                if(status!=0)
                {
                    cout << BOLD << "print3d: " << RED << "error: " << RESET << "test failed" << endl;
                }

                sleep(1);
            }
            if((action&4)==4) //Custom gcode
            {
                for(int i=0;i<custom_count;i++)
                {
                	cout << "Starting execution of custom gcode:" << endl;
                    int status = printer->custom_gcode(actionArgG[i]);

                    cout << endl;
                    if(status!=0)
                    {
                        cout << BOLD << "print3d: " << RED << "error: " << RESET << "executing gcode failed" << endl;
                    }

                    sleep(1);
                }
            }
            if((action&8)==8) //Print
            {
                cout << "Starting print job:" << endl;

                {
                    int status=printer->print(actionArgP);

                    cout << endl;
                    if(status!=0)
                    {
                        cout << BOLD << "print3d: " << RED << "error: " << RESET << "printing failed" << endl;
                    }
                }
                sleep(1);
            }
            if((action&16)==16)
            {
                cout << "Stopping printer:" << endl;
                int status=printer->stop();

                cout << endl;
                if(status!=0)
                {
                	cout << BOLD << "print3d: " << RED << "error: " << RESET << "printing failed" << endl;
                }
                sleep(1);
            }
        }
    }

    return 0;
}

int start_daemon(string port, string model)
{
    cout << "Starting daemon..."; cout.flush();

    pid_t childpid;

    if((childpid = fork()) == -1)
    {
        cerr << "fork" << endl;
        return 2;
    }

    if(childpid == 0)
    {
        setsid();
        handle_signal(SIGHUP, SIG_IGN);
        chdir ("/");
        umask (0);
        for (int i = sysconf (_SC_OPEN_MAX); i > 0; i--)
            close (i);

        Log *log = new Log("print3d", LOG_LPR);
        cout.rdbuf(log);

        cout << "print3d daemon started" << endl;

        ofstream pidfile("/var/print3d.pid");
        pidfile << getpid() << endl;
        pidfile.flush();
        pidfile.close();

        if(access("/var/print3d.pipe", F_OK) == -1)
        {
            mkfifo("/var/print3d.pipe", ACCESSPERMS);
        }

        fifo_handle=open("/var/print3d.pipe", O_RDWR | O_NONBLOCK);
        if(fifo_handle == -1)
        {
            return 2;
        }

        //Connect to printer
        cout << "Setting up port..."; cout.flush();
        usb_handle = open (port.c_str(), O_RDWR | O_NOCTTY);

        printer=find_printer(model, usb_handle);
        if(!printer)
        {
            cout << "[" << RED << "fail" << RESET << "]" << endl;
            cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"printermodel not set or unknown. Use: [-m printermodel] or [-m] to see all models" << endl;
            close(usb_handle);
            return 2;
        }

        if(printer->setup()!=0)
        {
        	cout << BOLD << "print3d: " << RED << "fatal-error: " << RESET <<"could not connect to printer" << endl;

            syslog(LOG_INFO, "print3d daemon stopped");
            unlink("/var/print3d.pid");

            close(usb_handle);
            closelog();

        	return 2;
        }
        sleep(1);

        handle_signal(SIGUSR1, user_signal);
        handle_signal(SIGKILL, user_signal);
        handle_signal(SIGTERM, user_signal);

        while(true)
        {
        	//TODO: Mark

        	cout << action << endl;

        	if((action&1)==1)	//Info
        	{
                cout << "Starting fetching information" << endl;

        		action&=~1;

        		printer->output(new ofstream("/var/print3d.pipe"), false);
        		printer->information();
        		printer->output(&cout, true);
        	}
        	if((action&2)==2)	//Test
        	{
                cout << "Starting test" << endl;

        		action&=~2;

        		printer->output(new ofstream("/var/print3d.pipe"), false);
        		printer->test();
        		printer->output(&cout, true);
        	}
        	if((action&4)==4)	//Custom gcode
        	{
        		cout << "Starting execution of custom gcode" << endl;

        		action&=~4;

        		printer->output(new ofstream("/var/print3d.pipe"), false);
        		printer->custom_gcode(actionArg);
        		printer->output(&cout, true);
        	}
        	if((action&8)==8)	//Print
        	{
                cout << "Starting print job: " << actionArg << endl;

        		action&=~8;

        		printer->output(new ofstream("/var/print3d.pipe"), false);
        		printer->print(actionArg);
        		printer->output(&cout, true);
        	}
        	if((action&16)==16)	//Stop
        	{
        		cout << "Stopping" << endl;

        		action&=~16;

        		printer->output(new ofstream("/var/print3d.pipe"), false);
        		printer->stop();
        		printer->output(&cout, true);
        	}

            sleep(1);
        }

        closelog();

        return 0;
    }
    else
    {
        cout << "[" << GREEN << "ok" << RESET << "]" << endl;
    }

    return 0;
}

int check_daemon()
{
    if(access("/var/print3d.pid", F_OK) == -1)
    {
        return 0;
    }
    return 1;
}

int stop_daemon()
{
    cout << "Stopping daemon..."; cout.flush();

    pid_t pid=find_daemon();
    if(pid==-1)
    {
        cout << "[" << RED << "fail" << RESET << "]" << endl;
        return 2;
    }

    kill(pid, SIGTERM);

    unlink("/var/print3d.pid");

    cout << "[" << GREEN << "ok" << RESET << "]" << endl;

    return 0;
}

pid_t find_daemon()
{
    if(access("/var/print3d.pid", F_OK) == -1)
    {
        return -1;
    }

    ifstream pidfile("/var/print3d.pid");

    pid_t pid;
    pidfile >> pid;

    pidfile.close();

    return pid;
}

Printer* find_printer(string model, int USB)
{
    Printer* printer=NULL;
    if(model=="FreeSculptEXT1")
    {
        printer=new FreeSculpt(USB, 1, &cout);
    }
    else if(model=="GeeetechPrusaI3X")
    {
        printer=new GeeetechPrusaI3X(USB, &cout);
    }
    else if(model=="AnycubicI3Mega")
    {
    	printer=new AnycubicI3Mega(USB, &cout);
    }

    return printer;
}

int pass_action(char action, string argmuent, pid_t pid)
{
	using namespace abc; //Why?
    tcflush(fifo_handle,TCIOFLUSH);
    using namespace std;
	
    int len=PATH_MAX + ACTION_MAX;
    char* buffer=new char[len];
	buffer[0]=action; buffer++;

	strcpy(buffer, argmuent.c_str()); buffer--;

	write(fifo_handle, buffer, len);
	
	kill(pid, SIGUSR1); //Interrupt daemon

	return 0;
}
