#include "UBShapeFactory.h"
#include "UBGraphicsEllipseItem.h"
#include "UBGraphicsRectItem.h"
#include "UBGraphicsLineItem.h"
#include "UBGraphicsRegularPathItem.h"
#include "UBGraphicsPathItem.h"
#include "UBGraphicsFreehandItem.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "UBGraphicsScene.h"

UBShapeFactory::UBShapeFactory():
    mCurrentShape(NULL),
    mBoardView(NULL),
    mIsCreating(false),
    mIsPress(false),
    mIsRegularShape(true),
    mFirstClickForFreeHand(true),
    mCurrentStrokeColor(Qt::black),
    mCurrentFillFirstColor(Qt::transparent),
    mCurrentFillSecondColor(Qt::transparent),
    mDrawingController(NULL),
    mCurrentBrushStyle(Qt::SolidPattern),
    mCurrentPenStyle(Qt::SolidLine),
    mThickness(3),
    mFillType(Transparent)
{

}

void UBShapeFactory::prepareChangeFill()
{
   mDrawingController->setStylusTool(UBStylusTool::ChangeFill);
   mIsRegularShape = false;
   mIsCreating = false;
   mShapeType = None;
}

void getFamily(QGraphicsItem* o)
{
    if (o)
        if (o->parentItem())
            getFamily(o->parentItem());

    qDebug() << o;
}

void UBShapeFactory::changeFillColor(const QPointF& pos)
{
    UBGraphicsScene* scene = mBoardView->scene();
    QGraphicsItem* item = scene->itemAt(pos, QTransform());

    if (item)
    {
        UBShape * shape = dynamic_cast<UBShape*>(item);
        if (shape)        
        {
            if (mFillType != Gradient)
            {
                delete shape->fillingProperty();
                shape->initializeFillingProperty();
                shape->fillingProperty()->setColor(mCurrentFillFirstColor);
            }
            else
            {
                setGradientFillingProperty(shape);
            }
            item->update();
        }
    }
}

void UBShapeFactory::setGradientFillingProperty(UBShape* shape)
{
    //besoin de "downcaster" davantage pour recuperer le bounding rect de l'objet
    QRectF recup;

    UBGraphicsRectItem* rect = NULL;
    UBGraphicsLineItem* line = NULL;
    UBGraphicsFreehandItem * freeHandItem = NULL;
    UBGraphicsPathItem * pathItem = NULL;
    UBGraphicsRegularPathItem * regularPathItem = NULL;
    UBGraphicsEllipseItem * ellipse = dynamic_cast<UBGraphicsEllipseItem*>(shape);
    if (!ellipse)
    {
        rect = dynamic_cast<UBGraphicsRectItem*>(shape);
        if (!rect)
        {
            line = dynamic_cast<UBGraphicsLineItem*>(shape);
            if (!line)
            {
                freeHandItem = dynamic_cast<UBGraphicsFreehandItem*>(shape);
                if (!freeHandItem)
                {
                    pathItem = dynamic_cast<UBGraphicsPathItem*>(shape);
                    if (!regularPathItem)
                    {
                        regularPathItem = dynamic_cast<UBGraphicsRegularPathItem*>(shape);
                    }
                }
            }
        }
    }
    if (ellipse)
        recup = ellipse->rect();
    else if (rect)
        recup = rect->rect();
    else if (line)
        recup = line->boundingRect();
    else if (freeHandItem)
        recup = freeHandItem->boundingRect();
    else if (pathItem)
        recup = pathItem->boundingRect();
    else if (regularPathItem)
        recup = regularPathItem->boundingRect();


    QLinearGradient gradient(recup.topLeft(), recup.topRight());
    gradient.setColorAt(0, mCurrentFillFirstColor);
    gradient.setColorAt(1, mCurrentFillSecondColor);    
    shape->setFillingProperty(new UBFillProperty(gradient));
}

UBShapeFactory::FillType UBShapeFactory::fillType()
{
    return mFillType;
}

void UBShapeFactory::setFillType(FillType fillType)
{
    mFillType = fillType;
}

