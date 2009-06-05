#ifndef SPEED_LABEL_H
#define SPEED_LABEL_H

#include <qlabel.h>

class SpeedLabel : public QLabel
{
    Q_OBJECT

    public:

        SpeedLabel(QWidget *parent=0, Qt::WindowFlags f=0) : QLabel(parent, f) {} 

        public slots:

            void setNum(int x); 
};

#endif // SPEED_LABEL_H
