#include "watcherGraphDialog.h"

using namespace watcher;

INIT_LOGGER(QWatcherGraphDialog, "QWatcherGraphDialog"); 

QWatcherGraphDialog::QWatcherGraphDialog(const QString &label_, QWidget * parent, Qt::WindowFlags f) : 
    QDialog(parent, f),
    label(label_)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void QWatcherGraphDialog::done(int)
{
    TRACE_ENTER();
    // Just hide it. We still want it around so the embedded GraphPlot can keep track of incoming data.  
    hide(); 
    emit dialogVisible(label, false); 
    TRACE_EXIT();
}

bool QWatcherGraphDialog::createDialog(GraphPlot *thePlot)
{
    TRACE_ENTER();
    resize(474, 353);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(thePlot);
    TRACE_EXIT_RET(true);
    return true;
}

