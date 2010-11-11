#ifndef QOGRE_WATCHER_WIDGET_H
#define QOGRE_WATCHER_WIDGET_H
#include "QOgreWidget.h"

namespace watcher 
{
    using namespace QtOgre; 

    class QOgreWatcherWidget : public QOgreWidget {
        Q_OBJECT
        public:
            QOgreWatcherWidget();
            virtual ~QOgreWatcherWidget();

            public slots:
            signals:

        protected:
            void timerEvent(QTimerEvent *e);
            void mousePressEvent(QMouseEvent *e);
            void mouseReleaseEvent(QMouseEvent *e);
            void mouseMoveEvent(QMouseEvent *e);
            void createScene(void);

        private:
    };
} // namespace
#endif
