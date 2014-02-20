#ifndef UBABSTRACTGRAPHICSPATHITEM_H
#define UBABSTRACTGRAPHICSPATHITEM_H

#include "UBAbstractGraphicsItem.h"

class UBAbstractGraphicsPathItem : public UBAbstractGraphicsItem
{
public:
    UBAbstractGraphicsPathItem(QGraphicsItem *parent = 0);

    virtual void addPoint(const QPointF &point) = 0;

    QPainterPath path() const
    {
        return mPath;
    }

    void setPath(QPainterPath path)
    {
        mPath = path;
    }

    virtual QRectF boundingRect() const;

    virtual QPainterPath shape() const;

    virtual void copyItemParameters(UBItem *copy) const;

protected:
    QPainterPath mPath;
};

#endif // UBABSTRACTGRAPHICSPATHITEM_H
