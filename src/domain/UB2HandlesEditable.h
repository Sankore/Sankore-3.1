#ifndef UB2HANDLESEDITABLE_H
#define UB2HANDLESEDITABLE_H

#include "UBEditable.h"

class UB2HandlesEditable : public UBAbstractEditable
{
public:
    UB2HandlesEditable();

    UBAbstractHandle *horizontalHandle()
    {
        return mHandles.at(0);
    }

    UBAbstractHandle *verticalHandle()
    {
        return mHandles.at(1);
    }
};

#endif // UB2HANDLESEDITABLE_H
