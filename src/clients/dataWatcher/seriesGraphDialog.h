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

#include <tr1/memory>
#include "ui_seriesGraphDialog.h"
#include "declareLogger.h"

namespace watcher {
namespace ui {

class NodeInfo; //forward decl
typedef std::tr1::shared_ptr<NodeInfo> NodeInfoPtr;

/** Dialog box for plotting data from multiple nodes for a single data series.  */
class SeriesGraphDialog : public QDialog, Ui::seriesDialog {
    private:
	DECLARE_LOGGER();

	SeriesGraphDialog(const SeriesGraphDialog&);
	SeriesGraphDialog& operator=(const SeriesGraphDialog&);

	typedef std::map<QString, NodeInfoPtr> NodeMap;
	NodeMap nodeMap; // set of all nodes with data for this series

    public:
	explicit SeriesGraphDialog(const QString&);
	~SeriesGraphDialog();
	void data_point(const QString&, qlonglong, double);
};

} // namespace
} // namespace

// vim:sw=4
