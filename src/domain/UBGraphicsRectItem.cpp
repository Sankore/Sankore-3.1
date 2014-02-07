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

#include "UBGraphicsRectItem.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "UBShape.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"

UBGraphicsRectItem::UBGraphicsRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , UBShape()
{
    // Rect has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    mMultiClickState = 0;

    //By default, a rectangle isn't a square
    setAsRectangle();

    setDelegate(new UBGraphicsItemDelegate(this, 0));
    Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(true);
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    horizontalHandle()->setParentItem(this);
    verticalHandle()->setParentItem(this);
    diagonalHandle()->setParentItem(this);
}

UBGraphicsRectItem::~UBGraphicsRectItem()
{
    delete mFillingProperty;
    delete mStrokeProperty;

    mFillingProperty = NULL;
    mStrokeProperty = NULL;
}

void UBGraphicsRectItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

UBItem *UBGraphicsRectItem::deepCopy() const
{
    UBGraphicsRectItem* copy = new UBGraphicsRectItem();

    if (hasFillingProperty())
        copy->mFillingProperty = new UBFillProperty(*fillingProperty());

    if (hasStrokeProperty())
        copy->mStrokeProperty = new UBStrokeProperty(*strokeProperty());

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsRectItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsRectItem *cp = dynamic_cast<UBGraphicsRectItem*>(copy);
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

        if(mIsSquare){
            cp->setAsSquare();
        }
    }
}

void UBGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    painter->setBrush(*fillingProperty());
    painter->setPen(*strokeProperty());

    painter->drawRect(rect());
}

QVariant UBGraphicsRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsRectItem::itemChange(change, newValue);
}

void UBGraphicsRectItem::setRect(const QRectF &rect)
{
    QRectF r(rect);

    if(mIsSquare){
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

    QGraphicsRectItem::setRect(r);
}

QRectF UBGraphicsRectItem::boundingRect() const
{
    QRectF rect = QGraphicsRectItem::boundingRect();

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


QPainterPath UBGraphicsRectItem::shape() const
{
    QPainterPath path;

    path.addRect(boundingRect());

    return path;
}

void UBGraphicsRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mMultiClickState++;

    QGraphicsRectItem::mousePressEvent(event);

    if(mMultiClickState >= 1){
        QPointF bottomRight = rect().bottomRight();

        horizontalHandle()->setPos(bottomRight.x(), rect().y() + rect().height()/2);
        verticalHandle()->setPos(rect().x() + rect().width()/2, bottomRight.y());
        diagonalHandle()->setPos(bottomRight.x(), bottomRight.y());

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

void UBGraphicsRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mMultiClickState == 0){
        Delegate()->mouseMoveEvent(event);
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

void UBGraphicsRectItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}

void UBGraphicsRectItem::updateHandle(UBAbstractHandle *handle)
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

void UBGraphicsRectItem::deactivateEditionMode()
{
    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}
