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
#include "UBFillingProperty.h"
#include "UBStrokeProperty.h"

UBGraphicsEllipseItem::UBGraphicsEllipseItem(QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent)
    , UBShape()
{
    mFillingProperty = new UBFillingProperty();
    mStrokeProperty = new UBStrokeProperty();

    setDelegate(new UBGraphicsItemDelegate(this, 0));
    Delegate()->init();
    //Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
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

    copyItemParameters(copy);
    // TODO

    return copy;
}

void UBGraphicsEllipseItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsEllipseItem *cp = dynamic_cast<UBGraphicsEllipseItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
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

    fillingProperty()->fill(painter);
    strokeProperty()->stroke(painter);

    //QGraphicsEllipseItem::paint(painter, &styleOption, widget);
    painter->drawEllipse(rect());
}

QVariant UBGraphicsEllipseItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsEllipseItem::itemChange(change, newValue);
}
