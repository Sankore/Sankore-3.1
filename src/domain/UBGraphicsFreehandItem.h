#ifndef UBGRAPHICSFREEHANDITEM_H
#define UBGRAPHICSFREEHANDITEM_H

#include "UBAbstractGraphicsPathItem.h"

class UBGraphicsFreehandItem : public UBAbstractGraphicsPathItem
{
public:
    UBGraphicsFreehandItem(QGraphicsItem *parent = 0);

    virtual void addPoint(const QPointF &point);

    UBItem *deepCopy() const;

    void copyItemParameters(UBItem *copy) const;

    enum { Type = UBGraphicsItemType::GraphicsFreehandItemType};
    virtual int type() const { return Type; }
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // UBGRAPHICSFREEHANDITEM_H
