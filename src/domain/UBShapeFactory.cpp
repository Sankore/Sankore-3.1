#include "UBShapeFactory.h"
#include "UBFillingProperty.h"
#include "UBGraphicsEllipseItem.h"
//#include "UBGraphicsPathItem.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "UBGraphicsScene.h"

UBShapeFactory::UBShapeFactory():
    mCurrentShape(NULL),
    mBoardView(NULL),
    mIsCreating(false),
    mIsPress(false),
    mIsRegularShape(true)
{

}


void UBShapeFactory::changeFillColor(bool ok)
{
    if(ok){
        UBGraphicsScene* scene = mBoardView->scene();

        QList<QGraphicsItem*> items = scene->selectedItems();

        for(int i = 0; i < items.size(); i++){
            UBGraphicsEllipseItem *ellipse = dynamic_cast<UBGraphicsEllipseItem*>(items.at(i));

            if(ellipse){
                ellipse->fillingProperty()->setFirstColor(QColor(128, 255, 100, 128));
                ellipse->update();
            }

        }
    }
}

void UBShapeFactory::init()
{
    mBoardView = UBApplication::boardController->controlView();

    connect(mBoardView, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseRelease(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));

}


UBShape* UBShapeFactory::instanciateShape()
{
    switch (mShapeType) {
    case Ellipse:
        mCurrentShape = new UBGraphicsEllipseItem();
        mCurrentShape->fillingProperty()->setFirstColor(Qt::red);
        break;
/*    case Polygon:
        mCurrentShape = new UBGraphicsPathItem();
        mCurrentShape->fillingProperty()->setFirstColor(Qt::blue);
        break;*/
    default:
        break;
    }

    return mCurrentShape;
}

void UBShapeFactory::createEllipse(bool create)
{
    if(create){
        mIsRegularShape = true;
        mIsCreating = true;
        mShapeType = Ellipse;

        //mCurrentShape = new UBGraphicsEllipseItem;
        //mCurrentShape->fillingProperty()->setFirstColor(Qt::red);
    }
}

void UBShapeFactory::createPolygon(bool create)
{
    if(create){
        mIsRegularShape = false;
        mIsCreating = true;
        mShapeType = Polygon;
        //mCurrentShape = new UBGraphicsPolygonItem();
        //mCurrentShape->fillingProperty()->setFirstColor(Qt::red);
    }
}

void UBShapeFactory::createCircle(bool create)
{
    if(create){
        mIsRegularShape = true;
        mIsCreating = true;
        UBGraphicsEllipseItem* ellipse = new UBGraphicsEllipseItem;

        ellipse->setAsCircle();
    }
}

void UBShapeFactory::onMouseMove(QMouseEvent *event)
{
    if(mIsCreating && mIsPress){
        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if(mIsRegularShape){
            UBGraphicsEllipseItem* shape = dynamic_cast<UBGraphicsEllipseItem*>(mCurrentShape);
             QRectF rect = shape->rect();

             qreal w = cursorPosition.x() - rect.x();
             qreal h = cursorPosition.y() - rect.y();

             shape->setRect(QRectF(rect.x(), rect.y(), w, h));
        }else{

        }
    }
}

void UBShapeFactory::onMousePress(QMouseEvent *event)
{
    if(mIsCreating){
        mIsPress = true;

        QPointF cursorPosition = mBoardView->mapToScene(event->pos());

        if(mIsRegularShape){
            //UBGraphicsEllipseItem* ellipse = dynamic_cast<UBGraphicsEllipseItem*>(mCurrentShape);
            UBGraphicsEllipseItem* ellipse = dynamic_cast<UBGraphicsEllipseItem*>(instanciateShape());

            ellipse->setRect(QRectF(cursorPosition.x(), cursorPosition.y(), 0, 0));

            mBoardView->scene()->addItem(ellipse);
        }else{/*
            UBGraphicsPathItem* pathItem = dynamic_cast<UBGraphicsPathItem*>(mCurrentShape);
            if (mCurrentShape == NULL || pathItem == NULL)
            {
                pathItem = dynamic_cast<UBGraphicsPathItem*>(instanciateShape());
                mBoardView->scene()->addItem(pathItem);

                QPainterPath path = pathItem->path();
                path.moveTo(cursorPosition);
                pathItem->setPath(path);
            }

            QPainterPath path = pathItem->path();
            path.lineTo(cursorPosition);
            pathItem->setPath(path);
*/
        }
    }

}

void UBShapeFactory::onMouseRelease(QMouseEvent *event)
{
    if(mIsCreating){
        if (mIsRegularShape) // Les regularShapes finissent d'être dessinées quand on relache la souris. Par contre les shapes "multi-points" (polygones) ne sont pas forcement finis (sauf si point pressé == premier point).
        {
        mIsCreating = false;
            mCurrentShape = NULL;
        }
        mIsPress = false;
    }
}
