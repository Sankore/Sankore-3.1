#include "UB3HandlesEditable.h"

#include "UBHorizontalHandle.h"
#include "UBVerticalHandle.h"
#include "UBDiagonalHandle.h"

UB3HandlesEditable::UB3HandlesEditable()
{
    UBHorizontalHandle *horizontalHandle = new UBHorizontalHandle;
    UBVerticalHandle *verticalHandle = new UBVerticalHandle;
    UBDiagonalHandle *diagonalHandle = new UBDiagonalHandle;

    //must be insert according the order defines in th enumeration
    horizontalHandle->setId(0);
    verticalHandle->setId(1);
    diagonalHandle->setId(2);

    horizontalHandle->setEditableObject(this);
    verticalHandle->setEditableObject(this);
    diagonalHandle->setEditableObject(this);

    horizontalHandle->hide();
    verticalHandle->hide();
    diagonalHandle->hide();

    mHandles.push_back(horizontalHandle);
    mHandles.push_back(verticalHandle);
    mHandles.push_back(diagonalHandle);
}
