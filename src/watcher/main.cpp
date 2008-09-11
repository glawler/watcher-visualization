#include <iostream>
#include <QTimer>

#include "ui_watcher.h"

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

    Config &config=singletonConfig::instance();
    singletonConfig::lock();
    if (false==initConfig(config, argc, argv))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-c|--configFile] configfile" << endl;
        return 1;
    }
    singletonConfig::unlock();

    string logConf("log.properties");
    if (!config.lookupValue("logProperties", logConf))
    {
        cerr << "Unable to find logproperties setting in the configuration file, using default: " << logConf << endl;
    }

    PropertyConfigurator::configureAndWatch(logConf);

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");
    LOG_INFO("Although the legacgy watcher code does not use it, so it is not overly valuable");

    QApplication app(argc, argv);
    QMainWindow *window = new QMainWindow;
    Ui::MainWindow ui;
    ui.setupUi(window);

    ui.menuLayers->setTearOffEnabled(true);
    ui.menuView->setTearOffEnabled(true);
    ui.menuGoodwin->setTearOffEnabled(true);

    QObject::connect(ui.quitButton, SIGNAL(clicked()), &app, SLOT(quit()));

    ui.manetGLViewWindow->runLegacyWatcherMain(argc, argv);

    window->show();

    TRACE_EXIT();

    return app.exec();
}
