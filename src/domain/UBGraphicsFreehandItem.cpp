#include "UBGraphicsFreehandItem.h"

UBGraphicsFreehandItem::UBGraphicsFreehandItem(QGraphicsItem *parent) :
    UBAbstractGraphicsPathItem(parent)
{
    initializeStrokeProperty();
}

void UBGraphicsFreehandItem::addPoint(const QPointF & point)
{
    QPainterPath painterPath = path();

    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(point); // For the first point added, we must use moveTo().
    }
    else
    {
        painterPath.lineTo(point);
    }

    setPath(painterPath);
}

UBItem *UBGraphicsFreehandItem::deepCopy() const
{
    UBGraphicsFreehandItem * copy = new UBGraphicsFreehandItem();

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsFreehandItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsFreehandItem *cp = dynamic_cast<UBGraphicsFreehandItem*>(copy);
    if (cp)
    {
        cp->setPath(QPainterPath(this->path()));
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

        if (hasStrokeProperty())
            cp->mStrokeProperty = new UBStrokeProperty(*strokeProperty());
    }
}

void UBGraphicsFreehandItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    setPen(*strokeProperty());

    QGraphicsPathItem::paint(painter, &styleOption, widget);
}
