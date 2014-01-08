#include "UBAbstractSubPalette.h"

UBAbstractSubPalette::UBAbstractSubPalette(QWidget *parent, Qt::Orientation orient) :
    UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mMainAction(0)
{

}

UBAbstractSubPalette::UBAbstractSubPalette(Qt::Orientation orient, QWidget *parent) :
    UBActionPalette(orient, parent)
    , mMainAction(0)
{

}

void UBAbstractSubPalette::togglePalette()
{
    show();
}
