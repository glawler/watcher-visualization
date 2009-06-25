#include "speedlabel.h"

void SpeedLabel::setNum(int x)
{
    switch(x)
    {
        case 0: setText("1/4x"); break;
        case 1: setText("1/2x"); break;
        case 2: setText("1x"); break;
        case 3: setText("2x"); break;
        case 4: setText("4x"); break;
        case 5: setText("8x"); break;
        case 6: setText("16x"); break;
        case 7: setText("32x"); break;
        default: setText("err"); break;
    }
}
