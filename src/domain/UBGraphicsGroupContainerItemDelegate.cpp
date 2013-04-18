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



#include "UBGraphicsGroupContainerItemDelegate.h"

#include <QtGui>

#include "UBGraphicsScene.h"

#include "core/UBApplication.h"

#include "board/UBBoardPaletteManager.h"


#include "gui/UBResources.h"
#include "gui/UBCreateLinkPalette.h"

#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsGroupContainerItem.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"


#include "customWidgets/UBGraphicsItemAction.h"

#include "core/memcheck.h"

UBGraphicsGroupContainerItemDelegate::UBGraphicsGroupContainerItemDelegate(QGraphicsItem *pDelegated, QObject *parent) :
    UBGraphicsItemDelegate(pDelegated, parent, true, false, false), mDestroyGroupButton(0)

{
    //Wrapper function. Use it to set correct data() to QGraphicsItem as well
    setFlippable(false);
    setRotatable(false);
    setCanDuplicate(true);
}

UBGraphicsGroupContainerItem *UBGraphicsGroupContainerItemDelegate::delegated()
{
    return dynamic_cast<UBGraphicsGroupContainerItem*>(mDelegated);
}

void UBGraphicsGroupContainerItemDelegate::decorateMenu(QMenu *menu)
{
    mLockAction = menu->addAction(tr("Locked"), this, SLOT(lock(bool)));
    QIcon lockIcon;
    lockIcon.addPixmap(QPixmap(":/images/locked.svg"), QIcon::Normal, QIcon::On);
    lockIcon.addPixmap(QPixmap(":/images/unlocked.svg"), QIcon::Normal, QIcon::Off);
    mLockAction->setIcon(lockIcon);
    mLockAction->setCheckable(true);

    mShowOnDisplayAction = mMenu->addAction(tr("Visible on Extended Screen"), this, SLOT(showHide(bool)));
    mShowOnDisplayAction->setCheckable(true);

    QIcon showIcon;
    showIcon.addPixmap(QPixmap(":/images/eyeOpened.svg"), QIcon::Normal, QIcon::On);
    showIcon.addPixmap(QPixmap(":/images/eyeClosed.svg"), QIcon::Normal, QIcon::Off);
    mShowOnDisplayAction->setIcon(showIcon);

    mShowPanelToAddAnAction = menu->addAction(tr("Add an action"),this,SLOT(onAddActionClicked()));
}

//TODO claudio
// duplicated code UBGraphicsDelegateItem
void UBGraphicsGroupContainerItemDelegate::onAddActionClicked()
{
    UBCreateLinkPalette* linkPalette = UBApplication::boardController->paletteManager()->linkPalette();
    linkPalette->show();
    connect(linkPalette,SIGNAL(definedAction(UBGraphicsItemAction*)),this,SLOT(saveAction(UBGraphicsItemAction*)));
}

//TODO claudio
// duplicated code UBGraphicsDelegateItem
void UBGraphicsGroupContainerItemDelegate::saveAction(UBGraphicsItemAction* action)
{
    mAction = action;
    mMenu->removeAction(mShowPanelToAddAnAction);
    QString actionLabel;
    switch (mAction->linkType()) {
    case eLinkToAudio:
        actionLabel= tr("Remove link to audio");
        break;
    case eLinkToPage:
        actionLabel = tr("Remove link to page");
        break;
    case eLinkToWebUrl:
        actionLabel = tr("Remove link to web url");
    default:
        break;
    }

    mRemoveAnAction = mMenu->addAction(actionLabel,this,SLOT(onRemoveActionClicked()));
    mMenu->addAction(mRemoveAnAction);
}

//TODO claudio
// duplicated code UBGraphicsDelegateItem
void UBGraphicsGroupContainerItemDelegate::onRemoveActionClicked()
{
    if(mAction){
        mAction->actionRemoved();
        delete mAction;
        mAction = NULL;
    }
    mMenu->removeAction(mRemoveAnAction);
    mMenu->addAction(mShowPanelToAddAnAction);
}

void UBGraphicsGroupContainerItemDelegate::buildButtons()
{
    UBGraphicsItemDelegate::buildButtons();
}

bool UBGraphicsGroupContainerItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //TODO claudio
    // another chunk of duplicated code
    delegated()->deselectCurrentItem();
    UBStylusTool::Enum currentTool = (UBStylusTool::Enum)UBDrawingController::drawingController()->stylusTool();
    if(currentTool == UBStylusTool::Play){
        if(mAction)
            mAction->play();
        return true;
    }
    return false;
}

bool UBGraphicsGroupContainerItemDelegate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    return false;
}

bool UBGraphicsGroupContainerItemDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    return false;
}

void UBGraphicsGroupContainerItemDelegate::setAction(UBGraphicsItemAction* action)
{
    UBGraphicsItemDelegate::setAction(action);
}
