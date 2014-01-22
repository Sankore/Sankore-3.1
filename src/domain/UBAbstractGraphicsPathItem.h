#ifndef UBABSTRACTGRAPHICSPATHITEM_H
#define UBABSTRACTGRAPHICSPATHITEM_H

#include <QGraphicsItem>
#include "UBShape.h"

class UBAbstractGraphicsPathItem : public QGraphicsPathItem, public UBShape
{
public:
    UBAbstractGraphicsPathItem(QGraphicsItem *parent = 0);

    virtual int type() const = 0;

    // UBItem interface
    virtual void setUuid(const QUuid &pUuid);

    // QGraphicsItem interface
    virtual QRectF boundingRect() const
    {
        return path().boundingRect();
    }

    virtual void addPoint(const QPointF &point) = 0;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) = 0;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // UBABSTRACTGRAPHICSPATHITEM_H
