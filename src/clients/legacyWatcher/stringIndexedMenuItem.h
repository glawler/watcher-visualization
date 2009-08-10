/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file stringIndexedMenuItem.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef STRINGINDEXEDMENUITEM_H
#define STRINGINDEXEDMENUITEM_H

#include <QObject>
namespace watcher 
{
    /**
     * @class StringIndexedMenuItem
     *
     * Converts a toggled(bool) QAction signal into a string baced toggle signal. Used to 
     * glue together actions and string indexed menus. 
     *
     * Example:
     *       ...
     *       QAction *action=new QAction(QString::fromStdString(message->dataName), (QObject*)this);
     *       action->setCheckable(true);
     *       StringIndexedMenuItem *item = new StringIndexedMenuItem(QString("I'm an action in a menu"));
     *       connect(action, SIGNAL(triggered(bool)), item, SLOT(showMenuItem(bool)));
     *       connect(item, SIGNAL(showMenuItem(QString, bool)), this, SLOT(activateMenuItem(QString, bool)));
     *       ...
     */
    class StringIndexedMenuItem : public QObject
    {
        Q_OBJECT

        public: 
            StringIndexedMenuItem(const QString &str_) : str(str_) {} 

       public slots:
            void showMenuItem(bool b) { emit showMenuItem(str, b); }
            void setChecked(const QString &str_, bool b) { if (str==str_) emit setChecked(b); }

       signals:
            void showMenuItem(const QString &str, bool show); 
            void setChecked(bool b); 

        private:
            const QString str;
    };
}

#endif // STRINGINDEXEDMENUITEM_H
