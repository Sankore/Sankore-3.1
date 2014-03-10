#include "UBGraphicsFreehandItem.h"

#include <customWidgets/UBGraphicsItemAction.h>

UBGraphicsFreehandItem::UBGraphicsFreehandItem(QGraphicsItem *parent) :
    UBAbstractGraphicsPathItem(parent)
    , HANDLE_SIZE(20)
    , mIsInCreationMode(true)
{
    initializeStrokeProperty();
}

void UBGraphicsFreehandItem::setClosed(bool closed)
{
    if (closed)
        initializeFillingProperty();
}

void UBGraphicsFreehandItem::addPoint(const QPointF & point)
{
    prepareGeometryChange();

    QPainterPath painterPath = path();
    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(point); // For the first point added, we must use moveTo().                
        mStartEndPoint[0] = point;
    }
    else
    {
        painterPath.lineTo(point);
        mStartEndPoint[1] = point;
    }

    setPath(painterPath);

    update();
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

        if(Delegate()->action()){
            if(Delegate()->action()->linkType() == eLinkToAudio){
                UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
                UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
                cp->Delegate()->setAction(action);
            }
            else
                cp->Delegate()->setAction(Delegate()->action());
        }
    }
}

void UBGraphicsFreehandItem::setIsInCreationMode(bool mode)
{
     mIsInCreationMode = mode;
}

QRectF UBGraphicsFreehandItem::boundingRect() const
{
    QRectF rect = UBAbstractGraphicsPathItem::boundingRect();

    int enlarge = 0;

    if (mIsInCreationMode)//gérer les poignées aux extrémités
        enlarge += HANDLE_SIZE/2;

    rect.adjust(-enlarge, -enlarge, enlarge, enlarge);

    return rect;
}

void UBGraphicsFreehandItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    UBAbstractGraphicsItem::setStyle(painter);       

    painter->drawPath(path());

    if (mIsInCreationMode)
    {
        int hsize = HANDLE_SIZE/2;
        painter->drawEllipse(mStartEndPoint[0].x() - hsize, mStartEndPoint[0].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);

        if(path().elementCount() >= 2)
            painter->drawEllipse(mStartEndPoint[1].x() - hsize, mStartEndPoint[1].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);
    }
}
