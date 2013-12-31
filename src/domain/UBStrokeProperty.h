#ifndef UBSTROKEPROPERTY_H
#define UBSTROKEPROPERTY_H

#include <QColor>
#include <QPainter>

class UBStrokeProperty
{
public:
    UBStrokeProperty();
    UBStrokeProperty(const UBStrokeProperty & source);

    unsigned int thickness() const
    {
        return mThickness;
    }

    void setThickness(unsigned int thickness)
    {
        mThickness = thickness;
    }

    QColor color() const
    {
        return mColor;
    }

    void setColor(const QColor& color)
    {
        mColor = color;
    }

    Qt::PenStyle penStyle() const
    {
        return mPenStyle;
    }

    void setPenStyle(Qt::PenStyle penStyle)
    {
        mPenStyle = penStyle;
    }

    void stroke(QPainter *painter);

private:
    QColor mColor;
    unsigned int mThickness;
    Qt::PenStyle mPenStyle;
};

#endif // UBSTROKEPROPERTY_H
