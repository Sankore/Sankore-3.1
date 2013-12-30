#ifndef UBSHAPE_H
#define UBSHAPE_H

#include "UBItem.h"
#include "core/UB.h"

class UBFillingProperty;
class UBStrokeProperty;

class UBShape : public UBItem, public UBGraphicsItem
{
public:
    UBShape();

    UBFillingProperty* fillingProperty() const
    {
        return mFillingProperty;
    }

    UBStrokeProperty* strockeProperty() const
    {
        return mStrockeProperty;
    }

    bool hasFillingProperty() const
    {
        return mFillingProperty != NULL;
    }

    bool hasStrockeProperty() const
    {
        return mStrockeProperty != NULL;
    }

private:
    UBFillingProperty *mFillingProperty;
    UBStrokeProperty *mStrockeProperty;
};

#endif // UBSHAPE_H
