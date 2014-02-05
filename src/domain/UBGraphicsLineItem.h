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

#ifndef UBGRAPHICSLINEITEM_H
#define UBGRAPHICSLINEITEM_H

#include <QGraphicsLineItem>
#include "UBShape.h"

#include "UBEditable.h"

class UBGraphicsLineItem : public QGraphicsLineItem, public UBShape, public UBEditable
{
    public:
        UBGraphicsLineItem(QGraphicsItem* parent = 0);
        virtual ~UBGraphicsLineItem();

        enum { Type = UBGraphicsItemType::GraphicsShapeItemType };
        virtual int type() const { return Type; }

        virtual UBItem* deepCopy() const;

        virtual void copyItemParameters(UBItem *copy) const;

        // UBItem interface
        void setUuid(const QUuid &pUuid);

        const QPointF startPoint();

        const QPointF endPoint();

        void setStartPoint(QPointF pos);
        void setEndPoint(QPointF pos);

        // QGraphicsItem interface
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void updateHandle(UBAbstractHandle *handle);

        QRectF boundingRect() const;
        QPainterPath shape() const;
    protected:
        // QGraphicsItem interface
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void focusOutEvent(QFocusEvent *event);

    private:
        QPointF mStartPoint;
        QPointF mEndPoint;

        int mMultiClickState;
};

#endif // UBGRAPHICSLINEITEM_H
