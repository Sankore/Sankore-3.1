#include "UBStrokeProperty.h"

UBStrokeProperty::UBStrokeProperty():
    mColor(0, 0, 0, 255),
    mThickness(1),
    mPenStyle(Qt::SolidLine)
{

}

UBStrokeProperty::UBStrokeProperty(const UBStrokeProperty & source)
    : mColor(source.mColor)
    , mThickness(source.mThickness)
    , mPenStyle(source.mPenStyle)
{

}

void UBStrokeProperty::stroke(QPainter *painter)
{
    QPen pen = painter->pen();

    pen.setColor(mColor);
    pen.setWidth(mThickness);
    pen.setStyle(mPenStyle);

    painter->setPen(pen);

}
