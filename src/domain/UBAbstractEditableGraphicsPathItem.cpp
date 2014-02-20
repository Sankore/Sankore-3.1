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

    if(mMultiClickState >= 1){
        onActivateEditionMode();

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

QRectF UBAbstractEditableGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    if(mMultiClickState >= 1){
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

    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}

void UBAbstractEditableGraphicsPathItem::focusHandle(UBAbstractHandle *handle)
{
    Q_UNUSED(handle)

    setSelected(true);
    Delegate()->showFrame(false);
}

void UBAbstractEditableGraphicsPathItem::deactivateEditionMode()
{
    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}

QPainterPath UBAbstractEditableGraphicsPathItem::shape() const
{
    QPainterPath path;
    if(mMultiClickState >= 1){
        path.addRect(boundingRect());
        return path;
    }else{
        return this->path();
    }
}
