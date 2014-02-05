#include "UBGraphicsPathItem.h"

#include <customWidgets/UBGraphicsItemAction.h>
#include "UBFreeHandle.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "UBGraphicsScene.h"

UBGraphicsPathItem::UBGraphicsPathItem(QGraphicsItem* parent)
    : UBAbstractGraphicsPathItem(parent)
    , mClosed(false)
    , mMultiClickState(0)
    , HANDLE_SIZE(20)
{
    initializeStrokeProperty();
    initializeFillingProperty();
}

UBGraphicsPathItem::~UBGraphicsPathItem()
{

}

void UBGraphicsPathItem::addPoint(const QPointF & point)
{
    prepareGeometryChange();

    QPainterPath painterPath = path();

    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(point); // For the first point added, we must use moveTo().
        setPath(painterPath);

        mStartEndPoint[0] = point;
    }
    else
    {
        // If clic on first point, close the polygon
        // TODO à terme : utiliser la surface de la première poignée.
        QPointF pointDepart(painterPath.elementAt(0).x, painterPath.elementAt(0).y);
        QGraphicsEllipseItem poigneeDepart(pointDepart.x()-10, pointDepart.y()-10, 20, 20);

        if (poigneeDepart.contains(point))
        {
            setClosed(true);

        }
        else
        {
            painterPath.lineTo(point);
            setPath(painterPath);
        }

        mStartEndPoint[1] = point;
    }

    if(!mClosed){
        UBFreeHandle *handle = new UBFreeHandle;

        addHandle(handle);

        handle->setParentItem(this);
        handle->setEditableObject(this);
        handle->setPos(point);
        handle->setId(path().elementCount()-1);
        handle->hide();
    }
}

void UBGraphicsPathItem::setClosed(bool closed)
{
    mClosed = closed;

    QPainterPath painterPath = path();
    if (closed)
    {
        painterPath.closeSubpath(); // Automatically add a last point, identic to the first point.
    }
    else
    {
        // if last point and first point are the same, remove the last one, in order to open the path.
        int nbElements = painterPath.elementCount();
        if ( nbElements > 1)
        {
            QPainterPath::Element firstElement = painterPath.elementAt(0);
            QPainterPath::Element lastElement = painterPath.elementAt(nbElements - 1);

            QPointF firstPoint(firstElement.x, firstElement.y);
            QPointF lastPoint(lastElement.x, lastElement.y);

            if (firstPoint == lastPoint)
            {
                // Rebuild the path, excluding the last point.
                QPainterPath newPainterPath(firstPoint);
                for(int iElement=1; iElement<nbElements - 1; iElement++)
                {
                    newPainterPath.lineTo(painterPath.elementAt(iElement));
                }
                painterPath = newPainterPath;
            }
        }
    }

    setPath(painterPath);
}


UBItem *UBGraphicsPathItem::deepCopy() const
{
    UBGraphicsPathItem * copy = new UBGraphicsPathItem();

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsPathItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsPathItem *cp = dynamic_cast<UBGraphicsPathItem*>(copy);
    if (cp)
    {
        cp->setPath(QPainterPath(this->path()));
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setFlag(QGraphicsItem::ItemIsFocusable, true);

        if(Delegate()->action()){
            if(Delegate()->action()->linkType() == eLinkToAudio){
                UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
                UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
                cp->Delegate()->setAction(action);
            }
            else
                cp->Delegate()->setAction(Delegate()->action());
        }

        if (hasFillingProperty())
            cp->mFillingProperty = new UBFillProperty(*fillingProperty());

        if (hasStrokeProperty())
            cp->mStrokeProperty = new UBStrokeProperty(*strokeProperty());

        cp->mClosed = this->mClosed;

        for(int i = 0; i < mHandles.size(); i++){
            UBFreeHandle *handle = new UBFreeHandle(dynamic_cast<UBFreeHandle*>(mHandles.at(i)));
            cp->mHandles.push_back(handle);
            UBApplication::boardController->controlView()->scene()->addItem(handle);
            handle->setParentItem(cp);
            handle->setEditableObject(cp);
        }
    }
}

QRectF UBGraphicsPathItem::boundingRect() const
{
    QRectF rect = path().boundingRect();

    int enlarge = 0;

    if (strokeProperty())
    {
        int thickness = strokeProperty()->width();
        enlarge = thickness/2;
    }

    rect.adjust(-enlarge, -enlarge, enlarge, enlarge);

    if(mMultiClickState >= 2){
        qreal r = mHandles.first()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UBGraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    if(this->isClosed())
        painter->setBrush(*fillingProperty());

    painter->setPen(*strokeProperty());

    if (isClosed())
    {
        painter->fillPath(path(), painter->brush());
    }

    //QGraphicsPathItem::paint(painter, &styleOption, widget);
    painter->drawPath(path());

    if (!isClosed())
    {
        QPen penHandles;
        penHandles.setWidth(1);
        penHandles.setColor(Qt::black);
        penHandles.setStyle(Qt::SolidLine);
        painter->setPen(penHandles);

        int hsize = HANDLE_SIZE/2;

        painter->drawEllipse(mStartEndPoint[0].x() - hsize, mStartEndPoint[0].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);

        if(path().elementCount() >= 2)
            painter->drawEllipse(mStartEndPoint[1].x() - hsize, mStartEndPoint[1].y() - hsize, HANDLE_SIZE, HANDLE_SIZE);
    }
}

void UBGraphicsPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;

    UBAbstractGraphicsPathItem::mousePressEvent(event);

    if(mMultiClickState > 2){
        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

void UBGraphicsPathItem::updateHandle(UBAbstractHandle *handle)
{
    int id = handle->getId();

    QPainterPath oldPath = path();

    QPainterPath newPath;

    if(mClosed && id == 0){
        newPath.moveTo(handle->pos());
        for(int i = 1; i < oldPath.elementCount()-1; i++){
            newPath.lineTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
        }
        newPath.lineTo(handle->pos());
    }else{
        for(int i = 0; i < oldPath.elementCount(); i++){
            if(i == 0){
                if(i == id){
                    newPath.moveTo(handle->pos());
                }else{
                    newPath.moveTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
                }
            }else{
                if(i == id){
                    newPath.lineTo(handle->pos());
                }else{
                    newPath.lineTo(oldPath.elementAt(i).x, oldPath.elementAt(i).y);
                }
            }
        }
    }


    setPath(newPath);
}

QPainterPath UBGraphicsPathItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void UBGraphicsPathItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState > 1){
        mMultiClickState = 0;
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        showEditMode(false);
    }
}
