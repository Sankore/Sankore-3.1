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
#include "UBAbstractGraphicsItem.h"
#include "UBGraphicsDelegateFrame.h"

#include "board/UBDrawingController.h"
#include "UBFreeHandle.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

UBEditableGraphicsLineItem::UBEditableGraphicsLineItem(QGraphicsItem* parent)
    : UBEditableGraphicsPolygonItem(parent)
{
    // Line has Stroke and Fill capabilities :
    initializeStrokeProperty();
    initializeFillingProperty();

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

UBEditableGraphicsLineItem::~UBEditableGraphicsLineItem()
{

}

UBItem *UBEditableGraphicsLineItem::deepCopy() const
{
    UBEditableGraphicsLineItem* copy = new UBEditableGraphicsLineItem();

    copyItemParameters(copy);

    return copy;
}

void UBEditableGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    setStyle(painter);

    painter->drawPath(path());
}

QPointF UBEditableGraphicsLineItem::startPoint() const
{
    return path().elementAt(0);
}

QPointF UBEditableGraphicsLineItem::endPoint() const
{
    if(path().elementCount() == 2)
        return path().elementAt(1);
    else
        return path().elementAt(0);
}

void UBEditableGraphicsLineItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractEditableGraphicsPathItem::copyItemParameters(copy);
}


void UBEditableGraphicsLineItem::setStartPoint(QPointF pos)
{
    prepareGeometryChange();

    QPainterPath p;

    p.moveTo(this->pos());

    if(path().elementCount() == 2)
        p.lineTo(path().elementAt(1));

    setPath(p);

    this->setPos(pos);

    mHandles.at(0)->setPos(pos);

    update();
}

void UBEditableGraphicsLineItem::setEndPoint(QPointF pos)
{
    prepareGeometryChange();

    QPainterPath p;

    p.moveTo(path().elementAt(0));

    p.lineTo(pos);

    setPath(p);

    mHandles.at(1)->setPos(pos);

    update();
}

void UBEditableGraphicsLineItem::updateHandle(UBAbstractHandle *handle)
{
    prepareGeometryChange();

    if(handle->getId() == 0){
        QPainterPath p;

        p.moveTo(handle->pos());

        p.lineTo(path().elementAt(1));

        setPath(p);
    }else if(handle->getId() == 1){
        QPainterPath p;

        p.moveTo(path().elementAt(0));

        p.lineTo(handle->pos());

        setPath(p);
    }

    update();
}

void UBEditableGraphicsLineItem::setLine(QPointF start, QPointF end)
{
    prepareGeometryChange();

    QPainterPath p;
    p.moveTo(start);
    p.lineTo(end);

    setPath(p);

    update();
}

void UBEditableGraphicsLineItem::onActivateEditionMode()
{
    mHandles.at(0)->setPos(startPoint());
    mHandles.at(1)->setPos(endPoint());
}

QPainterPath UBEditableGraphicsLineItem::shape() const
{
    QPainterPath p;

    if(mMultiClickState >= 1 || isSelected()){
        p.addRect(boundingRect());
    }else{
        p = path();
    }

    return p;
}
