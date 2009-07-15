/** 
 * @file watcherMainWindow.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef WATRCHER_AMINE_WAINFODW_YOU_BETCHA_H
#define WATRCHER_AMINE_WAINFODW_YOU_BETCHA_H

#include <string>
#include <QtGui/QMainWindow>
#include "logger.h"

namespace watcher
{
    class WatcherMainWindow : public QMainWindow
    {
        Q_OBJECT

        public:

            explicit WatcherMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
            ~WatcherMainWindow(); 

            public slots:

            signals:

        protected:

        private:

                DECLARE_LOGGER();
    };
}

#endif // WATRCHER_AMINE_WAINFODW_YOU_BETCHA_H
