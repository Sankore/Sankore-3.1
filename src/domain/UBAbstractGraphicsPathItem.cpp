#include "UBAbstractGraphicsPathItem.h"

UBAbstractGraphicsPathItem::UBAbstractGraphicsPathItem(QGraphicsItem *parent):
    UBAbstractGraphicsItem(parent)
{

}

QRectF UBAbstractGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    rect = adjustBoundingRect(rect);

    return rect;
}

QPainterPath UBAbstractGraphicsPathItem::shape() const
{
    return path();
}

void UBAbstractGraphicsPathItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractGraphicsItem::copyItemParameters(copy);

    UBAbstractGraphicsPathItem* cp = dynamic_cast<UBAbstractGraphicsPathItem*>(copy);

    if(!cp) return;

    cp->setPath(path());
}
