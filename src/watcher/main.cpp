#include <iostream>
#include <QTimer>
#include <string>

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
    Config &config=singletonConfig::instance();
    singletonConfig::lock();
    if (false==initConfig(config, argc, argv, configFilename, 'f'))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-f|--configFile] configfile [standard watcher arguments]" << endl;
        return 1;
    }
    singletonConfig::unlock();

    string logConf("log.properties");
    if (!config.lookupValue("logProperties", logConf))
    {
        cerr << "Unable to find logproperties setting in the configuration file, using default: " << logConf << endl;
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

    WatcherScrollingGraphControl *sgc=WatcherScrollingGraphControl::getWatcherScrollingGraphControl();
    QObject::connect(ui.actionGraphBandwidth, SIGNAL(toggled(bool)), sgc, SLOT(showDialogGraph(bool)));
    QObject::connect(sgc, SIGNAL(showDialog(bool)), ui.actionGraphBandwidth, SLOT(toggled(bool)));

    QObject::connect(ui.actionGraphCPUUsage, SIGNAL(toggled(bool)), sgc, SLOT(showDialogCPUUsage(bool)));
    QObject::connect(sgc, SIGNAL(showDialog(bool)), ui.actionGraphCPUUsage, SLOT(toggled(bool)));

    ui.manetGLViewWindow->runLegacyWatcherMain(argc, argv);

    window->show();

    // Save any configuration changes made during the run.
    singletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT();

    return app.exec();
}
