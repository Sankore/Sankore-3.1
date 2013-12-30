#ifndef UBSTROCKEPROPERTY_H
#define UBSTROCKEPROPERTY_H

#include <QColor>
#include <QPainter>

class UBStrockeProperty
{
public:
    UBStrockeProperty();

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

#endif // UBSTROCKEPROPERTY_H
