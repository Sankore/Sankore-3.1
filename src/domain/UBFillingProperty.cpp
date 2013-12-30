#include "UBFillingProperty.h"

UBFillingProperty::UBFillingProperty():
    mFirstColor(0, 0, 0, 0),
    mSecondColor(0, 0, 0, 0),
    mBrushStyle(Qt::SolidPattern)
{

}

void UBFillingProperty::fill(QPainter *painter)
{
    QBrush brush = painter->brush();
    brush.setStyle(mBrushStyle);

    switch (mBrushStyle)
    {
    case Qt::SolidPattern:
    {
        brush.setColor(mFirstColor);
        break;
    }
    default:
        break;
    }

    painter->setBrush(brush);
}
