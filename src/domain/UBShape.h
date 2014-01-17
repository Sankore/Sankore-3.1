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

class UBStrockeProperty : public QPen
{
public:
    UBStrockeProperty():
        QPen(Qt::SolidLine)
    {

    }

    UBStrockeProperty(const UBStrockeProperty& src):
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

    UBStrockeProperty* strokeProperty() const
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

    void initializeStrockeProperty()
    {
        mStrokeProperty = new UBStrockeProperty;
    }

    void initializeFillingProperty()
    {
        mFillingProperty = new UBFillProperty;
    }

    virtual ~UBShape();

    void applyStyle(Qt::PenStyle penStyle);
    void applyStyle(Qt::BrushStyle brushStyle);
    void applyStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle);

    void applyFillColor(const QColor& color);
    void applyStrockeColor(const QColor& color);

    void setStrockeSize(int size);

protected:
    UBFillProperty *mFillingProperty;
    UBStrockeProperty *mStrokeProperty;
};

#endif // UBSHAPE_H
