#include <iostream>
#include <fstream>
#include <getopt.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <QApplication>
#include "QWatcherMainWindow.h"
#include "ui_ogreWatcher.h"      // the UI class/file for this window: Ui_WatcherMainWindow

#include "logger.h"
#include "libconfig.h++"
#include "singletonConfig.h"
#include "initConfig.h"

using namespace std;
using namespace watcher;
using namespace libconfig;

DECLARE_GLOBAL_LOGGER("ogreGlobalLogger"); 

int main(int argc, char **argv) 
{
    const char *usageString=" [options]\n" 
        "\t-c,--configFile=file\t\tUse this file to read watcher configuration information from.\n"
        "\t-s,--server=addr/name\t\tAddress or hostname of the watcher daemon to connect to.\n"
        "\t-n,--maxNodes=integer\t\tMaximum number of nodes in this test run.\n"
        "\t-y,--maxLayers=integer\t\tMaximum number of layers in this test run.\n"
        "\t-l,--logLevel=level\t\tSet the default log level. Must be one of\n"
        "                     \t\t\toff, fatal, error, warn, info, debug, or trace.\n"
        "\t-L,--logProperties=filename\t\tThe log.properties filename\n"
        ;

    Config &config=SingletonConfig::instance(); 
    SingletonConfig::lock();

    string server, logLevel;
    string configFilename(string(basename(argv[0]))+string(".cfg")); 
    string logPropsFilename(string(basename(argv[0]))+string(".log.properties")); 
    int maxLayers=-1, maxNodes=-1;
    while (true) {
        int option_index = 0;
        opterr=1;       // don't allow unknown args. 

        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"maxNodes", required_argument, 0, 'n'}, 
            {"maxLayers", required_argument, 0, 'y'},
            {"logProperties", required_argument, 0, 'L'},
            {"logLevel", required_argument, 0, 'l'},
            {"configFile", required_argument, 0, 'c'},
            {"help", no_argument, 0, 'h'},
            {0,0,0,0}
        };

        int c = getopt_long(argc, argv, "c:l:L:n:s:y:h", long_options, &option_index);

        if (c==-1) 
            break;

        switch (c) { 
            case 'c':
                configFilename=optarg;
                break;
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
            case '?':
                cout << "Usage : " << endl << basename(argv[0]) << usageString << endl;
                exit(EXIT_FAILURE); 
                break;
        }

    }
    try {
        // make sure it exists. 
        if (!boost::filesystem::exists(configFilename)) {
            cerr << "Configuration file \"" << configFilename << "\", not found. Creating it." << endl;
            ofstream f(configFilename.c_str(), ios_base::in | ios_base::out); 
            f.close();
        }
        else { 
            if (false==initConfig(config, argc, argv, configFilename))
            {
                cerr << "Error reading configuration file, unable to continue." << endl;
                cerr << "Usage: " << basename(argv[0]) << " [-c|--configFile] configfile" << endl;
                return 1;
            }
        }

        config.readFile(configFilename.c_str());
        SingletonConfig::setConfigFile(configFilename);
    }
    catch (ParseException &e)
    {
        cerr << "Error reading configuration file " << configFilename << ": " << e.what() << endl;
        cerr << "Error: \"" << e.getError() << "\" on line: " << e.getLine() << endl;
        exit(EXIT_FAILURE);  // !!!
    }

    // now that the default config has been loaded (or not), load the command line args
    // to override.
    libconfig::Setting &root=config.getRoot();
    if (!root.lookupValue("logProperties", logPropsFilename)) {
        cerr << "Unable to find logProperties setting in the configuration file, using default: " << logPropsFilename << endl;
        root.add("logProperties", Setting::TypeString)=logPropsFilename;
    }
    else 
        root["logProperties"]=logPropsFilename;

    if (!server.empty()) {
        if (!root.exists("server"))
            root.add("server", Setting::TypeString)=server;
        else
            root["server"]=server;
    }
    if (-1!=maxNodes) {
        if (!root.exists("maxNodes"))
            root.add("maxNodes", Setting::TypeInt)=maxNodes;
        else
            root["maxNodes"]=maxNodes;
    }
    if (-1!=maxLayers) {
        if (!root.exists("maxLayers"))
            root.add("maxLayers", Setting::TypeInt)=maxLayers;
        else
            root["maxLayers"]=maxLayers;
    }
    SingletonConfig::unlock();

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
    QWatcherMainWindow window(ui); 

    // setup app signals/slots
    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    QObject::connect(ui.action_Quit, SIGNAL(activated()), &app, SLOT(quit()));

    // Let's go!
    window.show();
    return app.exec();
}
