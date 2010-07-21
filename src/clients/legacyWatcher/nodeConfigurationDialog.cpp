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
 * @file nodeConfigurationDialog.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-20
 */
#include <Qt/qcolordialog.h>
#include <Qt/qfontdialog.h>
#include <Qt/qinputdialog.h>
#include <Qt/qmessagebox.h>
#include "nodeConfigurationDialog.h"
#include "logger.h"

using namespace std;

namespace watcher { 

    INIT_LOGGER(NodeConfigurationDialog, "NodeConfigurationDialog"); 

    NodeConfigurationDialog::NodeConfigurationDialog(WatcherGraph *&g, QWidget *parent, Qt::WindowFlags f) : 
        QDialog(parent, f), useNodeId(false), curNodeId(0), graph(g)
    {
        TRACE_ENTER();
        setupUi(this);
        labelWhichNode->setText("All Nodes"); // for now
        TRACE_EXIT();
    }

    NodeConfigurationDialog::~NodeConfigurationDialog()
    {
    }
    void NodeConfigurationDialog::setNodeLabelColor(void)
    {
        if (!graph)
            return;
        if (!graph->numValidNodes) {
            QMessageBox::information(this, tr("A Serious Lack of Nodes"), tr("No nodes to configure yet, sorry.")); 
            return; 
        }
        QColor clr(
                graph->nodes[0].labelColor.r, 
                graph->nodes[0].labelColor.g, 
                graph->nodes[0].labelColor.b, 
                graph->nodes[0].labelColor.a);
        QColorDialog *d = new QColorDialog(clr, this);
        d->setOption(QColorDialog::ShowAlphaChannel, true); 
        d->exec();
        if (QDialog::Accepted==d->result()) {
            clr=d->currentColor();
            watcher::Color c(clr.red(), clr.green(), clr.blue(), clr.alpha()); 
            QString ss;
            ss.sprintf("background-color: #%02x%02x%02x; color: #%02x%02x%02x", 
                    clr.red(), clr.green(), clr.blue(), 
                    (~clr.red())&0xFF, (~clr.green())&0xFF, (~clr.blue())&0xFF);  
            labelColorButton->setStyleSheet(ss); 
            if (useNodeId)
                graph->nodes[curNodeId].labelColor=c;
            else
                for (size_t n=0; n<graph->numValidNodes; n++) 
                    graph->nodes[n].labelColor=c;
        }
    }
    void NodeConfigurationDialog::setNodeLabelFont(void)
    {
        if (!graph)
            return;
        if (!graph->numValidNodes) {
            QMessageBox::information(this, tr("A Serious Lack of Nodes"), tr("No nodes to configure yet, sorry.")); 
            return; // we have at least one
        }
        QFont initial(graph->nodes[0].labelFont.c_str(), graph->nodes[0].labelPointSize); 
        bool ok;
        QFont font=QFontDialog::getFont(&ok, initial, this); 
        if (ok) {
            if (useNodeId) {
                graph->nodes[curNodeId].labelFont=font.family().toStdString(); 
                graph->nodes[curNodeId].labelPointSize=font.pointSize(); 
            }
            else
                for (size_t n=0; n<graph->numValidNodes; n++) {
                    graph->nodes[n].labelFont=font.family().toStdString(); 
                    graph->nodes[n].labelPointSize=font.pointSize(); 
                }
        }
    }
    void NodeConfigurationDialog::setNodeColor(void)
    {
        if (!graph)
            return;
        if (!graph->numValidNodes) {
            QMessageBox::information(this, tr("A Serious Lack of Nodes"), tr("No nodes to configure yet, sorry.")); 
            return; // we have at least one
        }
        QColor clr(
                graph->nodes[0].color.r, 
                graph->nodes[0].color.g, 
                graph->nodes[0].color.b, 
                graph->nodes[0].color.a);
        QColorDialog *d = new QColorDialog(clr, this);
        d->setOption(QColorDialog::ShowAlphaChannel, true); 
        d->exec();
        if (QDialog::Accepted==d->result()) {
            clr=d->currentColor();
            watcher::Color c(clr.red(), clr.green(), clr.blue(), clr.alpha()); 
            QString ss;
            ss.sprintf("background-color: #%02x%02x%02x; color: #%02x%02x%02x", 
                    clr.red(), clr.green(), clr.blue(), 
                    (~clr.red())&0xFF, (~clr.green())&0xFF, (~clr.blue())&0xFF);  
            LOG_DEBUG("setNodeColor: ss=" << ss.toStdString()); 
            nodeColorButton->setStyleSheet(ss); 
            if (useNodeId) 
                graph->nodes[curNodeId].color=c;
            else 
                for (size_t n=0; n<graph->numValidNodes; n++) 
                    graph->nodes[n].color=c;
        }
    }
    void NodeConfigurationDialog::setNodeLabel(QString str)
    {
        if (!graph)
            return;
        string s;
        if (str=="four octets")
            s=NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::FOUR_OCTETS); 
        else if (str=="three octets")
            s=NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::THREE_OCTETS);
        else if (str=="two octets")
            s=NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::TWO_OCTETS);
        else if (str=="last octet")
            s=NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::LAST_OCTET);
        else if(str=="hostname")
            s=NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::HOSTNAME);
        else {
            bool ok;
            QString text = QInputDialog::getText(this, tr("Node Label"), tr("Node Label:"), QLineEdit::Normal, QString(), &ok);
            if (ok && !text.isEmpty())
                s=text.toStdString(); 
        }
        if (useNodeId) 
            graph->nodes[curNodeId].rebuildLabel(s); 
        else 
            for (size_t n=0; n<graph->numValidNodes; n++) 
                graph->nodes[n].rebuildLabel(s); 
    }
    void NodeConfigurationDialog::setNodeShape(QString str)
    {
        if (!graph)
            return;
        NodePropertiesMessage::NodeShape shape;
        if (str=="Sphere (Circle)")
            shape=NodePropertiesMessage::CIRCLE;
        else if (str=="Cube (Square)")
            shape=NodePropertiesMessage::SQUARE;
        else if (str=="Torus (Circle)")
            shape=NodePropertiesMessage::TORUS;
        else if (str=="Teapot (Square)")
            shape=NodePropertiesMessage::TEAPOT;
        else if (str=="Pyramid (Triangle)")
            shape=NodePropertiesMessage::TRIANGLE;
        else
            shape=NodePropertiesMessage::CIRCLE;

        if (useNodeId) 
            graph->nodes[curNodeId].shape=shape;
        else 
            for (size_t n=0; n<graph->numValidNodes; n++) 
                graph->nodes[n].shape=shape;
    }
    void NodeConfigurationDialog::setNodeSize(int size)
    {
        if (!graph)
            return;
        if (useNodeId) 
            graph->nodes[curNodeId].size=size;
        else 
            for (size_t n=0; n<graph->numValidNodes; n++) 
                graph->nodes[n].size=size;
    }

    void NodeConfigurationDialog::configureDialog()
    {
        if (!graph || !graph->numValidNodes)
            return;
        configureDialog(0); 
    }
    void NodeConfigurationDialog::configureDialog(size_t nodeId)
    {
        if (!graph || !graph->numValidNodes)
            return;
        NodeDisplayInfo &node=graph->nodes[nodeId];
        QString ss;
        ss.sprintf("background-color: #%02x%02x%02x; color: #%02x%02x%02x", 
                node.color.r, node.color.g, node.color.b, 
                (~node.color.r)&0xFF, (~node.color.g)&0xFF, (~node.color.b)&0xFF);  
        nodeColorButton->setStyleSheet(ss); 
        ss.sprintf("background-color: #%02x%02x%02x; color: #%02x%02x%02x", 
                node.labelColor.r, node.labelColor.g, node.labelColor.b, 
                (~node.labelColor.r)&0xFF, (~node.labelColor.g)&0xFF, (~node.labelColor.b)&0xFF);  
        labelColorButton->setStyleSheet(ss); 

        nodeSizeSpinBox->setValue(node.size);

        // GTL - figure out how to set the combo boxes based on node shape/label.
    }
    void NodeConfigurationDialog::nodeClicked(size_t nodeId) 
    {
        if (!graph)
            return;
        if (useNodeId && curNodeId==nodeId) {
            useNodeId=false;
            curNodeId=0;
            labelWhichNode->setText("All Nodes"); 
        }
        else {
            useNodeId=true; 
            curNodeId=nodeId;
            configureDialog(nodeId); 
            NodeIdentifier nid(boost::asio::ip::address_v4(graph->index2Nid(static_cast<unsigned int>(nodeId)))); // oof.
            labelWhichNode->setText(nid.to_string().c_str()); 
        }
    }
}
