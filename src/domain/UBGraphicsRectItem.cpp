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
#include "UBFillingProperty.h"
#include "UBStrokeProperty.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"

UBGraphicsRectItem::UBGraphicsRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
    , UBShape()
{
    // Rect has Stroke and Fill capabilities :
    mFillingProperty = new UBFillingProperty();
    mStrokeProperty = new UBStrokeProperty();

    //By default, a rectangle isn't a square
    setAsRectangle();

    setDelegate(new UBGraphicsItemDelegate(this, 0));
    Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(false);
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
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
        copy->mFillingProperty = new UBFillingProperty(*fillingProperty());

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
    }
}

void UBGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    fillingProperty()->fill(painter);
    strokeProperty()->stroke(painter);

    //QGraphicsEllipseItem::paint(painter, &styleOption, widget);
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
        if(r.width() < r.height()){
            r.setHeight(r.width());
        }else{
            r.setWidth(r.height());
        }
    }

    QGraphicsRectItem::setRect(r);
}

void UBGraphicsRectItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    Delegate()->mousePressEvent(event);
}

void UBGraphicsRectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Delegate()->mouseMoveEvent(event))
    {

    }
    else
    {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

