#ifndef UBFILLINGPROPERTY_H
#define UBFILLINGPROPERTY_H

#include <QColor>
#include <QPainter>

class UBFillingProperty
{
public:
    UBFillingProperty();

    QColor firstColor() const
    {
        return mFirstColor;
    }

    void setFirstColor(const QColor& color)
    {
        mFirstColor = color;
    }

    QColor secondColor() const
    {
        return mSecondColor;
    }

    void setSecondColor(const QColor& color)
    {
        mSecondColor = color;
    }

    Qt::BrushStyle brushStyle() const
    {
        return mBrushStyle;
    }

    void setBrushStyle(Qt::BrushStyle brushStyle)
    {
        mBrushStyle = brushStyle;
    }

    void fill(QPainter *painter);

private:
    QColor mFirstColor;
    QColor mSecondColor;
    Qt::BrushStyle mBrushStyle;
};

#endif // UBFILLINGPROPERTY_H
