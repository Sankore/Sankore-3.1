#ifndef UBSHAPE_H
#define UBSHAPE_H

#include "UBItem.h"
#include "core/UB.h"
#include "UBFillingProperty.h"
#include "UBStrokeProperty.h"


class UBShape : public UBItem, public UBGraphicsItem
{
public:
    UBShape();

    UBFillingProperty* fillingProperty() const
    {
        return mFillingProperty;
    }

    UBStrokeProperty* strokeProperty() const
    {
        return mStrokeProperty;
    }

    bool hasFillingProperty() const
    {
        return mFillingProperty != 0;
    }

    bool hasStrokeProperty() const
    {
        return mStrokeProperty != NULL;
    }

protected:
    UBFillingProperty *mFillingProperty;
    UBStrokeProperty *mStrokeProperty;
};

#endif // UBSHAPE_H
