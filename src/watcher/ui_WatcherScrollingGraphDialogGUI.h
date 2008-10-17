/********************************************************************************
** Form generated from reading ui file 'WatcherScrollingGraphDialogGUI.ui'
**
** Created: Fri Oct 17 15:19:42 2008
**      by: Qt User Interface Compiler version 4.3.5
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_WATCHERSCROLLINGGRAPHDIALOGGUI_H
#define UI_WATCHERSCROLLINGGRAPHDIALOGGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QSpacerItem>
#include "qwt_plot.h"

class Ui_WatcherScrollingGraphDialogGUI
{
public:
    QGridLayout *gridLayout;
    QwtPlot *qwtPlot;
    QSpacerItem *spacerItem;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *WatcherScrollingGraphDialogGUI)
    {
    if (WatcherScrollingGraphDialogGUI->objectName().isEmpty())
        WatcherScrollingGraphDialogGUI->setObjectName(QString::fromUtf8("WatcherScrollingGraphDialogGUI"));
    WatcherScrollingGraphDialogGUI->resize(474, 353);
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(WatcherScrollingGraphDialogGUI->sizePolicy().hasHeightForWidth());
    WatcherScrollingGraphDialogGUI->setSizePolicy(sizePolicy);
    WatcherScrollingGraphDialogGUI->setAutoFillBackground(false);
    gridLayout = new QGridLayout(WatcherScrollingGraphDialogGUI);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    qwtPlot = new QwtPlot(WatcherScrollingGraphDialogGUI);
    qwtPlot->setObjectName(QString::fromUtf8("qwtPlot"));
    qwtPlot->setProperty("thisIsABool", QVariant(false));

    gridLayout->addWidget(qwtPlot, 0, 0, 1, 2);

    spacerItem = new QSpacerItem(157, 29, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacerItem, 1, 0, 1, 1);

    buttonBox = new QDialogButtonBox(WatcherScrollingGraphDialogGUI);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
    buttonBox->setSizePolicy(sizePolicy1);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::NoButton|QDialogButtonBox::Save);

    gridLayout->addWidget(buttonBox, 1, 1, 1, 1);


    retranslateUi(WatcherScrollingGraphDialogGUI);
    QObject::connect(buttonBox, SIGNAL(accepted()), WatcherScrollingGraphDialogGUI, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), WatcherScrollingGraphDialogGUI, SLOT(reject()));

    QMetaObject::connectSlotsByName(WatcherScrollingGraphDialogGUI);
    } // setupUi

    void retranslateUi(QDialog *WatcherScrollingGraphDialogGUI)
    {
    WatcherScrollingGraphDialogGUI->setWindowTitle(QApplication::translate("WatcherScrollingGraphDialogGUI", "Watcher Scrolling Graph", 0, QApplication::UnicodeUTF8));
    qwtPlot->setProperty("propertiesDocument", QVariant(QString()));
    Q_UNUSED(WatcherScrollingGraphDialogGUI);
    } // retranslateUi

};

namespace Ui {
    class WatcherScrollingGraphDialogGUI: public Ui_WatcherScrollingGraphDialogGUI {};
} // namespace Ui

#endif // UI_WATCHERSCROLLINGGRAPHDIALOGGUI_H
