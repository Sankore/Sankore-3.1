#include "UB2HandlesEditable.h"

#include "UBHorizontalHandle.h"
#include "UBVerticalHandle.h"

UB2HandlesEditable::UB2HandlesEditable():
    UBAbstractEditable()
{
    UBHorizontalHandle *horizontalHandle = new UBHorizontalHandle;
    UBVerticalHandle *verticalHandle = new UBVerticalHandle;

    horizontalHandle->setId(0);
    verticalHandle->setId(1);

    horizontalHandle->setEditableObject(this);
    verticalHandle->setEditableObject(this);

    horizontalHandle->hide();
    verticalHandle->hide();

    mHandles.push_back(horizontalHandle);
    mHandles.push_back(verticalHandle);
}
