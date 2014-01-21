#ifndef UBGRAPHICSPATHITEM_H
#define UBGRAPHICSPATHITEM_H

#include "UBAbstractGraphicsPathItem.h"

class UBGraphicsPathItem : public UBAbstractGraphicsPathItem
{
public:
    UBGraphicsPathItem(QGraphicsItem* parent = 0);
    ~UBGraphicsPathItem();

    virtual void addPoint(const QPointF &point);
    inline bool isClosed() const {return mClosed;}
    inline void setClosed(bool closed);


    // UBItem interface
    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;

    // QGraphicsItem interface
    enum { Type = UBGraphicsItemType::GraphicsPathItemType };
    virtual int type() const { return Type; }
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QRectF boundingRect() const;
private:
    bool mClosed;

    const int HANDLE_SIZE; // in pixels
};

#endif // UBGRAPHICSPATHITEM_H
