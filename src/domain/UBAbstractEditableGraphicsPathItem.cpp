#include "UBAbstractEditableGraphicsPathItem.h"

#include "UBFreeHandle.h"

UBAbstractEditableGraphicsPathItem::UBAbstractEditableGraphicsPathItem(QGraphicsItem *parent):
    UBAbstractGraphicsPathItem(parent)
{
    mMultiClickState = 0;
}

void UBAbstractEditableGraphicsPathItem::onActivateEditionMode()
{
    //NOOP
}

void UBAbstractEditableGraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;

    UBAbstractGraphicsPathItem::mousePressEvent(event);

    if(mMultiClickState %2 == 1){
        onActivateEditionMode();

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
    else
    {
        Delegate()->showFrame(true);
        showEditMode(false);
    }
}

QRectF UBAbstractEditableGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    rect = UBAbstractGraphicsPathItem::adjustBoundingRect(rect);

    if(mMultiClickState %2 == 1){
        qreal r = mHandles.first()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UBAbstractEditableGraphicsPathItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mMultiClickState == 0){
        Delegate()->mouseMoveEvent(event);
        UBAbstractGraphicsItem::mouseMoveEvent(event);
    }
}

void UBAbstractEditableGraphicsPathItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    mMultiClickState = 0;
    showEditMode(false);
}

void UBAbstractEditableGraphicsPathItem::focusHandle(UBAbstractHandle *handle)
{
    Q_UNUSED(handle)

    setSelected(true);
    Delegate()->showFrame(false);
}

void UBAbstractEditableGraphicsPathItem::deactivateEditionMode()
{
    mMultiClickState = 0;
    showEditMode(false);
}

QPainterPath UBAbstractEditableGraphicsPathItem::shape() const
{
    QPainterPath path;
    if(mMultiClickState %2 == 1){
        path.addRect(boundingRect());
        return path;
    }else{
        return this->path();
    }
}
