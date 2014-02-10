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

#include "UBGraphicsLineItem.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "UBShape.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"
#include "UBFreeHandle.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

UBGraphicsLineItem::UBGraphicsLineItem(QGraphicsItem* parent)
    : QGraphicsLineItem(parent)
    , UBShape()
{
    // Line has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

    mMultiClickState = 0;

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

    UBFreeHandle *startHandle = new UBFreeHandle;
    UBFreeHandle *endHandle = new UBFreeHandle;

    endHandle->setId(1);

    startHandle->setParentItem(this);
    endHandle->setParentItem(this);

    startHandle->setEditableObject(this);
    endHandle->setEditableObject(this);

    startHandle->hide();
    endHandle->hide();

    mHandles.push_back(startHandle);
    mHandles.push_back(endHandle);
}

UBGraphicsLineItem::~UBGraphicsLineItem()
{
    delete mFillingProperty;
    delete mStrokeProperty;

    mFillingProperty = NULL;
    mStrokeProperty = NULL;
}

void UBGraphicsLineItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}

UBItem *UBGraphicsLineItem::deepCopy() const
{
    UBGraphicsLineItem* copy = new UBGraphicsLineItem();

    if (hasFillingProperty())
        copy->mFillingProperty = new UBFillProperty(*fillingProperty());

    if (hasStrokeProperty())
        copy->mStrokeProperty = new UBStrokeProperty(*strokeProperty());

    copyItemParameters(copy);

    return copy;
}

void UBGraphicsLineItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsLineItem *cp = dynamic_cast<UBGraphicsLineItem*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setLine(line());
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

        for(int i = 0; i < mHandles.size(); i++){
            UBFreeHandle *handle = new UBFreeHandle(dynamic_cast<UBFreeHandle*>(mHandles.at(i)));
            cp->mHandles.push_back(handle);
            UBApplication::boardController->controlView()->scene()->addItem(handle);
            handle->setParentItem(cp);
            handle->setEditableObject(cp);
        }
    }
}

void UBGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    painter->setBrush(*fillingProperty());
    painter->setPen(*strokeProperty());

    painter->drawLine(mStartPoint, mEndPoint);
}

//pas de référence : sémantique de valeur
const QPointF UBGraphicsLineItem::startPoint()
{
    return mStartPoint;
}

const QPointF UBGraphicsLineItem::endPoint()
{
    return mEndPoint;
}

void UBGraphicsLineItem::setStartPoint(QPointF pos)
{
    mStartPoint = pos;
}

void UBGraphicsLineItem::setEndPoint(QPointF pos)
{
    mEndPoint = pos;
}

QVariant UBGraphicsLineItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsLineItem::itemChange(change, newValue);
}

void UBGraphicsLineItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    mMultiClickState++;

    QGraphicsLineItem::mousePressEvent(event);

    if(mMultiClickState >= 1){
        mHandles.at(0)->setPos(mStartPoint);
        mHandles.at(1)->setPos(mEndPoint);

        Delegate()->showFrame(false);
        setFocus();
        showEditMode(true);
    }
}

void UBGraphicsLineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(mMultiClickState == 0){
        Delegate()->mouseMoveEvent(event);
        QGraphicsLineItem::mouseMoveEvent(event);
    }
}

void UBGraphicsLineItem::updateHandle(UBAbstractHandle *handle)
{
    if(handle->getId() == 0){
        mStartPoint = handle->pos();
    }else if(handle->getId() == 1){
        mEndPoint = handle->pos();
    }

    setLine(QLineF(mStartPoint, mEndPoint));
}

void UBGraphicsLineItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}

QRectF UBGraphicsLineItem::boundingRect() const
{
    QRectF rect = QGraphicsLineItem::boundingRect();

    int thickness = strokeProperty()->width();
    rect.adjust(-thickness/2, -thickness/2, thickness/2, thickness/2); // enlarge boundingRect, in order to contain border thickness.

    if(mMultiClickState >= 1){
        qreal r = mHandles.first()->radius();

        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

QPainterPath UBGraphicsLineItem::shape() const
{
    QPainterPath path;

    if(mMultiClickState >= 1 || isSelected()){
        path.addRect(boundingRect());
        return path;
    }else{
        return QGraphicsLineItem::shape();
    }
}

void UBGraphicsLineItem::deactivateEditionMode()
{
    if(mMultiClickState >= 1){
        mMultiClickState = 0;
        showEditMode(false);
    }
}

void UBGraphicsLineItem::focusHandle(UBAbstractHandle *handle)
{
    Q_UNUSED(handle)

    setSelected(true);
    Delegate()->showFrame(false);
}
