/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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



#include "UBDrawingPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
const int UBDrawingPalette::PRESS_DURATION = 300;

UBDrawingPalette::UBDrawingPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;   

    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionPolygon;
    actions << UBApplication::mainWindow->actionChangeFillingColor;

    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    groupActions();

    adjustSizeAndPosition();
    initPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(pressed()), this, SLOT(drawingToolPressed()));
        connect(button, SIGNAL(pressed(int)), this, SLOT(updateCheckedId(int)));
        connect(button, SIGNAL(released()), this, SLOT(drawingToolReleased()));
    }

    setupSubPalettes(parent, orient);

}

void UBDrawingPalette::updateCheckedId(int i)
{
    mLastSelectedId = i;
}


void UBDrawingPalette::setupSubPalettes(QWidget* parent, Qt::Orientation orientation)
{

    if (orientation == Qt::Vertical)
    {
        //Sub Palette for ellipses and circles
        mSubPalettes.push_back(new UBEllipsePalette(parent, Qt::Horizontal));

        int x = this->pos().x() + this->width();

        mSubPalettes.back()->setCustomPosition(true);

        mSubPalettes.back()->move(x, this->pos().y());

//        mEllipsePalette->hide();
    }
    else
    {
        mSubPalettes.push_back(new UBEllipsePalette(parent));

        int x = this->pos().x() + this->width();

        mSubPalettes.back()->setCustomPosition(true);

        mSubPalettes.back()->move(x, this->pos().y());
    }

    initSubPalettesPosition(rect().topLeft());
}

void UBDrawingPalette::initPosition()
{
    if(UBSettings::settings()->appDrawingPaletteOrientationHorizontal->get().toBool())
    {
        QWidget* pParentW = parentWidget();
        if(NULL != pParentW)
        {
            mCustomPosition = true;
            QPoint pos;
            int parentWidth = pParentW->width();
            int parentHeight = pParentW->height();
            int posX = (parentWidth / 2) - (width() / 2);
            int posY = parentHeight - border() - height();

            pos.setX(posX);
            pos.setY(posY);
            move(pos);
        }
    }
}

UBDrawingPalette::~UBDrawingPalette()
{

}

void UBDrawingPalette::mousePressEvent(QMouseEvent *event)
{
    emit pressed(0);
    UBActionPalette::mousePressEvent(event);
}

void UBDrawingPalette::drawingToolPressed()
{

    UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Drawing);
    mActionButtonPressedTime = QTime::currentTime();

    mPendingActionButtonPressed = true;
    QTimer::singleShot(UBDrawingPalette::PRESS_DURATION, this, SLOT(drawingToolReleased()));
}

void UBDrawingPalette::drawingToolReleased()
{
    if (mPendingActionButtonPressed)
    {
        if( mActionButtonPressedTime.msecsTo(QTime::currentTime()) > UBDrawingPalette::PRESS_DURATION)
        {
            mSubPalettes.at(0)->togglePalette();
        }
        else
        {
            //mSubPalettes.at(0)->mainAction()->trigger();
        }

        mPendingActionButtonPressed = false;
    }
    else
    {

    }
}

void UBDrawingPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPoint oldPos = this->pos();

    UBActionPalette::mouseMoveEvent(event);

    QPoint delta = this->pos() - oldPos;
    updateSubPalettesPosition(delta);
}

void UBDrawingPalette::updateSubPalettesPosition(const QPoint& delta)
{
    for(int i = 0; i < mSubPalettes.size(); i++){
        QPoint newPos = mSubPalettes.at(i)->pos() + delta;

        mSubPalettes.at(i)->move(newPos);
    }

}

void UBDrawingPalette::initSubPalettesPosition(const QPointF& drawingPaletteTopLeft)
{
    Q_UNUSED(drawingPaletteTopLeft);
}


