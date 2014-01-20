#ifndef UBGRAPHICSPATHITEM_H
#define UBGRAPHICSPATHITEM_H

#include <QGraphicsPathItem>
#include "UBShape.h"

class UBGraphicsPathItem : public QGraphicsPathItem, public UBShape
{
public:
    UBGraphicsPathItem(QGraphicsItem* parent = 0);
    ~UBGraphicsPathItem();

    void addPoint(const QPointF &point);
    inline bool isClosed() const {return mClosed;}
    inline void setClosed(bool closed);


    // UBItem interface
    UBItem *deepCopy() const;
    void copyItemParameters(UBItem *copy) const;
    virtual void setUuid(const QUuid &pUuid);

    // QGraphicsItem interface
    enum { Type = UBGraphicsItemType::GraphicsPathItemType };
    virtual int type() const { return Type; }
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    // QGraphicsItem interface
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);


private:
    void initialize();
    bool mClosed;

    const int HANDLE_SIZE; // in pixels
};

#endif // UBGRAPHICSPATHITEM_H
