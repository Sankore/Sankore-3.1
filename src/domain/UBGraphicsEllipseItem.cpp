/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBGraphicsEllipseItem.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "UBShape.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"

#include "UBHorizontalHandle.h"
#include "UBVerticalHandle.h"
#include "UBDiagonalHandle.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

UBGraphicsEllipseItem::UBGraphicsEllipseItem(QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent)
    , UBShape()
    , UB3HandlesEditable()
{
    // Ellipse has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    mMultiClickState = 0;

    //By default, an ellipse isn't a circle
    mIsCircle = false;

    setDelegate(new UBGraphicsItemDelegate(this, 0));
    Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(true);    
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    horizontalHandle()->setParentItem(this);
    verticalHandle()->setParentItem(this);
    diagonalHandle()->setParentItem(this);
}

UBGraphicsEllipseItem::~UBGraphicsEllipseItem()
{
    delete mFillingProperty;
    delete mStrokeProperty;

    mFillingProperty = NULL;
    mStrokeProperty = NULL;
}

void UBGraphicsEllipseItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

UBItem *UBGraphicsEllipseItem::deepCopy() const
{
    UBGraphicsEllipseItem* copy = new UBGraphicsEllipseItem();

    if (hasFillingProperty())
        copy->mFillingProperty = new UBFillProperty(*fillingProperty());

    if (hasStrokeProperty())
        copy->mStrokeProperty = new UBStrokeProperty(*strokeProperty());

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsEllipseItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsEllipseItem *cp = dynamic_cast<UBGraphicsEllipseItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->rect());
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

        if(mIsCircle){
            cp->setAsCircle();
        }
    }
}

QPointF UBGraphicsEllipseItem::center() const
{
    QPointF centre;

    centre.setX(rect().x() + rect().width()/2);
    centre.setY(rect().y() + rect().height()/2);

    return centre;
}

qreal UBGraphicsEllipseItem::radius(Qt::Orientation orientation) const
{
    qreal rayon;

    if (orientation == Qt::Horizontal)
    {
        rayon = rect().width()/2;
    }
    else
    {
        rayon = rect().height()/2;
    }

    return rayon;
}

void UBGraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    painter->setBrush(*fillingProperty());
    painter->setPen(*strokeProperty());

    painter->drawEllipse(rect());
}

QVariant UBGraphicsEllipseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsEllipseItem::itemChange(change, newValue);
}

void UBGraphicsEllipseItem::setRect(const QRectF &rect)
{
    QRectF r(rect);    

    if(mIsCircle)
    {
        if (r.width() < 0)
        {
            if (r.height() < 0)
                r.setWidth(r.height());
            else
                r.setWidth(-r.height());
        }
        else
        {
            if (r.height() < 0)
                r.setHeight(-r.width());
            else
                r.setHeight(r.width());
        }
    }

    QGraphicsEllipseItem::setRect(r);
}

QRectF UBGraphicsEllipseItem::boundingRect() const
{
    QRectF rect = QGraphicsEllipseItem::boundingRect();

    if (strokeProperty())
    {
        int thickness = strokeProperty()->width();
        rect.adjust(-thickness/2, -thickness/2, thickness/2, thickness/2); // enlarge boundingRect, in order to contain border thickness.
    }

    if(mMultiClickState >= 1){
        qreal r = horizontalHandle()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

void UBGraphicsEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;

    QGraphicsEllipseItem::mousePressEvent(event);

    if(mMultiClickState >= 1){
        QRectF rectangle = rect();
        QPointF bottomRight = rectangle.bottomRight();

        horizontalHandle()->setPos(bottomRight.x(), rectangle.y() + rectangle.height()/2);
        verticalHandle()->setPos(rectangle.x() + rectangle.width()/2, bottomRight.y());
        diagonalHandle()->setPos(bottomRight.x(), bottomRight.y());

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

void UBGraphicsEllipseItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mMultiClickState == 0){
        Delegate()->mouseMoveEvent(event);
        QGraphicsEllipseItem::mouseMoveEvent(event);
    }
}

void UBGraphicsEllipseItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}

void UBGraphicsEllipseItem::updateHandle(UBAbstractHandle *handle)
{
    QRectF newRect = rect();

    if(handle->getId() == 1){
        //it's the vertical handle
        newRect.setBottom(handle->pos().y());

        horizontalHandle()->setPos(newRect.bottomRight().x(), newRect.y() + newRect.height()/2);

        diagonalHandle()->setPos(newRect.bottomRight().x(), newRect.bottomRight().y());
    }else if(handle->getId() == 0){
        //it's the horizontal handle
        newRect.setRight(handle->pos().x());

        verticalHandle()->setPos(newRect.x() + newRect.width()/2 , newRect.bottomRight().y() );

        diagonalHandle()->setPos(newRect.bottomRight().x() , newRect.bottomRight().y() );
    }else{
        //it's the diagonal handle
        newRect.setRight(handle->pos().x());
        newRect.setBottom(handle->pos().y());

        verticalHandle()->setPos(newRect.x() + newRect.width()/2 , newRect.bottomRight().y());

        horizontalHandle()->setPos(newRect.bottomRight().x() , newRect.y() + newRect.height()/2);
    }

    setRect(newRect);

    if(fillingProperty()->gradient()){
        QLinearGradient g(rect().topLeft(), rect().topRight());

        g.setColorAt(0, fillingProperty()->gradient()->stops().at(0).second);
        g.setColorAt(1, fillingProperty()->gradient()->stops().at(1).second);

        setFillingProperty(new UBFillProperty(g));
    }
}

QPainterPath UBGraphicsEllipseItem::shape() const
{
    QPainterPath path;

    path.addRect(boundingRect());

    return path;
}

void UBGraphicsEllipseItem::deactivateEditionMode()
{
    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}
