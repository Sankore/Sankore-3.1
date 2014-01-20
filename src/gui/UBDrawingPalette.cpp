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

#include "UBDrawingPolygonPalette.h"
#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include <QActionGroup>

#include "UBDrawingStrokePropertiesPalette.h"
#include "UBDrawingFillPropertiesPalette.h"

const int UBDrawingPalette::PRESS_DURATION = 1;

UBDrawingPalette::UBDrawingPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
/*
    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionEllipse;
    actions << UBApplication::mainWindow->actionPolygon;
    actions << UBApplication::mainWindow->actionChangeFillingColor;
//    actions << mSubPalettes[0]->actions()[0];
//    actions << mSubPalettes[1]->actions()[0];


    setActions(actions);
    setButtonIconSize(QSize(42, 42));

    groupActions();

    adjustSizeAndPosition();
    initPosition();

    connectButtons();

    setupSubPalettes(parent);

    connectDrawingActions();
    connectSubPalettes();
*/
    monInit();
}

void UBDrawingPalette::monInit()
{
    // Créer les subPalettes
    // Créer les boutons de cette Palette
    // Pour certaines SubPalette, une de ses Action est attribué au bouton de cette Palette

    UBActionPaletteButton * btnSubPaletteShape = addButtonSubPalette(new UBShapesPalette(Qt::Horizontal, parentWidget()));

    UBActionPaletteButton * btnSubPalettePolygon = addButtonSubPalette(new UBDrawingPolygonPalette(Qt::Horizontal, parentWidget()));

    UBActionPaletteButton * btnSubPaletteStrokeProperties = addButtonSubPalette(new UBDrawingStrokePropertiesPalette(Qt::Horizontal, parentWidget()), UBApplication::mainWindow->actionStrokeProperties);

    UBActionPaletteButton * btnSubPaletteFillProperties = addButtonSubPalette(new UBDrawingFillPropertiesPalette(Qt::Horizontal, parentWidget()), UBApplication::mainWindow->actionFillProperties);

    UBActionPaletteButton * btnPaintBucket = addActionButton(UBApplication::mainWindow->actionChangeFillingColor);

    // Certains de ces boutons sont groupés
    mButtonGroup = new QButtonGroup(this);
    mButtonGroup->addButton(btnSubPaletteShape);
    mButtonGroup->addButton(btnSubPalettePolygon);
    mButtonGroup->addButton(btnPaintBucket);

    initSubPalettesPosition();

    adjustSizeAndPosition();
}

