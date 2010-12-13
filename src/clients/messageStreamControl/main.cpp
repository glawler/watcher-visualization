#include <iostream>
#include <fstream>
#include <getopt.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <QApplication>
#include "ui_messageStreamControl.h"      

#include "logger.h"

using namespace std;
using namespace watcher;

DECLARE_GLOBAL_LOGGER("messageStreamControl"); 

int main(int argc, char **argv) 
{
    const char *usageString=" [options]\n" 
        "\t-s,--server=addr/name\t\tAddress or hostname of the watcher daemon to connect to.\n"
        "\t-l,--logLevel=level\t\tSet the default log level. Must be one of\n"
        "                     \t\t\toff, fatal, error, warn, info, debug, or trace.\n"
        "\t-L,--logProperties=filename\t\tThe log.properties filename\n"
        ;

    string server, logLevel;
    string logPropsFilename(string(basename(argv[0]))+string(".log.properties")); 
    int maxLayers=-1, maxNodes=-1;
    while (true) {
        int option_index = 0;
        opterr=1;       // don't allow unknown args. 

        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"logProperties", required_argument, 0, 'L'},
            {"logLevel", required_argument, 0, 'l'},
            {"help", no_argument, 0, 'h'},
            {0,0,0,0}
        };

        int c = getopt_long(argc, argv, "l:L:s:hH?", long_options, &option_index);

        if (c==-1) 
            break;

        switch (c) { 
            case 's':
                server=optarg;
                break;
            case 'n':
                maxNodes=boost::lexical_cast<int>(optarg); 
                break;
            case 'l':
                logLevel=optarg;
                if (logLevel!="off" && logLevel!="fatal" && logLevel!="error" && logLevel!="warn" && 
                        logLevel!="info" && logLevel!="debug" && logLevel!="trace") { 
                    cout << endl << "logLevel must be one of off, fatal, error, warn, info, debug, or trace." << endl;
                    cout << "Usage : " << endl << basename(argv[0]) << usageString << endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'L':
                logPropsFilename=optarg;
                break;
            case 'y':
                maxLayers=boost::lexical_cast<int>(optarg); 
                break;
            case 'h':
            case 'H':
            case '?':
                cout << "Usage : " << endl << basename(argv[0]) << usageString << endl;
                exit(EXIT_FAILURE); 
                break;
        }

    }

    if (server.empty()) {
        std::cerr << "-s,--server is a required option." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!boost::filesystem::exists(logPropsFilename)) {
        cerr << "Log properties file not found - logging disabled." << endl;
        Logger::getRootLogger()->setLevel(Level::getOff());
    }
    else {
        LOAD_LOG_PROPS(logPropsFilename); 
        LOG_INFO("Logger initialized from file \"" << logPropsFilename << "\"");
    }
    if (!logLevel.empty()) {
        cout << "Setting default log level to " << logLevel << endl;
        Logger::getRootLogger()->setLevel(Level::toLevel(logLevel)); 
    }

    QApplication::setColorSpec(QApplication::CustomColor);
    QApplication app(argc, argv);
    Ui_WatcherMainWindow ui; 
    QWatcherMainWindow window(ui, server); 

    // setup app signals/slots
    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    QObject::connect(ui.action_Quit, SIGNAL(activated()), &app, SLOT(quit()));

    // Let's go!
    window.show();
    return app.exec();
}
