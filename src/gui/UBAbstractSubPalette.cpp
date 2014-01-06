#include "UBAbstractSubPalette.h"

UBAbstractSubPalette::UBAbstractSubPalette(QWidget *parent, Qt::Orientation orient) :
    UBActionPalette(Qt::TopLeftCorner, parent, orient)
{

}

UBAbstractSubPalette::UBAbstractSubPalette(Qt::Orientation orient, QWidget *parent) :
    UBActionPalette(orient, parent)
{

}

void UBAbstractSubPalette::togglePalette()
{
    show();
}
