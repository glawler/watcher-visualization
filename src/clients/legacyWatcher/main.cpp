#include <iostream>
#include <QTimer>
#include <string>
#include <GL/glut.h>

#include "ui_watcher.h"
#include "watcherMainWindow.h"
#include "watcherScrollingGraphControl.h"

#include "logger.h"
#include "libconfig.h++"
#include "initConfig.h"
#include "singletonConfig.h"

using namespace std;
using namespace watcher;
using namespace libconfig;

int main(int argc, char *argv[])
{
    TRACE_ENTER();

    string configFilename;
    SingletonConfig::lock(); 
    Config &config=SingletonConfig::instance(); 
    if (false==initConfig(config, argc, argv, configFilename))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-f|--configFile] configfile [standard watcher arguments]" << endl;
        return 1;
    }
    SingletonConfig::setConfigFile(configFilename);
    SingletonConfig::unlock();

    string logConf("watcher.log.properties");
    if (!config.lookupValue("logProperties", logConf))
    {
        cerr << "Unable to find logproperties setting in the configuration file, using default: " << logConf << endl;
        config.getRoot().add("logProperties", Setting::TypeString)=logConf;
    }

    LOAD_LOG_PROPS(logConf); 

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");
    LOG_INFO("Although the legacy watcher code does not use it, so it is not overly valuable");

    QApplication app(argc, argv);
    WatcherMainWindow *window = new WatcherMainWindow;
    Ui::MainWindow ui;
    ui.setupUi(window);

    ui.menuLayers->setTearOffEnabled(true);
    ui.menuView->setTearOffEnabled(true);

    QObject::connect(ui.quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), ui.manetGLViewWindow, SLOT(saveConfiguration()));

    // 
    // Connect the scrolling graph dialog controller to other bits.
    //
    {
        WatcherScrollingGraphControl *sgc=WatcherScrollingGraphControl::getWatcherScrollingGraphControl();

        QObject::connect(ui.actionGraphBandwidth, SIGNAL(toggled(bool)), sgc, SLOT(showBandwidthGraphDialog(bool)));
        QObject::connect(sgc, SIGNAL(bandwidthDialogShowed(bool)), ui.actionGraphBandwidth, SLOT(setChecked(bool)));

        QObject::connect(ui.actionGraphLoadAverage, SIGNAL(toggled(bool)), sgc, SLOT(showLoadAverageGraphDialog(bool)));
        QObject::connect(sgc, SIGNAL(loadAverageDialogShowed(bool)), ui.actionGraphLoadAverage, SLOT(setChecked(bool)));

        // Support for generic graph-name-based dialogs is not yet supported by the main GUI window
        // Need to figure out how to do dynamic menus (or somesuch). 

        QObject::connect(ui.manetGLViewWindow, SIGNAL(nodeDataInGraphsToggled(unsigned int)), 
                          sgc, SLOT(toggleNodeDataInGraphs(unsigned int)));
        QObject::connect(sgc, SIGNAL(nodeDataInGraphsToggled(unsigned int)), 
                          ui.manetGLViewWindow, SLOT(toggleNodeSelectedForGraph(unsigned int)));

        QObject::connect(ui.manetGLViewWindow, SIGNAL(nodeDataInGraphsShowed(unsigned int, bool)), 
                          sgc, SLOT(showNodeDataInGraphs(unsigned int, bool)));
        QObject::connect(sgc, SIGNAL(nodeDataInGraphsShowed(unsigned int, bool)), 
                          ui.manetGLViewWindow, SLOT(showNodeSelectedForGraph(unsigned int, bool)));
    }

    glutInit(&argc, argv); 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    if (!ui.manetGLViewWindow->loadConfiguration())
    {
        LOG_FATAL("Error in cfg file, unable to continue"); 
        // write out what we have.
        SingletonConfig::saveConfig();
    }

    window->show();

    TRACE_EXIT();
    return app.exec();
}