UBActionPaletteButton * UBDrawingPalette::addButtonSubPalette(UBAbstractSubPalette * subPalette, QAction* action)
{
    UBActionPaletteButton * button = 0;
    if (action == 0 && subPalette->actions().size()>0)
    {
        action = subPalette->actions()[0];
    }

    if (action != 0)
    {
        button = new UBActionPaletteButton(action, this);
        //button->setIconSize(mButtonSize);
        button->setToolButtonStyle(mToolButtonStyle);
        subPalette->setActionPaletteButtonParent(button);
        mSubPalettes2[button] = subPalette;
        layout()->addWidget(button);
        layout()->setAlignment(button,Qt::AlignHCenter | Qt::AlignVCenter);
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
    return button;
}

UBActionPaletteButton * UBDrawingPalette::addActionButton(QAction * action)
{
    UBActionPaletteButton * actionButton = new UBActionPaletteButton(action);

    layout()->addWidget(actionButton);
    layout()->setAlignment(actionButton, Qt::AlignHCenter | Qt::AlignVCenter);

    connect(action, SIGNAL(triggered()), this, SLOT(buttonClicked()));

    return actionButton;
}



UBDrawingPalette::~UBDrawingPalette()
{

}


void UBDrawingPalette::connectDrawingActions()
{/*
    // Devrait être dans la subPalette UBShapePalette :
    connect(UBApplication::mainWindow->actionEllipse, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createEllipse(bool)));
    connect(UBApplication::mainWindow->actionCircle, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createCircle(bool)));
    connect(UBApplication::mainWindow->actionRectangle, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createRectangle(bool)));
    connect(UBApplication::mainWindow->actionSquare, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createSquare(bool)));
    connect(UBApplication::mainWindow->actionSmartLine, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createLine(bool)));
    connect(UBApplication::mainWindow->actionSmartPen, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createPen(bool)));
    connect(UBApplication::mainWindow->actionPolygon, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(createPolygon(bool)));

    connect(UBApplication::mainWindow->actionChangeFillingColor, SIGNAL(triggered(bool)), this, SLOT(changeProperty(bool)));
    connect(UBApplication::mainWindow->actionChangeFillingColor, SIGNAL(triggered(bool)), &UBApplication::boardController->shapeFactory(), SLOT(changeFillColor(bool)));
*/

}

void UBDrawingPalette::connectButtons()
{
    foreach(const UBActionPaletteButton* button, mButtons)
    {
        //connect(button, SIGNAL(pressed()), button, SLOT(toggle())); // Cette ligne empeche le Exclusive mode !
        //connect(button, SIGNAL(pressed()), this, SLOT(drawingToolPressed()));
        //connect(button, SIGNAL(released()), this, SLOT(drawingToolReleased()));
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
}

void UBDrawingPalette::buttonClicked()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>( sender() );
    if (button && mSubPalettes2.contains(button))
    {
        initSubPalettesPosition();
        mSubPalettes2.value(button)->togglePalette();
    }

    // Terminer tout dessin en cours de tracé (polygone)
    //UBApplication::boardController->shapeFactory().desactivate();

    //getButtonFromAction();

    //UBActionPalette::buttonClicked();
}

void UBDrawingPalette::setupSubPalettes(QWidget* parent)
{
    Qt::Orientation orientationSubPalette = (mOrientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;

    //Sub Palette for ellipses and circles
    UBShapesPalette * shapesPalette = new UBShapesPalette(orientationSubPalette, parent);
    shapesPalette->setCustomPosition(true);
    mSubPalettes[ShapesAction] = shapesPalette;



    //shapesPalette->setActionParent(mActions[0]);

    UBDrawingPolygonPalette * drawingPolygonPalette = new UBDrawingPolygonPalette(orientationSubPalette, parent);
    drawingPolygonPalette->setCustomPosition(true);
    mSubPalettes[PolygonAction] = drawingPolygonPalette;

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
    // Rem : positions would be very different if drawingPalette were horizontal...

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
/*
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

            int thisY = this->y();
            QPoint pos = button->pos();
            subPalette->move(x, y);
        }
    }
*/
    foreach(UBActionPaletteButton* button, mSubPalettes2.keys())
    {
        UBAbstractSubPalette* subPalette = mSubPalettes2.value(button);

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
/*
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
*/
void UBDrawingPalette::setVisible(bool checked)
{
    UBActionPalette::setVisible(checked);

    if ( ! checked)
    {
        foreach(UBAbstractSubPalette* subPalette, mSubPalettes2.values())
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
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes2.values()) {
        QPoint newPos = subPalette->pos() + delta;
        subPalette->move(newPos);
    }
}

void UBDrawingPalette::stackUnder(QWidget * w)
{
    UBActionPalette::stackUnder(w);

    // For all subpalettes :
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes2.values()) {
        subPalette->stackUnder(w);
    }
}

void UBDrawingPalette::raise()
{
    UBActionPalette::raise();

    // For all subpalettes :
    foreach (UBAbstractSubPalette* subPalette, mSubPalettes2.values()) {
        subPalette->raise();
    }
}

void UBDrawingPalette::changeProperty(bool ok)
{
    if(ok){
        UBApplication::boardController->shapeFactory().setThickness(15);
    }
}
