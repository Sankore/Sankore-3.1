#include "UBGraphicsPathItem.h"
#include "UBGraphicsDelegateFrame.h"

UBGraphicsPathItem::UBGraphicsPathItem(QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
    , mClosed(false)
{
    initialize();
}

void UBGraphicsPathItem::initialize()
{
    mFillingProperty = new UBFillingProperty();
    mStrokeProperty = new UBStrokeProperty();

    setDelegate(new UBGraphicsItemDelegate(this, 0));
    Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(false);
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}


UBGraphicsPathItem::~UBGraphicsPathItem()
{

}

void UBGraphicsPathItem::addPoint(const QPointF & point)
{
    QPainterPath painterPath = path();

    if (painterPath.elementCount() == 0)
    {
        painterPath.moveTo(point); // For the first point added, we must use moveTo().
        setPath(painterPath);
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

        /* Pas d'Action possible sur une Shape
        if(Delegate()->action()){
            if(Delegate()->action()->linkType() == eLinkToAudio){
                UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
                UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
                cp->Delegate()->setAction(action);
            }
            else
                cp->Delegate()->setAction(Delegate()->action());
        }
        */

        if (hasFillingProperty())
            cp->mFillingProperty = new UBFillingProperty(*fillingProperty());

        if (hasStrokeProperty())
            cp->mStrokeProperty = new UBStrokeProperty(*strokeProperty());

        cp->mClosed = this->mClosed;
    }
}

void UBGraphicsPathItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

QRectF UBGraphicsPathItem::boundingRect() const
{
    return path().boundingRect();
}

void UBGraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    fillingProperty()->fill(painter);
    strokeProperty()->stroke(painter);

    if (isClosed())
    {
        painter->fillPath(path(), painter->brush());
    }

    QGraphicsPathItem::paint(painter, &styleOption, widget);

    // Dessiner les sommets (et extremités)
    //if (isSelected())
    {
        for(int iElement = 0; iElement < path().elementCount(); ++iElement)
        {
            QPainterPath::Element element = path().elementAt(iElement);
            QPointF point(element.x, element.y);
            painter->drawEllipse(point, 10, 10);
        }
    }
}

QVariant UBGraphicsPathItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsPathItem::itemChange(change, newValue);
}
