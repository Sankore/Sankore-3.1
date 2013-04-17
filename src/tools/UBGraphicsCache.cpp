/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include <QDebug>

#include "UBGraphicsCache.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

QMap<UBGraphicsScene*, UBGraphicsCache*> UBGraphicsCache::sInstances;

UBGraphicsCache* UBGraphicsCache::instance(UBGraphicsScene *scene)
{
    if (!sInstances.contains(scene))
        sInstances.insert(scene, new UBGraphicsCache(scene));
    return sInstances[scene];
}

UBGraphicsCache::UBGraphicsCache(UBGraphicsScene *scene) : QGraphicsRectItem()
  , mMaskColor(Qt::black)
  , mMaskShape(eMaskShape_Circle)
  , mDrawMask(false)
  , mScene(scene)
  , mShouldDrawAtHoverEnter(false)
{
    setMode(static_cast<int>(OnClick)); 

    mHoleSize = UBSettings::settings()->casheLastHoleSize->get().toSize();
    
    QColor cacheColor;
    QStringList colors = UBSettings::settings()->cacheColor->get().toString().split(" ", QString::SkipEmptyParts);
    if (colors.count())
    {
        if (3 == colors.count())
            cacheColor = QColor(colors[0].toInt(),colors[1].toInt(),colors[2].toInt());
        if (4 == colors.count())
            cacheColor = QColor(colors[0].toInt(),colors[1].toInt(),colors[2].toInt(), colors[3].toInt());
    }

    if (cacheColor.isValid())
        mMaskColor = cacheColor;
    else
        mMaskColor = QColor(0,0,0,190);

    // Get the board size and pass it to the shape
    QRect boardRect = UBApplication::boardController->displayView()->rect();
    setRect(-15*boardRect.width(), -15*boardRect.height(), 30*boardRect.width(), 30*boardRect.height());
    setData(Qt::UserRole, QVariant("Cache"));
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::Cache)); //Necessary to set if we want z value to be assigned correctly

    QPixmap cursorImage(1,1);
    QPixmap cursorMask(1,1);
    QPainter cursorPainter(&cursorImage);
    QPainter cursorMaskPainter(&cursorMask);
    cursorPainter.setBackground(Qt::transparent);
    cursorMaskPainter.setBackground(Qt::transparent);
    cursorPainter.fillRect(cursorImage.rect(), Qt::color0);
    cursorPainter.end();

    mCursorForHole = QCursor(cursorImage, cursorMask);
}

UBGraphicsCache::~UBGraphicsCache()
{
    sInstances.remove(mScene);
}

UBItem* UBGraphicsCache::deepCopy() const
{
    UBGraphicsCache* copy = new UBGraphicsCache(mScene);

    copyItemParameters(copy);

    // TODO UB 4.7 ... complete all members ?

    return copy;
}

void UBGraphicsCache::copyItemParameters(UBItem *copy) const
{
    UBGraphicsCache *cp = dynamic_cast<UBGraphicsCache*>(copy);
    if (cp)
    {
        cp->setPos(this->pos());
        cp->setRect(this->rect());
        cp->setTransform(this->transform());
    }
}

QColor UBGraphicsCache::maskColor()
{
    return mMaskColor;
}

void UBGraphicsCache::setMaskColor(QColor color)
{
    mMaskColor = color;
    update();
}

eMaskShape UBGraphicsCache::maskshape()
{
    return mMaskShape;
}

void UBGraphicsCache::setMaskShape(eMaskShape shape)
{
    mMaskShape = shape;
    update();
}

void UBGraphicsCache::init()
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void UBGraphicsCache::setMode(int mode)
{
    mCurrentMode = static_cast<eMode>(mode);

    if (OnClick == mCurrentMode)
        setAcceptHoverEvents(false);
    if (Persistent == mCurrentMode)
        setAcceptHoverEvents(true);

    drawHole(false);
    mShouldDrawAtHoverEnter = false;
}

