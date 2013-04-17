/*
 * Copyright (C) 2012 Webdoc SA
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



#ifndef UBGRAPHICSCACHE_H
#define UBGRAPHICSCACHE_H

#include <QColor>
#include <QGraphicsSceneMouseEvent>

#include "domain/UBItem.h"
#include "core/UB.h"

typedef enum
{
    eMaskShape_Circle,
    eMaskShap_Rectangle
}eMaskShape;

class UBGraphicsCache : public QGraphicsRectItem, public UBItem
{

public:
    enum eMode
    {
        OnClick = 0,
        Persistent
    };

public:
    static UBGraphicsCache* instance(UBGraphicsScene *scene);
    ~UBGraphicsCache();

    enum { Type = UBGraphicsItemType::cacheItemType };

    virtual int type() const{ return Type;}

    virtual UBItem* deepCopy() const;

    virtual void copyItemParameters(UBItem *copy) const;

    QColor maskColor();
    void setMaskColor(QColor color);
    eMaskShape maskshape();
    void setMaskShape(eMaskShape shape);
    int holeWidth();
    int holeHeight();

    void setHoleWidth(int width);
    void setHoleHeight(int height);

    void setHolePos(QPointF pos);
    void setHoleSize(QSize size);

    void setMode(int mode);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    void drawHole(bool draw);

private:
    static QMap<UBGraphicsScene*, UBGraphicsCache*> sInstances;

    QColor mMaskColor;
    eMaskShape mMaskShape;
    bool mDrawMask;
    QPointF mHolePos;
    QSize mOldShapeSize;
    QPointF mOldShapePos;
    UBGraphicsScene* mScene;
    bool mShouldDrawAtHoverEnter;

    eMode mCurrentMode;

    QSize mHoleSize;

    UBGraphicsCache(UBGraphicsScene *scene);
    
    void init();
    QRectF updateRect(QPointF currentPoint);

    QCursor mSavedCursor;
    QCursor mCursorForHole;
};

#endif // UBGRAPHICSCACHE_H
