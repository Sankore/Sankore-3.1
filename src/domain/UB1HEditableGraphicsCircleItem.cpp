#include "UB1HEditableGraphicsCircleItem.h"

UB1HEditableGraphicsCircleItem::UB1HEditableGraphicsCircleItem(QGraphicsItem* parent):
    UB1HEditableGraphicsBasicShapeItem(parent)
{
    initializeStrokeProperty();
    initializeFillingProperty();

    mRadius = 0;
}

UB1HEditableGraphicsCircleItem::~UB1HEditableGraphicsCircleItem()
{

}

UBItem *UB1HEditableGraphicsCircleItem::deepCopy() const
{
    UB1HEditableGraphicsCircleItem* copy = new UB1HEditableGraphicsCircleItem();

    copyItemParameters(copy);

    return copy;
}

void UB1HEditableGraphicsCircleItem::copyItemParameters(UBItem *copy) const
{
    UB1HEditableGraphicsBasicShapeItem::copyItemParameters(copy);

    UB1HEditableGraphicsCircleItem *cp = dynamic_cast<UB1HEditableGraphicsCircleItem*>(copy);

    if(!cp) return;

    cp->mRadius = mRadius;
}

QPointF UB1HEditableGraphicsCircleItem::center() const
{
    QPointF centre;

    centre.setX(pos().x() + mRadius);
    centre.setY(pos().y() + mRadius);

    return centre;
}

void UB1HEditableGraphicsCircleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    UBAbstractGraphicsItem::setStyle(painter);

    painter->drawEllipse(QPointF(mRadius, mRadius), mRadius, mRadius);

    if(mMultiClickState >= 1){
        QPen p;
        p.setColor(QColor(128, 128, 200));
        p.setStyle(Qt::DotLine);
        p.setWidth(pen().width());

        painter->setPen(p);
        painter->setBrush(QBrush());

        painter->drawRect(0, 0, mRadius*2, mRadius*2);
    }
}

QRectF UB1HEditableGraphicsCircleItem::boundingRect() const
{
    QRectF rect(0, 0, mRadius*2, mRadius*2);

    rect = adjustBoundingRect(rect);

    if(mMultiClickState >= 1){
        qreal r = getHandle()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UB1HEditableGraphicsCircleItem::onActivateEditionMode()
{
    getHandle()->setPos(mRadius*2, mRadius*2);
}

void UB1HEditableGraphicsCircleItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    qreal maxSize = handle->radius() * 4;

    qreal r = qMin(handle->pos().x(), handle->pos().y()) / 2;

    if(r >= maxSize){
        mRadius = r;
    }

    getHandle()->setPos(mRadius*2, mRadius*2);

    if(hasGradient()){
        QLinearGradient g(QPointF(), QPointF(mRadius*2, 0));

        g.setColorAt(0, brush().gradient()->stops().at(0).second);
        g.setColorAt(1, brush().gradient()->stops().at(1).second);

        setBrush(g);
    }

    update();
}

QPainterPath UB1HEditableGraphicsCircleItem::shape() const
{
    QPainterPath path;
    if(mMultiClickState >= 1){
        path.addRect(boundingRect());
    }else{
        path.addEllipse(boundingRect());
    }

    return path;
}

void UB1HEditableGraphicsCircleItem::setRadius(qreal radius)
{
    prepareGeometryChange();
    mRadius = radius;
    update();
}

void UB1HEditableGraphicsCircleItem::setRect(QRectF rect)
{
    setPos(rect.topLeft());

    mRadius = rect.width()/2;
}

QRectF UB1HEditableGraphicsCircleItem::rect() const
{
    QRectF r;
    r.setTopLeft(pos());
    r.setWidth(mRadius*2);
    r.setHeight(mRadius*2);

    return r;
}

qreal UB1HEditableGraphicsCircleItem::radius() const
{
    return mRadius;
}
