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

const int UBDrawingPalette::PRESS_DURATION = 1;

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

    connectButtons();

    setupSubPalettes(parent, orient);

    connectDrawingActions();
    connectSubPalettes();
}


UBDrawingPalette::~UBDrawingPalette()
{

}


void UBDrawingPalette::connectDrawingActions()
{
    connect(UBApplication::mainWindow->actionEllipse, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createEllipse(bool)));
    connect(UBApplication::mainWindow->actionCircle, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createCircle(bool)));
    connect(UBApplication::mainWindow->actionRectangle, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createRectangle(bool)));
    connect(UBApplication::mainWindow->actionSquare, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createSquare(bool)));
    connect(UBApplication::mainWindow->actionSmartLine, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createLine(bool)));
    connect(UBApplication::mainWindow->actionSmartPen, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createPen(bool)));
    connect(UBApplication::mainWindow->actionPolygon, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createPolygon(bool)));
    connect(UBApplication::mainWindow->actionChangeFillingColor, SIGNAL(triggered(bool)), this, SLOT(changeProperty(bool)));

}

void UBDrawingPalette::connectButtons()
{
    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(pressed()), button, SLOT(toggle()));
        connect(button, SIGNAL(pressed()), this, SLOT(drawingToolPressed()));
        connect(button, SIGNAL(released()), this, SLOT(drawingToolReleased()));
    }
}

void UBDrawingPalette::setupSubPalettes(QWidget* parent, Qt::Orientation orientation)
{
    Qt::Orientation orientationSubPalette = (orientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;

    //Sub Palette for ellipses and circles
    UBShapesPalette * shapesPalette = new UBShapesPalette(parent, orientationSubPalette);
    shapesPalette->setCustomPosition(true);
    mSubPalettes[ShapesAction] = shapesPalette;

    initSubPalettesPosition();
}

void UBDrawingPalette::connectSubPalettes()
{
    connect(mSubPalettes[ShapesAction], SIGNAL(newMainAction()), this, SLOT(updateActions()));
}

void UBDrawingPalette::updateActions()
{
    QList<QAction*> actions;

    actions << mSubPalettes[ShapesAction]->mainAction();
    actions << UBApplication::mainWindow->actionPolygon;
    actions << UBApplication::mainWindow->actionChangeFillingColor;

    clearLayout();

    setActions(actions);
    setButtonIconSize(QSize(42, 42));
    groupActions();

    connectButtons();
}

void UBDrawingPalette::initPosition()
{
    // Rem : positions would be very different if palette were horizontal...

    mCustomPosition = true;

    int x = 0;
    int y = 0;

    // By default, place the drawingPalette next to the Library (RightPalette)
    if (UBApplication::boardController
            && UBApplication::boardController->paletteManager()
            && UBApplication::boardController->paletteManager()->rightPalette())
    {
        x = UBApplication::boardController->paletteManager()->rightPalette()->pos().x();
        x -= this->width();
    }

    // By default, center vertically the drawingPalette in her parent
    y = ( parentWidget()->height() - this->height() ) / 2;

    moveInsideParent(QPoint(x, y));

    initSubPalettesPosition(); // place the subPalettes next to the palette.
}

void UBDrawingPalette::initSubPalettesPosition()
{
    // For each button of palette ...
    for(int i = 0; i < mButtons.count(); ++i)
    {
        if (mSubPalettes.contains(i)) // ... if there is a subPalette associated to this button ...
        {
            // Place the subPalette :
            UBAbstractSubPalette* subPalette = mSubPalettes[i];
            UBActionPaletteButton* button = mButtons.at(i);

            // Depending on position of palette,
            int x = this->x() + this->width(); // place subPalette on the right of the palette ...
            if (this->x() > parentWidget()->width()/2)
            {
                x = this->x() - subPalette->width(); // ... or on the left on the palette.
            }

            // Align vertically the center of subPalette to center of button :
            int y = this->y() + button->y() + button->height()/2 - subPalette->height()/2;

            subPalette->move(x, y);
        }
    }
}

void UBDrawingPalette::drawingToolPressed()
{
    mLastSelectedId = mButtonGroup->checkedId();

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
            if (mSubPalettes.find(mLastSelectedId) != mSubPalettes.end())
            {
                initSubPalettesPosition();
                mSubPalettes[mLastSelectedId]->togglePalette();
            }
        }
        else
        {
            if (mSubPalettes.find(mLastSelectedId) != mSubPalettes.end())
                mSubPalettes[mLastSelectedId]->mainAction()->trigger();
            else
                mActions.at(mLastSelectedId)->trigger();

        }

        mPendingActionButtonPressed = false;
    }
    else
    {

    }
}

void UBDrawingPalette::changeVisibility(bool checked)
{
    if (checked)
        setVisible(true);
    else
    {
        setVisible(false);

        foreach(UBAbstractSubPalette* subPalette, mSubPalettes.values())
        {
            subPalette->hide();
        }
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
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes.values()) {
        QPoint newPos = subPalette->pos() + delta;
        subPalette->move(newPos);
    }
}

void UBDrawingPalette::stackUnder(QWidget * w)
{
    UBActionPalette::stackUnder(w);

    // For all subpalettes :
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes.values()) {
        subPalette->stackUnder(w);
    }
}

void UBDrawingPalette::raise()
{
    UBActionPalette::raise();

    // For all subpalettes :
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes.values()) {
        subPalette->raise();
    }
}

void UBDrawingPalette::changeProperty(bool ok)
{
    if(ok){
        UBApplication::boardController->shapeFactory().setThickness(15);
    }
}
