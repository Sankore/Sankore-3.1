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

#include "UBAbstractTeacherGuide.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

UBAbstractTeacherGuide::UBAbstractTeacherGuide(QWidget *parent):
    QStackedWidget(parent),
    mKeyboardActionFired(false)
{
    connect(UBApplication::boardController->controlView(), SIGNAL(clickOnBoard()), this, SLOT(showPresentationMode()));
    connectToStylusPalette();
}

void UBAbstractTeacherGuide::connectToStylusPalette()
{
    connect(UBApplication::mainWindow->actionPen, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionEraser, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionMarker, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionPointer, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionPlay, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionZoomIn, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionZoomOut, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionCapture, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionHand, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionLine, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionText, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionSelector, SIGNAL(triggered(bool)), this, SLOT(onTriggeredAction(bool)));
    connect(UBApplication::mainWindow->actionVirtualKeyboard, SIGNAL(triggered(bool)), this, SLOT(onTriggeredKeyboardAction(bool)));
}

void UBAbstractTeacherGuide::onTriggeredAction(bool checked)
{
    Q_UNUSED(checked);
    if(!mKeyboardActionFired)
        showPresentationMode();
    mKeyboardActionFired=false;
}

void UBAbstractTeacherGuide::onTriggeredKeyboardAction(bool checked)
{
    Q_UNUSED(checked);
    mKeyboardActionFired = true;
}
