#include "UBShape.h"

UBShape::UBShape():
    mFillingProperty(NULL),
    mStrokeProperty(NULL)
{

}

UBShape::~UBShape()
{
    if(mFillingProperty){
        delete mFillingProperty;
        mFillingProperty = NULL;
    }

    if(mStrokeProperty){
        delete mStrokeProperty;
        mStrokeProperty = NULL;
    }
}

void UBShape::applyStyle(Qt::PenStyle penStyle)
{
    Qt::BrushStyle brushStyle = this->fillingProperty()->style();

    applyStyle(brushStyle, penStyle);
}

void UBShape::applyStyle(Qt::BrushStyle brushStyle)
{
    Qt::PenStyle penStyle = this->strokeProperty()->style();

    applyStyle(brushStyle, penStyle);
}

void UBShape::applyStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle)
{
    if(hasFillingProperty())
        this->fillingProperty()->setStyle(brushStyle);

    if(hasStrokeProperty())
        this->strokeProperty()->setStyle(penStyle);
}

void UBShape::applyFillColor(const QColor& color)
{
    if(hasFillingProperty())
        fillingProperty()->setColor(color);
}

void UBShape::applyStrokeColor(const QColor& color)
{
    if(hasStrokeProperty())
        strokeProperty()->setColor(color);
}

void UBShape::setStrokeSize(int size)
{
    if(hasStrokeProperty())
        strokeProperty()->setWidth(size);
}

QRectF UBShape::boundingRect() const
{
    return QRectF();
}