void UBGraphicsCache::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);


    QColor maskColor;
    maskColor = mMaskColor;
    if(widget != UBApplication::boardController->controlView()->viewport())
        maskColor.setAlpha(255);
        
 
    painter->setBrush(maskColor);
    painter->setPen(maskColor);
    
    // Draw the hole
    QPainterPath path;
    path.addRect(rect());

    if(mDrawMask)
    {
        if(eMaskShape_Circle == mMaskShape)
        {
            path.addEllipse(mHolePos, mHoleSize.width()/2, mHoleSize.height()/2);
        }
        else if(eMaskShap_Rectangle == mMaskShape)
        {
            path.addRect(mHolePos.x() - mHoleSize.width()/2, mHolePos.y() - mHoleSize.height()/2, mHoleSize.width(), mHoleSize.height());
        }
        path.setFillRule(Qt::OddEvenFill);
    }

    painter->drawPath(path);
}

void UBGraphicsCache::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (OnClick == mCurrentMode)
    {
        drawHole(true);
    }
    
    if (Persistent == mCurrentMode)
        drawHole(!mDrawMask);

    setHolePos(event->pos());
}

void UBGraphicsCache::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (mShouldDrawAtHoverEnter)
        drawHole(true);

    mShouldDrawAtHoverEnter = false;

    update(updateRect(event->pos()));
}

void UBGraphicsCache::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mShouldDrawAtHoverEnter = mDrawMask;
    drawHole(false);
    update(updateRect(event->pos()));
}

void UBGraphicsCache::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    setHolePos(event->pos());
}

void UBGraphicsCache::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    setHolePos(event->pos());
}

void UBGraphicsCache::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    if (OnClick == mCurrentMode)
        drawHole(false);

    // Note: if refresh issues occure, replace the following 3 lines by: update();
    update(updateRect(event->pos()));
    mOldShapeSize = mHoleSize;
    mOldShapePos = event->pos();
}

void UBGraphicsCache::drawHole(bool draw)
{
    mDrawMask = draw;
    if (mDrawMask)
    {
        mSavedCursor = cursor();
        setCursor(mCursorForHole);
    }
    else
        setCursor(mSavedCursor);
        
}

void UBGraphicsCache::setHolePos(QPointF pos)
{
    mHolePos = pos;

    // Note: if refresh issues occure, replace the following 3 lines by: update();
    update(updateRect(pos));
    mOldShapeSize = mHoleSize;
    mOldShapePos = pos;
}

int UBGraphicsCache::holeWidth()
{
    return mHoleSize.width();
}

int UBGraphicsCache::holeHeight()
{
    return mHoleSize.height();
}

void UBGraphicsCache::setHoleWidth(int width)
{
    mHoleSize.setWidth(width);   
    UBSettings::settings()->casheLastHoleSize->set(mHoleSize);
}

void UBGraphicsCache::setHoleHeight(int height)
{
    mHoleSize.setHeight(height);
    UBSettings::settings()->casheLastHoleSize->set(mHoleSize);
}

void UBGraphicsCache::setHoleSize(QSize size)
{
    mHoleSize = size/UBApplication::boardController->controlView()->viewportTransform().m11();
    UBSettings::settings()->casheLastHoleSize->set(mHoleSize);
}

QRectF UBGraphicsCache::updateRect(QPointF currentPoint)
{
    QRectF r;
    int x;
    int y;

    x = qMin(currentPoint.x() - mHoleSize.width()/2, mOldShapePos.x() - mOldShapeSize.width()/2);
    y = qMin(currentPoint.y() - mHoleSize.height()/2, mOldShapePos.y() - mOldShapeSize.height()/2);
    r = QRect(  x,
                y,
                qAbs(currentPoint.x() - mOldShapePos.x()) + mHoleSize.width(),
                qAbs(currentPoint.y() - mOldShapePos.y()) + mHoleSize.height());
    return r;
}
