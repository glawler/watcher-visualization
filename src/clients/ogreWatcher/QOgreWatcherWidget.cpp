
#include "QOgreWatcherWidget.h"
#include "logger.h"

namespace watcher 
{
    INIT_LOGGER(QOgreWatcherWidget, "QOgreWidget.QOgreWatcherWidget"); 
    QOgreWatcherWidget::QOgreWatcherWidget(QWidget *parent) : QOgreWidget(parent) { 
    }
    // virtual
    QOgreWatcherWidget::~QOgreWatcherWidget() {
    }
    void QOgreWatcherWidget::resetPosition() {
        mCamera->lookAt(Ogre::Vector3(0, 0, 0));
        mCamera->setPosition(Ogre::Vector3(0, 0, 300)); 
    }
}
