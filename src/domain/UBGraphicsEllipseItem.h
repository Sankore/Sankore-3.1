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



#ifndef UBGRAPHICSELLIPSEITEM_H
#define UBGRAPHICSELLIPSEITEM_H

#include <QGraphicsEllipseItem>
#include "UBShape.h"

class UBGraphicsEllipseItem : public QGraphicsEllipseItem, public UBShape
{
public:
    UBGraphicsEllipseItem(QGraphicsItem* parent = 0);
    virtual ~UBGraphicsEllipseItem();

    virtual int type() const
    {
        return Type;
    }

    virtual UBItem* deepCopy() const;

    virtual void copyItemParameters(UBItem *copy) const;

    QPointF center() const;
    qreal radius(Qt::Orientation orientation) const;

    // UBItem interface
    void setUuid(const QUuid &pUuid);

    // QGraphicsItem interface
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


protected:
    // QGraphicsItem interface
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // UBGRAPHICSELLIPSEITEM_H
