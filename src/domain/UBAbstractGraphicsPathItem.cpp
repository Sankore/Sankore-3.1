#include "UBAbstractGraphicsPathItem.h"
#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"

UBAbstractGraphicsPathItem::UBAbstractGraphicsPathItem(QGraphicsItem *parent):
    QGraphicsPathItem(parent)
{
    setDelegate(new UBGraphicsItemDelegate(this, 0));
    Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(true);
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

void UBAbstractGraphicsPathItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

QVariant UBAbstractGraphicsPathItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsPathItem::itemChange(change, newValue);
}


void UBAbstractGraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->mousePressEvent(event);
}

void UBAbstractGraphicsPathItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Delegate()->mouseReleaseEvent(event);
    QGraphicsPathItem::mouseReleaseEvent(event);
}
