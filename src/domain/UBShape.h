#ifndef UBSHAPE_H
#define UBSHAPE_H

#include "UBItem.h"
#include "core/UB.h"

class UBFillProperty : public QBrush
{
public:
    UBFillProperty():
        QBrush(Qt::SolidPattern)
    {

    }

    UBFillProperty(const UBFillProperty& src):
        QBrush(src)
    {

    }
};

class UBStrokeProperty : public QPen
{
public:
    UBStrokeProperty():
        QPen(Qt::SolidLine)
    {

    }

    UBStrokeProperty(const UBStrokeProperty& src):
        QPen(src)
    {

    }
};

class UBShape : public UBItem, public UBGraphicsItem
{
public:
    UBShape();

    UBFillProperty* fillingProperty() const
    {
        return mFillingProperty;
    }

    UBStrokeProperty* strokeProperty() const
    {
        return mStrokeProperty;
    }

    bool hasFillingProperty() const
    {
        return mFillingProperty != NULL;
    }

    bool hasStrokeProperty() const
    {
        return mStrokeProperty != NULL;
    }

    void initializeStrokeProperty()
    {
        mStrokeProperty = new UBStrokeProperty();
    }

    void initializeFillingProperty()
    {
        mFillingProperty = new UBFillProperty();
    }

    virtual ~UBShape();

    void applyStyle(Qt::PenStyle penStyle);
    void applyStyle(Qt::BrushStyle brushStyle);
    void applyStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle);

    void applyFillColor(const QColor& color);
    void applyStrokeColor(const QColor& color);

    void setStrokeSize(int size);

protected:
    UBFillProperty *mFillingProperty;
    UBStrokeProperty *mStrokeProperty;

    virtual QRectF boundingRect() const;
};

#endif // UBSHAPE_H
