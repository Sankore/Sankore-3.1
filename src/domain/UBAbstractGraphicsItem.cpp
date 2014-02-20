#include "UBAbstractGraphicsItem.h"
#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"
#include "customWidgets/UBGraphicsItemAction.h"

UBAbstractGraphicsItem::UBAbstractGraphicsItem(QGraphicsItem *parent):
    QAbstractGraphicsShapeItem(parent)
{
    setDelegate(new UBGraphicsItemDelegate(this, 0, true, false, false));
    Delegate()->init();
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(true);
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::NoResizing);

    setUuid(QUuid::createUuid());
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
}

UBAbstractGraphicsItem::~UBAbstractGraphicsItem()
{

}

void UBAbstractGraphicsItem::setStyle(Qt::PenStyle penStyle)
{
    Qt::BrushStyle brushStyle = Qt::NoBrush;
    if (hasFillingProperty())
        brushStyle = brush().style();

    setStyle(brushStyle, penStyle);
}

void UBAbstractGraphicsItem::setStyle(Qt::BrushStyle brushStyle)
{
    Qt::PenStyle penStyle = Qt::NoPen;
    if(hasStrokeProperty())
        penStyle = pen().style();

    setStyle(brushStyle, penStyle);
}

void UBAbstractGraphicsItem::setStyle(Qt::BrushStyle brushStyle, Qt::PenStyle penStyle)
{
    if(hasFillingProperty()){
        QBrush b = brush();
        b.setStyle(brushStyle);
        setBrush(b);
    }

    if(hasStrokeProperty()){
        QPen p = pen();
        p.setStyle(penStyle);
        setPen(p);
    }
}

void UBAbstractGraphicsItem::setFillColor(const QColor& color)
{
    if(hasFillingProperty()){
        QBrush b = brush();
        b.setColor(color);
        setBrush(b);
    }
}

void UBAbstractGraphicsItem::setStrokeColor(const QColor& color)
{
    if(hasStrokeProperty()){
        QPen p = pen();
        p.setColor(color);
        setPen(p);
    }
}

void UBAbstractGraphicsItem::setStrokeSize(int size)
{
    if(hasStrokeProperty()){
        QPen p = pen();
        p.setWidth(size);
        setPen(p);
    }
}

void UBAbstractGraphicsItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid));
}

QVariant UBAbstractGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QAbstractGraphicsShapeItem::itemChange(change, newValue);
}

void UBAbstractGraphicsItem::setStyle(QPainter *painter)
{
    if(hasStrokeProperty()){
        painter->setPen(pen());
    }

    if(hasFillingProperty()){
        painter->setBrush(brush());
    }
}

void UBAbstractGraphicsItem::initializeStrokeProperty()
{
    QPen p(Qt::SolidLine);
    p.setWidth(1);
    p.setColor(Qt::black);
    setPen(p);
}

void UBAbstractGraphicsItem::initializeFillingProperty()
{
    QBrush b(Qt::SolidPattern);
    b.setColor(Qt::black);
    setBrush(b);
}

QRectF UBAbstractGraphicsItem::adjustBoundingRect(QRectF rect) const
{
    if(hasStrokeProperty()){
        int r = pen().width();
        rect.adjust(-r, -r, r, r);
    }

    return rect;
}

bool UBAbstractGraphicsItem::hasFillingProperty() const
{
    return brush() != QBrush();
}

bool UBAbstractGraphicsItem::hasStrokeProperty() const
{
    return pen() != QPen();
}

bool UBAbstractGraphicsItem::hasGradient() const
{
    return brush().gradient() != NULL;
}

void UBAbstractGraphicsItem::copyItemParameters(UBItem *copy) const
{
    UBAbstractGraphicsItem *cp = dynamic_cast<UBAbstractGraphicsItem*>(copy);

    if(!cp) return;

    cp->setPos(this->pos());
    cp->setTransform(this->transform());
    cp->setFlag(QGraphicsItem::ItemIsMovable, true);
    cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
    cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));

    if(Delegate()->action()){
        if(Delegate()->action()->linkType() == eLinkToAudio){
            UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
            UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
            cp->Delegate()->setAction(action);
        }
        else
            cp->Delegate()->setAction(Delegate()->action());
    }

    if(cp->hasFillingProperty()){
        cp->setBrush(brush());
    }

    if(cp->hasStrokeProperty()){
        cp->setPen(pen());
    }
}