void UBShapeFactory::init()
{
    mBoardView = UBApplication::boardController->controlView();
    mDrawingController = UBDrawingController::drawingController();

    connect(mBoardView, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseRelease(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));

}


UBShape* UBShapeFactory::instanciateCurrentShape()
{
    switch (mShapeType) {
    case Ellipse:
    {
        UBGraphicsEllipseItem * ellipse = new UBGraphicsEllipseItem();
        mCurrentShape = ellipse;
        mBoundingRect = ellipse->rect();
        break;
    }
    case Circle:
    {
        UBGraphicsEllipseItem * ellipse = new UBGraphicsEllipseItem();
        ellipse->setAsCircle();
        mCurrentShape = ellipse;
        mBoundingRect = ellipse->rect();
        break;
    }
    case Rectangle:
    {
        UBGraphicsRectItem* rect = new UBGraphicsRectItem();
        rect->setAsRectangle();
        mCurrentShape = rect;
        mBoundingRect = rect->rect();
        break;
    }
    case Square:
    {
        UBGraphicsRectItem* square = new UBGraphicsRectItem();
        square->setAsSquare();
        mCurrentShape = square;
        mBoundingRect = square->rect();
        break;
    }
    case Line:
    {
        UBGraphicsLineItem* line = new UBGraphicsLineItem();
        mCurrentShape = line;
        mBoundingRect = line->boundingRect();
        break;
    }
    case Pen:
    {
        UBGraphicsFreehandItem * pathItem = new UBGraphicsFreehandItem();
        mCurrentShape = pathItem;
        mBoundingRect = pathItem->boundingRect();
        break;
    }
    case Polygon:
    {
        UBGraphicsPathItem * pathItem = new UBGraphicsPathItem();
        mCurrentShape = pathItem;
        mBoundingRect = pathItem->boundingRect();
        break;
    }
    case RegularPolygon:
    {
        UBGraphicsRegularPathItem* regularPathItem = new UBGraphicsRegularPathItem(mNVertices);
        mCurrentShape = regularPathItem;
        mBoundingRect = regularPathItem->boundingRect();
        break;
    }
    default:
        break;
    }

    mCurrentShape->applyStyle(mCurrentBrushStyle, mCurrentPenStyle);
    if (mFillType != Gradient)
    {
        mCurrentShape->applyFillColor(mCurrentFillFirstColor);
    }
    else
    {
        QLinearGradient gradient(mBoundingRect.topLeft(), mBoundingRect.topRight());
        gradient.setColorAt(0, mCurrentFillFirstColor);
        gradient.setColorAt(1, mCurrentFillSecondColor);
        mCurrentShape->setFillingProperty(new UBFillProperty(gradient));
    }
    mCurrentShape->applyStrokeColor(mCurrentStrokeColor);
    mCurrentShape->setStrokeSize(mThickness);

    return mCurrentShape;
}

void UBShapeFactory::createEllipse(bool create)
{
    if(create){
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Ellipse;
    }
}

void UBShapeFactory::createCircle(bool create)
{
    if(create){
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Circle;
    }
}

void UBShapeFactory::createRectangle(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Rectangle;
    }
}

void UBShapeFactory::createSquare(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Square;
    }
}

void UBShapeFactory::createLine(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Line;
    }
}

void UBShapeFactory::createPen(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = false;
        mIsCreating = true;
        mShapeType = Pen;
    }
}

void UBShapeFactory::createRegularPolygon(int nVertices)
{
    mDrawingController->setStylusTool(UBStylusTool::Drawing);
    mIsRegularShape = false;
    mIsCreating = true;
    mShapeType = RegularPolygon;
    mNVertices = nVertices;
}

void UBShapeFactory::createPolygon(bool create)
{
    if(create)
    {
        mDrawingController->setStylusTool(UBStylusTool::Drawing);
        mIsRegularShape = false;
        mIsCreating = true;
        mShapeType = Polygon;
    }
}

