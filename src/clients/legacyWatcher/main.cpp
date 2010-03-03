/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file main.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <iostream>
#include <QTimer>
#include <string>
#include <GL/glut.h>
#include <getopt.h>

#include "watcherMainWindow.h"
#include "watcherScrollingGraphControl.h"

#include "logger.h"
#include "libconfig.h++"
#include "initConfig.h"
#include "singletonConfig.h"

using namespace std;
using namespace watcher;
using namespace libconfig;

DECLARE_GLOBAL_LOGGER("watcherGlobalLogger"); 

int main(int argc, char *argv[])
{
    TRACE_ENTER();

    const char *usageString=" -c,--configFile=cfgFile [-s,--server=server][-l,--logLevel=[debug|info|warn|error|fatal|off] -h,--help"; 

    for (int i=0; i!=argc; i++)
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "-?")) {
            cout << "Usage : " << basename(argv[0]) << usageString << endl;
            exit(EXIT_FAILURE);
        }

    string configFilename;
    SingletonConfig::lock(); 
    Config &config=SingletonConfig::instance(); 
    try {
        if (false==initConfig(config, argc, argv, configFilename))
        {
            cerr << "Error reading configuration file, unable to continue." << endl;
            cerr << "Usage: " << basename(argv[0]) << usageString << endl;
            return 1;
        }
        SingletonConfig::setConfigFile(configFilename);
    }
    catch (const SettingException &e) {
        LOG_ERROR("Error in configuration setting \"" << e.getPath() << "\"");
    }

    // Override config for server if given on the command line
    for (int i=0; i!=argc; i++) {
        if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--server")) { 
            if (i!=argc) { 
                libconfig::Setting &root=config.getRoot();
                string prop="server";
                if (!root.exists(prop))
                    root.add(prop, Setting::TypeString);
                root[prop]=argv[i+1]; 
                i++;
            }
            else { 
                cerr << "Error in \"server\" arguement: you must specify the server!" << endl;
                exit(EXIT_FAILURE); 
            }
        }
    }

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

    QApplication::setColorSpec(QApplication::CustomColor);
    QApplication app(argc, argv);

    WatcherMainWindow *window = new WatcherMainWindow;
    Ui::MainWindow ui;
    ui.setupUi(window);

    ui.menuLayers->setTearOffEnabled(true);
    ui.menuView->setTearOffEnabled(true);

    QObject::connect(ui.quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), ui.manetGLViewWindow, SLOT(saveConfiguration()));

    // Is there a way to get this from within manetGLView via theApp or something?
    ui.manetGLViewWindow->setLayerMenu(ui.menuLayers);
    ui.manetGLViewWindow->setPlaybackSlider(ui.playbackSlider);

    // 
    // Connect the scrolling graph dialog controller to other bits.
    //
    {
        WatcherScrollingGraphControl *sgc=WatcherScrollingGraphControl::getWatcherScrollingGraphControl();

        // Add the dyanmic graphs to this menu.
        sgc->setMenu(ui.menuGraph);

        QObject::connect(ui.manetGLViewWindow, SIGNAL(nodeDataInGraphsToggled(unsigned int)), sgc, SLOT(toggleNodeDataInGraphs(unsigned int)));
        QObject::connect(sgc, SIGNAL(nodeDataInGraphsToggled(unsigned int)), ui.manetGLViewWindow, SLOT(toggleNodeSelectedForGraph(unsigned int)));

        QObject::connect(ui.manetGLViewWindow, SIGNAL(nodeDataInGraphsShowed(unsigned int, bool)), sgc, SLOT(showNodeDataInGraphs(unsigned int, bool)));
        QObject::connect(sgc, SIGNAL(nodeDataInGraphsShowed(unsigned int, bool)), ui.manetGLViewWindow, SLOT(showNodeSelectedForGraph(unsigned int, bool)));
    }

    srand(time(NULL));

    glutInit(&argc, argv); 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    if (!ui.manetGLViewWindow->loadConfiguration())
    {
        LOG_FATAL("Error in cfg file, unable to continue"); 
        // write out what we have.
        SingletonConfig::saveConfig();
        TRACE_EXIT();
        return EXIT_FAILURE; 
    }

    window->show();

    TRACE_EXIT();
    return app.exec();
}
