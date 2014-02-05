#include "UBGraphicsRegularPathItem.h"
#include "UBGraphicsDelegateFrame.h"

#include <cmath>
#include "UBDiagonalHandle.h"

#include <customWidgets/UBGraphicsItemAction.h>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "UBGraphicsScene.h"

QPainterPath d_oldPath;
QPointF d_center;
QPainterPath d_circle;

UBGraphicsRegularPathItem::UBGraphicsRegularPathItem(int nVertices, QPointF startPos, QGraphicsItem * parent)
    : UBAbstractGraphicsPathItem(parent)
    , UB1HandleEditable()
    , mMultiClickState(0)
    , mNVertices(nVertices)
    , mStartPoint(startPos)
{
    initializeStrokeProperty();
    initializeFillingProperty();
    createGraphicsRegularPathItem();

    getHandle()->setParentItem(this);
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

    qreal dist = 0;//(sqrt(2*minFace*minFace) - minFace) / 2.0;

    mCenter = QPointF(mStartPoint.x() + minFace / 2.0 - dist, mStartPoint.y() + minFace / 2.0 - dist);

    mRadius = minFace / 2.0 ;
    QPointF nextPoint = mCenter - QPointF(mVertices.at(0).first * mRadius, mVertices.at(0).second * mRadius);
    QPointF firstPoint = nextPoint;

    path.moveTo(firstPoint);

    for (int i = 1; i < mNVertices; i++)
    {
        nextPoint = mCenter - QPointF(mVertices.at(i).first * mRadius, mVertices.at(i).second * mRadius);

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
    mStartPoint = mapToItem(this, pos);
}

QRectF UBGraphicsRegularPathItem::boundingRect() const
{
    QRectF retour = path().boundingRect();

    int enlarge = 0;

    if(mMultiClickState >= 2){
        QPointF ph = getHandle()->pos();
        qreal r = getHandle()->radius();

        QPointF diff = (ph - retour.topLeft()) - (retour.bottomRight() - retour.topLeft());

        retour.adjust(0, 0, diff.x() + r, diff.y() + r);
    }

    if (strokeProperty())
    {
        int thickness = strokeProperty()->width();
        enlarge = thickness/2;
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

        /*
        UBDiagonalHandle *handle = new UBDiagonalHandle(dynamic_cast<UBDiagonalHandle*>(getHandle()));
        handle->hide();
        cp->mHandles.push_back(handle);
        UBApplication::boardController->controlView()->scene()->addItem(handle);
        handle->setParentItem(cp);
        handle->setEditableObject(cp);
        */

        cp->mVertices = mVertices;
        cp->mNVertices = mNVertices;
    }
}

void UBGraphicsRegularPathItem::updateHandle(UBAbstractHandle *handle)
{
    QPointF p = handle->pos();
/*
    //We have to offset the given posisition to updatePath
    QPointF diff(p - mStartPoint);

    if (diff.x() < 0)
        diff.setX(-diff.x());
    if (diff.y() < 0)
        diff.setY(-diff.y());

    qreal diffMin = qMin(diff.x(),diff.y());
    qreal r = getHandle()->radius();

    qreal dist = sqrt(2*diffMin*diffMin) - diffMin;

    qreal offset = dist - r;

    if(hasStrokeProperty())
        dist -= strokeProperty()->width();

    updatePath(QPointF(p.x() + offset, p.y() + offset));
*/
    QPointF diff = p - boundingRect().topLeft();

    if (diff.x() < 0)
        diff.setX(-diff.x());
    if (diff.y() < 0)
        diff.setY(-diff.y());

    qreal diffMin = qMin(diff.x(),diff.y());

    updatePath(QPointF(p.x() - getHandle()->radius(), p.y() -
                       getHandle()->radius()));
    //handle->setPos(boundingRect().bottomRight().x() - r, boundingRect().bottomRight().y() - r);
}

void UBGraphicsRegularPathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QRectF oldBoundingRect = boundingRect();

    mMultiClickState++;

    UBAbstractGraphicsPathItem::mousePressEvent(event);

    if(mMultiClickState == 2){
        QPainterPath circle;

        circle.addEllipse(mCenter, mRadius, mRadius);

        getHandle()->setPos(circle.boundingRect().bottomRight());

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

void UBGraphicsRegularPathItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState > 1){
        mMultiClickState = 0;
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
        showEditMode(false);
    }
}

QPainterPath UBGraphicsRegularPathItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void UBGraphicsRegularPathItem::redim(float size)
{
    qreal radius = size/2;

    QPointF center(radius, radius);

    //center += this->boundingRect().topLeft();

    QPainterPath path;

    /*

    QPointF diff2 = this->boundingRect().topLeft() - getHandle()->pos();

    qreal r = qMin(diff2.x(), diff2.y());

    */

    /*
    qreal remaining = (sqrt(diff2.x()*diff2.x() + diff2.y()*diff2.y()) - r);

    radius = (r + remaining)/2;
    */

    //radius = r / 2;// - getHandle()->radius() / 2;

    QPointF vertice(mVertices.at(0).first, mVertices.at(0).second);
    QPointF currentPoint = center - vertice*radius;
    QPointF firstPoint = currentPoint;

    path.moveTo(currentPoint);

    for (int i = 1; i < mNVertices; i++){
        vertice = QPointF(mVertices.at(i).first, mVertices.at(i).second);
        currentPoint = center - vertice*radius;
        path.lineTo(currentPoint);
    }

    path.lineTo(firstPoint);
/*
    QPointF diff = this->path().boundingRect().topLeft() - path.boundingRect().topLeft();

    QPainterPath cpath;

    center += diff;

    d_center = center;

    vertice = QPointF(mVertices.at(0).first, mVertices.at(0).second);
    firstPoint = vertice;

    cpath.moveTo(center - vertice * radius);

    for(int i = 1; i < mNVertices; i++){
        vertice = QPointF(mVertices.at(i).first, mVertices.at(i).second);
        cpath.lineTo(center - vertice * radius);
    }

    cpath.lineTo(center - firstPoint * radius);
*/
    setPath(path);
}