void UBShapeFactory::onMouseMove(QMouseEvent *event)
{
    if(mIsCreating && mIsPress){
        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if(mIsRegularShape)
        {
            if (mShapeType == Ellipse || mShapeType == Circle)
            {
                UBGraphicsEllipseItem* shape = dynamic_cast<UBGraphicsEllipseItem*>(mCurrentShape);
                QRectF rect = shape->rect();

                qreal w = cursorPosition.x() - rect.x();
                qreal h = cursorPosition.y() - rect.y();

                mBoundingRect = QRectF(rect.x(), rect.y(), w, h);
                shape->setRect(mBoundingRect);

            }
            else if (mShapeType == Rectangle || mShapeType == Square)
            {
                UBGraphicsRectItem* shape = dynamic_cast<UBGraphicsRectItem*>(mCurrentShape);
                QRectF rect = shape->rect();

                qreal w = cursorPosition.x() - rect.x();
                qreal h = cursorPosition.y() - rect.y();

                mBoundingRect = QRectF(rect.x(), rect.y(), w, h);
                shape->setRect(mBoundingRect);
            }
            else if (mShapeType == Line)
            {
                UBGraphicsLineItem* line= dynamic_cast<UBGraphicsLineItem*>(mCurrentShape);
                QLineF newLine(line->startPoint(), cursorPosition);
                line->setLine(newLine);
                line->setEndPoint(cursorPosition);

                mBoundingRect = line->boundingRect();
            }
        }else{
            if(mShapeType == Pen){
                UBGraphicsFreehandItem *freeHand = dynamic_cast<UBGraphicsFreehandItem*>(mCurrentShape);
                if (freeHand)
                {
                    QPointF point = event->pos() - freeHand->path().currentPosition();

                    if(point.manhattanLength() > 3){
                        freeHand->addPoint(cursorPosition);
                    }

                    mBoundingRect = freeHand->boundingRect();
                }
            }else if (mShapeType == RegularPolygon){
                UBGraphicsRegularPathItem* regularPathItem = dynamic_cast<UBGraphicsRegularPathItem*>(mCurrentShape);
                regularPathItem->updatePath(cursorPosition);
                mBoundingRect = regularPathItem->boundingRect();
            }
        }
        if (mCurrentShape)
        {
            if (mFillType == Gradient)
            {
                setGradientFillingProperty(mCurrentShape);
            }
        }
    }
}

void UBShapeFactory::onMousePress(QMouseEvent *event)
{
    if(mIsCreating){
        mIsPress = true;

        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if(mIsRegularShape){
            if (mShapeType == Ellipse || mShapeType == Circle)
            {
                UBGraphicsEllipseItem* ellipse = dynamic_cast<UBGraphicsEllipseItem*>(instanciateCurrentShape());

                mBoundingRect = QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0);
                ellipse->setRect(mBoundingRect);

                mBoardView->scene()->addItem(ellipse);
            }
            else if (mShapeType == Rectangle || mShapeType == Square)
            {
                UBGraphicsRectItem* rect = dynamic_cast<UBGraphicsRectItem*>(instanciateCurrentShape());

                mBoundingRect = QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0);
                rect->setRect(mBoundingRect);

                mBoardView->scene()->addItem(rect);
            }
            else if (mShapeType == Line)
            {
                UBGraphicsLineItem* line = dynamic_cast<UBGraphicsLineItem*>(instanciateCurrentShape());

                line->setLine(QLineF(cursorPosition, cursorPosition));
                line->setStartPoint(cursorPosition);
                line->setEndPoint(cursorPosition);

                mBoundingRect = line->boundingRect();

                mBoardView->scene()->addItem(line);
            }
        }else{
            if (mShapeType == RegularPolygon)
            {
                UBGraphicsRegularPathItem* regularPathItem = dynamic_cast<UBGraphicsRegularPathItem*>(instanciateCurrentShape());
                regularPathItem->setStartPoint(cursorPosition);

                mBoundingRect = regularPathItem->boundingRect();

                mBoardView->scene()->addItem(regularPathItem);
            }
            else //Polygon
            {
                if(mShapeType == Pen){
                    if(mFirstClickForFreeHand){
                        UBGraphicsFreehandItem* pathItem = dynamic_cast<UBGraphicsFreehandItem*>(instanciateCurrentShape());

                        pathItem->addPoint(cursorPosition);

                        mFirstClickForFreeHand = false;
                        mBoundingRect = pathItem->boundingRect();
                        mBoardView->scene()->addItem(pathItem);                        
                    }
                }else{
                    UBGraphicsPathItem* pathItem = dynamic_cast<UBGraphicsPathItem*>(mCurrentShape);
                    if (mCurrentShape == NULL || pathItem == NULL)
                    {
                        pathItem = dynamic_cast<UBGraphicsPathItem*>(instanciateCurrentShape());
                        mBoardView->scene()->addItem(pathItem);
                    }
                    pathItem->addPoint(cursorPosition);

                    mBoundingRect = pathItem->boundingRect();

                    if (pathItem->isClosed())
                    {
                        mCurrentShape = NULL;
                    }
                }
            }
        }
    }

}

