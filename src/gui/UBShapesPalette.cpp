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



#include "UBShapesPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

UBShapesPalette::UBShapesPalette(QWidget *parent, Qt::Orientation orient)
    : UBAbstractSubPalette(parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionCircle;
    actions << UBApplication::mainWindow->actionRectangle;
    actions << UBApplication::mainWindow->actionSquare;
    actions << UBApplication::mainWindow->actionSmartLine;
    actions << UBApplication::mainWindow->actionSmartPen;


    setActions(actions);
    setButtonIconSize(QSize(28, 28));

    groupActions();

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(released()), this, SLOT(shapesToolReleased()));
    }

    hide();

}

UBShapesPalette::UBShapesPalette(Qt::Orientation orient, QWidget *parent )
    : UBAbstractSubPalette(parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionCircle;
    actions << UBApplication::mainWindow->actionRectangle;
    actions << UBApplication::mainWindow->actionSquare;
    actions << UBApplication::mainWindow->actionSmartLine;
    actions << UBApplication::mainWindow->actionSmartPen;

    setActions(actions);
    setButtonIconSize(QSize(28, 28));

    groupActions();

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(released()), this, SLOT(shapesToolReleased()));
    }

    hide();
}

UBShapesPalette::~UBShapesPalette()
{

}

void UBShapesPalette::shapesToolPressed()
{

}

void UBShapesPalette::shapesToolReleased()
{
    mMainAction = mButtonGroup->checkedId();

    hide();

    emit newMainAction();
}

void UBShapesPalette::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void UBShapesPalette::togglePalette()
{
    show();
}
