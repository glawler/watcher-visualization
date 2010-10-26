#include <QtGui>
#include <libconfig.h++>
#include <values.h>  // DBL_MAX

#include "watcherQtGUIConfig.h"
#include "watcherConfigurationDialog.h"
#include "backgroundImage.h"
#include "logger.h"
#include "singletonConfig.h"

namespace watcher 
{
    INIT_LOGGER(WatcherQtGUIConfig, "WatcherGUIConfig.WatcherQtGUIConfig");

    WatcherQtGUIConfig::WatcherQtGUIConfig(QWidget *parent) : QWidget(parent) {
        TRACE_ENTER();
        TRACE_EXIT();
    }
    WatcherQtGUIConfig::~WatcherQtGUIConfig() { 
        TRACE_ENTER();
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::toggleMonochrome(bool isOn) {
        TRACE_ENTER();
        static float previousBackgroundColor[4]={0.0, 0.0, 0.0, 0.0}; 
        monochromeMode=isOn;
        if (isOn) {
            memcpy(previousBackgroundColor, rgbaBGColors, sizeof(previousBackgroundColor));
            rgbaBGColors[0]=1.0;
            rgbaBGColors[1]=1.0;
            rgbaBGColors[2]=1.0;
            rgbaBGColors[3]=1.0;
        }
        else {
            memcpy(rgbaBGColors, previousBackgroundColor, sizeof(rgbaBGColors)); 
        }
        emit monochromeToggled(isOn); 
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::toggleThreeDView(bool isOn) {
        TRACE_ENTER();
        threeDView=isOn;
        emit threeDViewToggled(isOn); 
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::toggleBackgroundImage(bool isOn) {
        TRACE_ENTER();
        LOG_DEBUG("Turning background image " << (isOn==true?"on":"off")); 
        backgroundImage=isOn; 
        emit backgroundImageToggled(isOn); 
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::toggleLoopPlayback(bool isOn) {
        TRACE_ENTER();
        LOG_DEBUG("Toggling looped playback: " << (isOn==true?"on":"off"));
        autorewind=isOn;
        emit loopPlaybackToggled(isOn);
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::showPlaybackTime(bool isOn) {
        TRACE_ENTER();
        showPlaybackTimeInStatusString=isOn;
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::showPlaybackRange(bool isOn) {
        TRACE_ENTER();
        showPlaybackRangeString=isOn;
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::showWallTime(bool isOn) {
        TRACE_ENTER();
        showWallTimeinStatusString=isOn;
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::toggleGlobalView(bool isOn) {
        TRACE_ENTER();
        showGlobalView=isOn;
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::toggleGroundGrid(bool isOn) {
        TRACE_ENTER();
        showGroundGrid=isOn;
        TRACE_EXIT();
    }
    void WatcherQtGUIConfig::spawnBackgroundColorDialog() {
        QRgb rgb=0xffffffff;
        bool ok=false;
        rgb=QColorDialog::getRgba(rgb, &ok);
        if (ok) {
            rgbaBGColors[0]=qRed(rgb)/255.0;
            rgbaBGColors[1]=qGreen(rgb)/255.0;
            rgbaBGColors[2]=qBlue(rgb)/255.0;
            rgbaBGColors[3]=qAlpha(rgb)/255.0;
        }
    }
    void WatcherQtGUIConfig::loadBackgroundImage(void) {
        QString filename;
        filename=QFileDialog::getOpenFileName(this, "Choose a background image file", ".", "*.bmp"); 
        if (!filename.isEmpty()) {
            if (!BackgroundImage::getInstance().loadImageFile(filename.toStdString())) {
                if (QMessageBox::Yes==QMessageBox::question(NULL, 
                            tr("Error loading file"), tr("Error loading the image file. Try again?"), QMessageBox::Yes, QMessageBox::No))
                    loadBackgroundImage();
            }
            else {
                toggleBackgroundImage(true);
                emit enableBackgroundImage(true);
            }
        }
    }
    void WatcherQtGUIConfig::setGPSScale() {
        TRACE_ENTER();
        bool ok;
        double value=QInputDialog::getDouble(this, tr("GPS Scale"), tr("Please enter the new GPS Scale value"), 
                gpsScale, 1, DBL_MAX, 1, &ok);

        if (ok) {
            LOG_DEBUG("Setting GPS scale to " << value); 

            double oldValue=gpsScale; 
            gpsScale=value;
            emit gpsScaleUpdated(oldValue); 

            libconfig::Config &cfg=SingletonConfig::instance();
            SingletonConfig::lock();
            libconfig::Setting &root=cfg.getRoot();
            root["gpsScale"]=gpsScale;
            SingletonConfig::unlock();
        }
        TRACE_EXIT();
    }
    bool WatcherQtGUIConfig::loadConfiguration() {
        if (WatcherGUIConfig::loadConfiguration()) {
            libconfig::Config &cfg=SingletonConfig::instance();
            SingletonConfig::lock();
            libconfig::Setting &root=cfg.getRoot();
            if (!root.lookupValue("server", serverName)) {
                serverName="localhost"; 
                LOG_WARN("Please specify the server name in the cfg file");
                LOG_WARN("I set the default to localhost, but that may not be what you want."); 
                root.add("server", libconfig::Setting::TypeString)=serverName;
            }

            while (maxNodes==0 || maxLayers==0 || serverName.empty()) { 
                WatcherConfigurationDialog *d=new WatcherConfigurationDialog(serverName, maxNodes, maxLayers);  
                d->setModal(true); 
                d->exec();
                LOG_DEBUG("WatcherConfigDialog result: " << ((d->result()==QDialog::Rejected)?"Rejected":"Accepted")); 
                if (d->result()==QDialog::Rejected) { 
                    SingletonConfig::unlock();
                    LOG_FATAL("User did not give needed info to start, exiting."); 
                    return false;
                }
                LOG_DEBUG("Got values from conf dialog - server: " << serverName << ", maxNodes: " << maxNodes << ", maxLayers: " << maxLayers); 
                if (!root.exists("server"))
                    root.add("server", libconfig::Setting::TypeString);
                root["server"]=serverName;
                if (!root.exists("maxNodes"))
                    root.add("maxNodes", libconfig::Setting::TypeInt);
                root["maxNodes"]=static_cast<int>(maxNodes);
                if (!root.exists("maxLayers"))
                    root.add("maxLayers", libconfig::Setting::TypeInt);
                root["maxLayers"]=static_cast<int>(maxLayers);
            }

            SingletonConfig::unlock();

            // Let anyone who's interested in the config get notified of init settings.
            emit threeDViewToggled(threeDView);
            emit monochromeToggled(monochromeMode);
            emit backgroundImageToggled(backgroundImage);
            emit globalViewToggled(showGlobalView); 
            emit groundGridToggled(showGroundGrid);
            emit loopPlaybackToggled(autorewind);
            emit enableStreamFiltering(messageStreamFiltering);
            emit checkPlaybackTime(showPlaybackTimeInStatusString);
            emit checkPlaybackRange(showPlaybackRangeString);
            emit checkWallTime(showWallTimeinStatusString);
            emit enableBackgroundImage(backgroundImage);        // greys the GUI checkbox
            toggleBackgroundImage(backgroundImage);             // checks/unchecks the checkbox
        }
        return true;
    }
    bool WatcherQtGUIConfig::saveConfiguration() {
        return WatcherGUIConfig::saveConfiguration(); 
    }
}