void UBShapeFactory::onMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event);
    mIsPress = false;

    UBGraphicsLineItem* line= dynamic_cast<UBGraphicsLineItem*>(mCurrentShape);
    if (line)
        if (line->startPoint() == line->endPoint())
             mBoardView->scene()->removeItem(line);

    if(mShapeType == Pen){
        mCurrentShape = NULL;
        mFirstClickForFreeHand = true;
    }
}

void UBShapeFactory::desactivate()
{
    mIsPress = false;
    mIsCreating = false;
    mCurrentShape = NULL;
    mShapeType = None;
}

bool UBShapeFactory::isShape(QGraphicsItem *item)
{
    return item->type() == UBGraphicsItemType::GraphicsShapeItemType
            || item->type() == UBGraphicsItemType::GraphicsPathItemType
            || item->type() == UBGraphicsItemType::GraphicsRegularPathItemType
            || item->type() == UBGraphicsItemType::GraphicsFreehandItemType;
}

void UBShapeFactory::setFillingStyle(Qt::BrushStyle brushStyle)
{
    //save the style and then update all selected elements
    mCurrentBrushStyle = brushStyle;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

        if(shape)
            shape->applyStyle(mCurrentBrushStyle);


        items.at(i)->update();
    }
}

void UBShapeFactory::setStrokeStyle(Qt::PenStyle penStyle)
{
    mCurrentPenStyle = penStyle;


    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

        if(shape)
        {
            shape->applyStyle(mCurrentPenStyle);
        }

        items.at(i)->update();
    }
}

void UBShapeFactory::setThickness(int thickness)
{
    mThickness = thickness;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

        if(shape){
            shape->setStrokeSize(mThickness);
        }

        items.at(i)->update();
    }
}

void UBShapeFactory::setStrokeColor(QColor color)
{
    mCurrentStrokeColor = color;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

        if(shape)
        {
            shape->applyStrokeColor(mCurrentStrokeColor);
        }

        items.at(i)->update();
    }
}

QColor UBShapeFactory::strokeColor()
{
    return mCurrentStrokeColor;
}


void UBShapeFactory::setFillingFirstColor(QColor color)
{
    mCurrentFillFirstColor = color;

    updateFillingPropertyOnSelectedItems();
}

void UBShapeFactory::setFillingSecondColor(QColor color)
{
    mCurrentFillSecondColor = color;

    updateFillingPropertyOnSelectedItems();
}

void UBShapeFactory::updateFillingPropertyOnSelectedItems()
{
    UBGraphicsScene* scene = mBoardView->scene();

     QList<QGraphicsItem*> items = scene->selectedItems();

     for(int i = 0; i < items.size(); i++)
     {
         UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

         if(shape)
         {
             if (mFillType != Gradient)
             {
                 if (shape->fillingProperty())
                    shape->fillingProperty()->setStyle(mCurrentBrushStyle);
                shape->applyFillColor(mCurrentFillFirstColor);
             }
             else
                setGradientFillingProperty(shape);

         }

         items.at(i)->update();

     }
}

QColor UBShapeFactory::fillFirstColor()
{
    return mCurrentFillFirstColor;
}

QColor UBShapeFactory::fillSecondColor()
{
    return mCurrentFillSecondColor;
}
