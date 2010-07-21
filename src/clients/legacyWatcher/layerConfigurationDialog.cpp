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
 * @file layerConfigurationDialog.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-13
 */
#include <Qt/qcolordialog.h>
#include <Qt/qfontdialog.h>
#include <Qt/qmessagebox.h>
#include <boost/foreach.hpp>
#include "layerConfigurationDialog.h"
#include "logger.h"

using namespace std;

namespace watcher { 

    INIT_LOGGER(LayerConfigurationDialog, "LayerConfigurationDialog"); 

    LayerConfigurationDialog::LayerConfigurationDialog(QWidget *parent, Qt::WindowFlags f) : 
        QDialog(parent, f), currentLayer(NULL)
    {
        TRACE_ENTER();
        setupUi(this);

        // previewWindow.setLayerData(&l); 

        TRACE_EXIT();
    }

    void LayerConfigurationDialog::addLayer(WatcherLayerData *l)
    {
        layers.push_back(l);
        QListWidgetItem *item=new QListWidgetItem(l->layerName.c_str(), layerListWidget); 
        QFont f(item->font());
        f.setBold(l->isActive); 
        item->setFont(f); 
    }

    LayerConfigurationDialog::~LayerConfigurationDialog()
    {
        TRACE_ENTER();
        // remove test label and test floating label
        TRACE_EXIT();
    }

    void LayerConfigurationDialog::layerToggle(std::string &name)
    {
        QString layerName(name.c_str()); 
        QList<QListWidgetItem *> items=layerListWidget->findItems(layerName, Qt::MatchExactly); 
        BOOST_FOREACH(QListWidgetItem *item, items) { 
            BOOST_FOREACH(WatcherLayerData *layer, layers) {
                if (layer->layerName==item->text().toStdString()) {
                    QFont f(item->font());
                    f.setBold(layer->isActive); 
                    item->setFont(f); 
                }
            }
        }
        if (currentLayer && currentLayer->layerName==name)
            layerActiveCheckBox->setCheckState(currentLayer->isActive ? Qt::Checked : Qt::Unchecked);

    }

    bool LayerConfigurationDialog::checkForValidCurrentLayer()
    {
        if (!currentLayer) {
            QMessageBox::information(this, tr("Layer Selection"), tr("Please select a layer first.")); 
            return false;
        }
        return true;
    }

    void LayerConfigurationDialog::setFont(string &fontFamily, float &pointSize)
    {
        QFont initial(fontFamily.c_str(), pointSize); 
        bool ok;
        QFont font=QFontDialog::getFont(&ok, initial, this); 
        if (ok) {
            fontFamily=font.family().toStdString(); 
            pointSize=font.pointSize(); 
        }
    }

    void LayerConfigurationDialog::setEdgeLabelFont()
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setFont(currentLayer->edgeDisplayInfo.labelFont, currentLayer->edgeDisplayInfo.labelPointSize); 
    }

    void LayerConfigurationDialog::setNodeLabelFont()
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setFont(currentLayer->referenceLabelDisplayInfo.fontName, currentLayer->referenceLabelDisplayInfo.pointSize); 
    }

    void LayerConfigurationDialog::setFloatingLabelFont()
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setFont(currentLayer->referenceFloatingLabelDisplayInfo.fontName, currentLayer->referenceFloatingLabelDisplayInfo.pointSize); 
    }

    void LayerConfigurationDialog::setCurrentLayer(QListWidgetItem *item)
    {
        string itemText(item->text().toStdString()); 
        BOOST_FOREACH(WatcherLayerData *l, layers) { 
            if (l && l->layerName==itemText) {
                currentLayer=l;
                configureDialogWithCurrentLayer(); 
                break;
            }
        }
    }

    void LayerConfigurationDialog::configureDialogWithCurrentLayer()
    {
        // set colored buttons
        struct {
            QToolButton *b;
            const Color &c;
        } cbs[] = {
            { edgeColorButton, currentLayer->edgeDisplayInfo.color }, 
            { edgeLabelColorButton, currentLayer->edgeDisplayInfo.labelColor },
            { floatingLabelColorButton, currentLayer->referenceFloatingLabelDisplayInfo.foregroundColor }, 
            { nodeLabelColorButton, currentLayer->referenceLabelDisplayInfo.foregroundColor }, 
        };
        for (size_t i=0; i<(sizeof(cbs)/sizeof(cbs[0])); i++) {
            QString ss;
            ss.sprintf("background-color: #%02x%02x%02x", cbs[i].c.r, cbs[i].c.g, cbs[i].c.b); 
            cbs[i].b->setStyleSheet(ss); 
        }
        // set edit boxes
        edgeLabelEditBox->setText(currentLayer->edgeDisplayInfo.label.c_str()); 

        // set spinners
        edgeWidthSpinBox->setValue(currentLayer->edgeDisplayInfo.width); 

        // isActive checkbox
        layerActiveCheckBox->setCheckState(currentLayer->isActive ? Qt::Checked : Qt::Unchecked);
    }

    void LayerConfigurationDialog::layerToggled(bool toggled) 
    {
        emit layerToggled(currentLayer->layerName.c_str(), toggled);
    }

    void LayerConfigurationDialog::setColor(Color &c, QToolButton *b)
    {
        if (!checkForValidCurrentLayer()) 
            return;
        if (currentLayer) { 
            QColor clr(c.r, c.g, c.b, c.a);
            QColorDialog *d = new QColorDialog(clr, this);
            d->setOption(QColorDialog::ShowAlphaChannel, true); 
            d->exec();
            if (QDialog::Accepted==d->result()) {
                clr=d->currentColor();
                c=Color(clr.red(), clr.green(), clr.blue(), clr.alpha()); 
                QString ss;
                ss.sprintf("background-color: #%02x%02x%02x; color: #%02x%02x%02x", 
                        clr.red(), clr.green(), clr.blue(), 
                        (~clr.red())&0xFF, (~clr.green())&0xFF, (~clr.blue())&0xFF);  
                b->setStyleSheet(ss); 
            }
        }
    }

    void LayerConfigurationDialog::setNodeLabelColor() 
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setColor(currentLayer->referenceLabelDisplayInfo.foregroundColor, nodeLabelColorButton); 
    }
    void LayerConfigurationDialog::setFloatingLabelColor() 
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setColor(currentLayer->referenceFloatingLabelDisplayInfo.foregroundColor, floatingLabelColorButton); 
    }
    void LayerConfigurationDialog::setEdgeLabelColor() 
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setColor(currentLayer->edgeDisplayInfo.labelColor, edgeLabelColorButton); 
    }
    void LayerConfigurationDialog::setEdgeColor() 
    {
        if (!checkForValidCurrentLayer()) 
            return;
        setColor(currentLayer->edgeDisplayInfo.color, edgeColorButton); 
    }
    void LayerConfigurationDialog::setEdgeWidth(int w)
    {
        if (!checkForValidCurrentLayer()) 
            return;
        currentLayer->edgeDisplayInfo.width=w;
    }
    void LayerConfigurationDialog::updateEdgeLabel(QString text)
    {
        if (!checkForValidCurrentLayer()) 
            return;
        string edgeLabel=text.toStdString(); 
        LOG_INFO("Layerconfig: Set edge label to " << edgeLabel << " on layer " << currentLayer->layerName); 
        currentLayer->edgeDisplayInfo.label=text.toStdString();
    }
}

