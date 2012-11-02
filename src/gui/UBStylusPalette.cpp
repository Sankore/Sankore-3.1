/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBStylusPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"

#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

UBStylusPalette::UBStylusPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction *> penActions;

    penActions << UBApplication::mainWindow->actionPen;
    penActions << UBApplication::mainWindow->actionPenLine;
    addActions(penActions);

    QList<QAction *> markerActions;
    markerActions << UBApplication::mainWindow->actionMarker;
    markerActions << UBApplication::mainWindow->actionMarkerLine;
    addActions(markerActions);

    addAction(UBApplication::mainWindow->actionEraser);

    addAction(UBApplication::mainWindow->actionSelector);
    addAction(UBApplication::mainWindow->actionPlay);

    addAction(UBApplication::mainWindow->actionHand);
    addAction(UBApplication::mainWindow->actionZoomIn);
    addAction(UBApplication::mainWindow->actionZoomOut);

    addAction(UBApplication::mainWindow->actionPointer);
    addAction(UBApplication::mainWindow->actionText);
    addAction(UBApplication::mainWindow->actionCapture);

    if(UBPlatformUtils::hasVirtualKeyboard())
        addAction(UBApplication::mainWindow->actionVirtualKeyboard, false);


    setButtonIconSize(QSize(42, 42));

    if (mButtonGroup)
        connect(mButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(buttonGroupClicked(int)));

    adjustSizeAndPosition();

    initPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(doubleClicked()), this, SLOT(stylusToolDoubleClicked()));
    }

}

void UBStylusPalette::initPosition()
{
    if(!UBSettings::settings()->appToolBarOrientationVertical->get().toBool())
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

UBStylusPalette::~UBStylusPalette()
{

}

void UBStylusPalette::stylusToolDoubleClicked()
{
    emit stylusToolDoubleClicked(mButtonGroup->checkedId());
}


