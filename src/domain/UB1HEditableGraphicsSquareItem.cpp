#include "UB1HEditableGraphicsSquareItem.h"

#include "customWidgets/UBGraphicsItemAction.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

UB1HEditableGraphicsSquareItem::UB1HEditableGraphicsSquareItem(QGraphicsItem* parent)
    : UB1HEditableGraphicsBasicShapeItem(parent)
{
    // Rect has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    mSide = 0;
}

UB1HEditableGraphicsSquareItem::~UB1HEditableGraphicsSquareItem()
{

}

UBItem *UB1HEditableGraphicsSquareItem::deepCopy() const
{
    UB1HEditableGraphicsSquareItem* copy = new UB1HEditableGraphicsSquareItem();

    copyItemParameters(copy);

    return copy;
}

void UB1HEditableGraphicsSquareItem::copyItemParameters(UBItem *copy) const
{
    UB1HEditableGraphicsBasicShapeItem::copyItemParameters(copy);

    UB1HEditableGraphicsSquareItem *cp = dynamic_cast<UB1HEditableGraphicsSquareItem*>(copy);

    if(!cp) return;

    cp->mSide = mSide;
}

void UB1HEditableGraphicsSquareItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->drawRect(0, 0, mSide, mSide);
}

QPainterPath UB1HEditableGraphicsSquareItem::shape() const
{
    QPainterPath path;

    if(mMultiClickState >= 1){
        path.addRect(boundingRect());
    }else{
        path.addRect(0, 0, mSide, mSide);
    }

    return path;
}

void UB1HEditableGraphicsSquareItem::onActivateEditionMode()
{
    getHandle()->setPos(mSide, mSide);
}

void UB1HEditableGraphicsSquareItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    qreal side = qMin(handle->pos().x(), handle->pos().y());

    if(side > maxSize){
        mSide = side;
    }

    getHandle()->setPos(mSide, mSide);

    if(hasGradient()){
        QLinearGradient g(QPointF(), QPointF(mSide, 0));

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);
    }

    update();
}

QRectF UB1HEditableGraphicsSquareItem::boundingRect() const
{
    QRectF rect(0, 0, mSide, mSide);

    adjustBoundingRect(rect);

    if(mMultiClickState >= 1){
        qreal r = mHandles.at(0)->radius();
        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UB1HEditableGraphicsSquareItem::setRect(QRectF rect)
{
    prepareGeometryChange();

    setPos(rect.topLeft());

    mSide = qMin(rect.width(), rect.height());

    update();
}

QRectF UB1HEditableGraphicsSquareItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());
    r.setWidth(mSide);
    r.setHeight(mSide);

    return r;
}
