#ifndef UB1HANDLEEDITABLE_H
#define UB1HANDLEEDITABLE_H

#include "UBEditable.h"
#include "UBDiagonalHandle.h"

class UB1HandleEditable : public UBEditable
{
public:
    UB1HandleEditable();

    UBAbstractHandle *getHandle() const
    {
        return mHandles.first();
    }
};

#endif // UB1HANDLEEDITABLE_H
