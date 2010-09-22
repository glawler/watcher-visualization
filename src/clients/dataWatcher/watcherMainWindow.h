/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

#include <map>
#include <boost/thread.hpp>

#include "declareLogger.h"

#include "ui_mainwindow.h"

namespace watcher {
namespace ui {

class SeriesGraphDialog; //forward decl
class WatcherStreamListDialog; //forward decl

/** The Main Window for the DataWatcher application.  This is just a wrapper
 * around Ui::MainWindow for the purpose of attaching our own slots.
 */
class MainWindow : public QMainWindow, public Ui::MainWindow {
    private:
	Q_OBJECT
	DECLARE_LOGGER();

	/** mapping from series name to the menu item in the Series menu. */
	typedef std::map<QString, SeriesGraphDialog*> SeriesMap;
	SeriesMap seriesMap;

	boost::thread *checkIOThread;
	WatcherStreamListDialog *streamsDialog;

	void checkIO();
	void closeAllGraphs();

    signals:
	void dataPointReceived(const QString& dataName, const QString& fromID, const QString& layer, qlonglong when, double value);
	void clockTick(qlonglong);

    public slots:
	void dataPointHandler(const QString& dataName, const QString& fromID, const QString& layer, qlonglong when, double value);
        void listStreams();
	void reconnect();
        void seekStream(qlonglong);
	void selectStream(unsigned long);

    public:
	MainWindow();
        void setup();
};

} // ui
} // watcher

// vim:sw=4
