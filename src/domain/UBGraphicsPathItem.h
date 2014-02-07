#ifndef UBGRAPHICSPATHITEM_H
#define UBGRAPHICSPATHITEM_H

#include "UBAbstractGraphicsPathItem.h"
#include "UBEditable.h"

class UBGraphicsPathItem : public UBAbstractGraphicsPathItem, public UBEditable
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
    QPainterPath shape() const;

    virtual void updateHandle(UBAbstractHandle *handle);
    virtual void deactivateEditionMode();

    void focusHandle(UBAbstractHandle *handle);
protected:
    void focusOutEvent(QFocusEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    bool mClosed;

    int mMultiClickState;

    QPointF mStartEndPoint[2];

    int HANDLE_SIZE; //in pixel
};

#endif // UBGRAPHICSPATHITEM_H
