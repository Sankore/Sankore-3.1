#include "UB1HandleEditable.h"

UB1HandleEditable::UB1HandleEditable():
    UBEditable()
{
    UBDiagonalHandle *handle = new UBDiagonalHandle;

    handle->setEditableObject(this);

    handle->hide();

    mHandles.push_back(handle);
}
