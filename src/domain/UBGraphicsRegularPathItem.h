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

#ifndef UBGRAPHICSREGULARSHAPES_H
#define UBGRAPHICSREGULARSHAPES_H

#include "UBAbstractGraphicsPathItem.h"

class UBGraphicsRegularPathItem : public UBAbstractGraphicsPathItem
{
    public:

        enum NVertices
        {
            Triangle = 3,
            Carre,
            Pentagone,
            Hexagone,
            Heptagone,
            Octogone
        };

        UBGraphicsRegularPathItem(int nVertices = Triangle, QPointF startPoint = QPointF(0,0), QGraphicsItem* parent = 0);
        ~UBGraphicsRegularPathItem();

        void createGraphicsRegularPathItem();
        virtual void addPoint(const QPointF &point);

        void updatePath(QPointF newPos);
        void setStartPoint(QPointF pos);        

        // UBItem interface
        UBItem *deepCopy() const;
        void copyItemParameters(UBItem *copy) const;

        enum { Type = UBGraphicsItemType::GraphicsRegularPathItemType };
        virtual int type() const { return Type; }        
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        QRectF boundingRect() const;

        inline const int nVertices() const { return mNVertices; }
        inline const QPointF& startPoint() const { return mStartPoint; }

    private:      
        int mNVertices;
        QList<QPair<double, double> > mVertices;
        QPointF mStartPoint;

        const int HANDLE_SIZE; // in pixels
};

#endif // UBGRAPHICSREGULARSHAPES_H
