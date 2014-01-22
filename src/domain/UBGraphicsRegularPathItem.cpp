#include "UBGraphicsRegularPathItem.h"
#include "UBGraphicsDelegateFrame.h"

#include <cmath>

UBGraphicsRegularPathItem::UBGraphicsRegularPathItem(int nVertices, QPointF startPos, QGraphicsItem * parent)
    : UBAbstractGraphicsPathItem(parent)
    , mNVertices(nVertices)
    , mStartPoint(startPos)
    , HANDLE_SIZE(20)
{
    initializeStrokeProperty();
    initializeFillingProperty();
    createGraphicsRegularPathItem();
}

UBGraphicsRegularPathItem::~UBGraphicsRegularPathItem()
{

}

void UBGraphicsRegularPathItem::createGraphicsRegularPathItem()
{
    const qreal PI = 3.14159265359;
    qreal pointDepart = 0.0;
    if (mNVertices % 2 == 0 && mNVertices % 3 == 0)
        pointDepart = PI / 3.0;
    else
    {
        if (mNVertices % 2 == 0)
            pointDepart = PI / 4.0;
        else
            pointDepart = PI/2.0;
    }

    mVertices.clear();

    for (int i=0; i < mNVertices; i++)
    {
        qreal angle = pointDepart + qreal(i)*2.0*PI/qreal(mNVertices);
        mVertices.append(QPair<qreal, qreal>(cos(angle), sin(angle)));
    }
}

void UBGraphicsRegularPathItem::updatePath(QPointF newPos)
{
    QPainterPath path;

    qreal diffX = newPos.x() - mStartPoint.x();
    qreal diffY = newPos.y() - mStartPoint.y();

    if (diffX < 0)
        diffX = -diffX;
    if (diffY < 0)
        diffY = -diffY;

    qreal minFace = qMin(diffX,diffY);
    qreal dist = (sqrt(2*minFace*minFace) - minFace) / 2.0;
    QPointF origin = QPointF(mStartPoint.x() + minFace / 2.0 - dist, mStartPoint.y() + minFace / 2.0 - dist);
    qreal rayon = minFace / 2.0 ;
    QPointF nextPoint = origin - QPointF(mVertices.at(0).first * rayon, mVertices.at(0).second * rayon);
    QPointF firstPoint = nextPoint;

    path.moveTo(firstPoint);
    for (int i = 1; i < mNVertices; i++)
    {
        nextPoint = origin - QPointF(mVertices.at(i).first * rayon, mVertices.at(i).second * rayon);
        path.lineTo(nextPoint);
    }
    path.lineTo(firstPoint);
    setPath(path);
}

void UBGraphicsRegularPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    painter->setBrush(*fillingProperty());
    painter->setPen(*strokeProperty());

    painter->fillPath(path(), painter->brush());
    painter->drawPath(path());
}

void UBGraphicsRegularPathItem::setStartPoint(QPointF pos)
{
    mStartPoint = pos;
}

QRectF UBGraphicsRegularPathItem::boundingRect() const
{
    QRectF retour = path().boundingRect();

    int enlarge = 0;

    if (strokeProperty())
    {
        int thickness = strokeProperty()->width();
        enlarge = thickness/2;
    }

    // IF Handles are drawn
    {
        enlarge = qMax(enlarge, HANDLE_SIZE/2); // if handles are widther than border, enlarge more boundingRect.
    }

    retour.adjust(-enlarge, -enlarge, enlarge, enlarge);

    return retour;
}

void UBGraphicsRegularPathItem::addPoint(const QPointF & point)
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

UBItem *UBGraphicsRegularPathItem::deepCopy() const
{
    UBGraphicsRegularPathItem * copy = new UBGraphicsRegularPathItem();

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsRegularPathItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsRegularPathItem *cp = dynamic_cast<UBGraphicsRegularPathItem*>(copy);
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
            cp->mFillingProperty = new UBFillProperty(*fillingProperty());

        if (hasStrokeProperty())
            cp->mStrokeProperty = new UBStrokeProperty(*strokeProperty());

    }
}
