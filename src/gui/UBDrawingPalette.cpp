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
        connect(button, SIGNAL(released()), this, SLOT(drawingToolReleased()));
    }

    setupSubPalettes(orient);
}

void UBDrawingPalette::setupSubPalettes(Qt::Orientation orientation)
{
    //Sub Palette for ellipses and circles
    if (orientation == Qt::Vertical)
        mEllipsePalette = new UBEllipsePalette(Qt::Horizontal);
    else
        mEllipsePalette = new UBEllipsePalette();
}

void UBDrawingPalette::initPosition()
{
    if(!UBSettings::settings()->appDrawingPaletteOrientationHorizontal->get().toBool())
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
            moveInsideParent(pos);
        }
    }
}

void UBDrawingPalette::toggleEllipsePalette()
{
    mEllipsePalette->show();
}

UBDrawingPalette::~UBDrawingPalette()
{

}

void UBDrawingPalette::drawingToolPressed()
{
    emit drawingToolPressed(mButtonGroup->checkedId());
}

void UBDrawingPalette::drawingToolReleased()
{
    emit drawingToolReleased(mButtonGroup->checkedId());
}

void UBDrawingPalette::mouseMoveEvent(QMouseEvent *event)
{
    UBActionPalette::mouseMoveEvent(event);
    updateSubPalettesPosition(event);
}

void UBDrawingPalette::updateSubPalettesPosition(QMouseEvent *event)
{
    Q_UNUSED(event);
}


