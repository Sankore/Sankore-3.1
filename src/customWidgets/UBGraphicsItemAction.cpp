/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBGraphicsItemAction.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "web/UBWebController.h"

UBGraphicsItemAction::UBGraphicsItemAction(eUBGraphicsItemLinkType linkType, QObject *parent) :
    QObject(parent)
{
    mLinkType = linkType;
}


UBGraphicsItemPlayAudioAction::UBGraphicsItemPlayAudioAction(QString audioFile, QObject *parent) :
    UBGraphicsItemAction(eLinkToAudio,parent)
{
    Q_ASSERT(audioFile.length() > 0);
    mAudioPath = audioFile;
}

void UBGraphicsItemPlayAudioAction::play()
{
    qDebug() << "playing the audio file " << mAudioPath;
}

QString UBGraphicsItemPlayAudioAction::save()
{
    return mAudioPath;
}


UBGraphicsItemMoveToPageAction::UBGraphicsItemMoveToPageAction(eUBGraphicsItemMovePageAction actionType, int page, QObject* parent) :
    UBGraphicsItemAction(eLinkToPage,parent)
{
    mActionType = actionType;
    mPage = page;
}

void UBGraphicsItemMoveToPageAction::play()
{
    UBBoardController* boardController = UBApplication::boardController;

    switch (mActionType) {
    case eMoveToFirstPage:
        boardController->firstScene();
        break;
    case eMoveToLastPage:
        boardController->lastScene();
        break;
    case eMoveToPreviousPage:
        boardController->previousScene();
        break;
    case eMoveToNextNextPage:
        boardController->nextScene();
        break;
    case eMoveToPage:
        boardController->setActiveDocumentScene(mPage);
        break;
    default:
        break;
    }
}

QString UBGraphicsItemMoveToPageAction::save()
{
    qDebug() << "UBGraphicsItemMoveToPageAction";
}


UBGraphicsItemLinkToWebPageAction::UBGraphicsItemLinkToWebPageAction(QString url, QObject *parent) :
    UBGraphicsItemAction(eLinkToWebUrl,parent)
{
    Q_ASSERT(url.length());
    mUrl = url;
}

void UBGraphicsItemLinkToWebPageAction::play()
{
    UBApplication::webController->loadUrl(QUrl(mUrl));
}

QString UBGraphicsItemLinkToWebPageAction::save()
{
    return mUrl;
}
