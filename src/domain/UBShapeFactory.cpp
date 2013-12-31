#include "UBShapeFactory.h"
#include "UBFillingProperty.h"
#include "UBGraphicsEllipseItem.h"

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

void changeFillColor(UBShape *shape, const QColor &newColor) const
{
    shape->fillingProperty()->setFirstColor(newColor);
}

void UBShapeFactory::init()
{
    mBoardView = UBApplication::boardController->controlView();

    connect(mBoardView, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(onMouseMove(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(onMouseRelease(QMouseEvent*)));
    connect(mBoardView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(onMousePress(QMouseEvent*)));

}

void UBShapeFactory::createEllipse(bool create)
{
    if(create){
        mIsRegularShape = true;
        mIsCreating = true;
        mCurrentShape = new UBGraphicsEllipseItem;

        mCurrentShape->fillingProperty()->setFirstColor(Qt::red);
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

        if(mIsRegularShape){
            UBGraphicsEllipseItem* ellipse = dynamic_cast<UBGraphicsEllipseItem*>(mCurrentShape);

            QPointF cursorPosition = mBoardView->mapToScene(event->pos());

            ellipse->setRect(cursorPosition.x(), cursorPosition.y(), 0, 0);

            mBoardView->scene()->addItem(ellipse);
        }else{

        }
    }

}

void UBShapeFactory::onMouseRelease(QMouseEvent *event)
{
    if(mIsCreating){
        mIsCreating = false;
        mIsPress = false;
        mCurrentShape = NULL;
    }
}
