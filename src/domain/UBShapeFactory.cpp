#include "UBShapeFactory.h"
#include "UBGraphicsEllipseItem.h"
#include "UBGraphicsRectItem.h"
#include "UBGraphicsLineItem.h"
#include "UBGraphicsRegularPathItem.h"
#include "UBGraphicsPathItem.h"

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
    mCurrentStrokeColor(Qt::black),
    mCurrentFillFirstColor(Qt::transparent),
    mDrawingController(NULL),
    mCurrentBrushStyle(Qt::SolidPattern),
    mCurrentPenStyle(Qt::SolidLine),
    mThickness(3)
{

}


void UBShapeFactory::changeFillColor(bool ok)
{
    if(ok){

        this->desactivate();
        UBGraphicsScene* scene = mBoardView->scene();

        QList<QGraphicsItem*> items = scene->selectedItems();

        for(int i = 0; i < items.size(); i++){
            UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

            if(shape){
                shape->fillingProperty()->setColor(QColor(128, 255, 100, 128));
                items.at(i)->update();
            }
        }
    }
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
        mCurrentShape = new UBGraphicsEllipseItem();
        break;
    }
    case Circle:
    {
        UBGraphicsEllipseItem * ellipse = new UBGraphicsEllipseItem();
        ellipse->setAsCircle();
        mCurrentShape = ellipse;
        break;
    }
    case Rectangle:
    {
        UBGraphicsRectItem* rect = new UBGraphicsRectItem();
        rect->setAsRectangle();
        mCurrentShape = rect;
        break;
    }
    case Square:
    {
        UBGraphicsRectItem* square = new UBGraphicsRectItem();
        square->setAsSquare();
        mCurrentShape = square;
        break;
    }
    case Line:
    {
        UBGraphicsLineItem* line = new UBGraphicsLineItem();
        mCurrentShape = line;
        break;
    }
    case Polygon:
    {
        UBGraphicsPathItem * pathItem = new UBGraphicsPathItem();
        mCurrentShape = pathItem;
        break;
    }
    case RegularPolygon:
    {
        UBGraphicsRegularPathItem* regularPathItem = new UBGraphicsRegularPathItem(mNVertices);
        mCurrentShape = regularPathItem;
        break;
    }
    default:
        break;
    }

    mCurrentShape->applyStyle(mCurrentBrushStyle, mCurrentPenStyle);
    mCurrentShape->applyFillColor(mCurrentFillFirstColor);
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
        mIsRegularShape = true;
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

                shape->setRect(QRectF(rect.x(), rect.y(), w, h));
            }
            else if (mShapeType == Rectangle || mShapeType == Square)
            {
                UBGraphicsRectItem* shape = dynamic_cast<UBGraphicsRectItem*>(mCurrentShape);
                QRectF rect = shape->rect();

                qreal w = cursorPosition.x() - rect.x();
                qreal h = cursorPosition.y() - rect.y();

                shape->setRect(QRectF(rect.x(), rect.y(), w, h));
            }
            else if (mShapeType == Line)
            {
                UBGraphicsLineItem* line= dynamic_cast<UBGraphicsLineItem*>(mCurrentShape);
                QLineF newLine(line->startPoint(), cursorPosition);
                line->setLine(newLine);
                line->setEndPoint(cursorPosition);
            }
        }
        else
        {
            if (mShapeType == RegularPolygon)
            {
                UBGraphicsRegularPathItem* regularPathItem = dynamic_cast<UBGraphicsRegularPathItem*>(mCurrentShape);
                regularPathItem->updatePath(cursorPosition);
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

                ellipse->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

                mBoardView->scene()->addItem(ellipse);
            }
            else if (mShapeType == Rectangle || mShapeType == Square)
            {
                UBGraphicsRectItem* rect = dynamic_cast<UBGraphicsRectItem*>(instanciateCurrentShape());

                rect->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

                mBoardView->scene()->addItem(rect);
            }
            else if (mShapeType == Line)
            {
                UBGraphicsLineItem* line = dynamic_cast<UBGraphicsLineItem*>(instanciateCurrentShape());

                line->setLine(QLineF(cursorPosition, cursorPosition));
                line->setStartPoint(cursorPosition);
                line->setEndPoint(cursorPosition);

                mBoardView->scene()->addItem(line);
            }
        }else{
            if (mShapeType == RegularPolygon)
            {
                UBGraphicsRegularPathItem* regularPathItem = dynamic_cast<UBGraphicsRegularPathItem*>(instanciateCurrentShape());
                regularPathItem->setStartPoint(cursorPosition);

                mBoardView->scene()->addItem(regularPathItem);
            }
            else //Polygon
            {
                UBGraphicsPathItem* pathItem = dynamic_cast<UBGraphicsPathItem*>(mCurrentShape);
                if (mCurrentShape == NULL || pathItem == NULL)
                {
                    pathItem = dynamic_cast<UBGraphicsPathItem*>(instanciateCurrentShape());
                    mBoardView->scene()->addItem(pathItem);
                }
                pathItem->addPoint(cursorPosition);

                if (pathItem->isClosed())
                {
                    mCurrentShape = NULL;
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
}

void UBShapeFactory::desactivate()
{
    mIsPress = false;
    mIsCreating = false;
    mCurrentShape = NULL;
}

bool UBShapeFactory::isShape(QGraphicsItem *item)
{
    return item->type() == UBGraphicsEllipseItem ::Type
            || item->type() == UBGraphicsPathItem::Type;
}

void UBShapeFactory::setFillingStyle(Qt::BrushStyle brushStyle)
{
    //save the style and then update all selected elements
    mCurrentBrushStyle = brushStyle;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

        if(shape){
            shape->applyStyle(mCurrentBrushStyle, mCurrentPenStyle);
        }

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

        if(shape){
            shape->applyStyle(mCurrentBrushStyle, mCurrentPenStyle);
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

        if(shape){
            shape->applyStrokeColor(mCurrentStrokeColor);
        }

        items.at(i)->update();
    }
}

QColor UBShapeFactory::strokeColor()
{
    return mCurrentStrokeColor;
}


void UBShapeFactory::setFillingColor(QColor color)
{
    mCurrentFillFirstColor = color;

    UBGraphicsScene* scene = mBoardView->scene();

    QList<QGraphicsItem*> items = scene->selectedItems();

    for(int i = 0; i < items.size(); i++){
        UBShape * shape = dynamic_cast<UBShape*>(items.at(i));

        if(shape){
            shape->applyFillColor(mCurrentFillFirstColor);
        }

        items.at(i)->update();
    }
}

QColor UBShapeFactory::fillFirstColor()
{
    return mCurrentFillFirstColor;
}
