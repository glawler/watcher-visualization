/**
 * @file stringBasedMenuItem.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 */
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

       signals:
            void showMenuItem(QString str, bool show); 

        private:
            const QString str;
    };
}

