#include "UBShapeEditable.h"

UBAbstractEditableGraphicsShapeItem::UBAbstractEditableGraphicsShapeItem(QGraphicsItem *parent):
    UBAbstractGraphicsItem(parent)
{
    mMultiClickState = 0;
}

void UBAbstractEditableGraphicsShapeItem::onActivateEditionMode()
{
    //NOOP
}

void UBAbstractEditableGraphicsShapeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;

    UBAbstractGraphicsItem::mousePressEvent(event);

    if(mMultiClickState >= 1){
        prepareGeometryChange();

        onActivateEditionMode();

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

void UBAbstractEditableGraphicsShapeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mMultiClickState == 0){
        Delegate()->mouseMoveEvent(event);
        UBAbstractGraphicsItem::mouseMoveEvent(event);
    }
}

void UBAbstractEditableGraphicsShapeItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState >= 1){
        prepareGeometryChange();

        mMultiClickState = 0;
        showEditMode(false);
    }
}
