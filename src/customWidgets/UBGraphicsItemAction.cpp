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
#include "core/UBPersistenceManager.h"
#include "board/UBBoardController.h"
#include "web/UBWebController.h"
#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"



UBGraphicsItemAction::UBGraphicsItemAction(eUBGraphicsItemLinkType linkType, QObject *parent) :
    QObject(parent)
{
    mLinkType = linkType;
}


UBGraphicsItemPlayAudioAction::UBGraphicsItemPlayAudioAction(QString audioFile, bool isNewAction, QObject *parent) :
    UBGraphicsItemAction(eLinkToAudio,parent)
  , mMediaObject(0)
  , mIsLoading(true)
{
    Q_ASSERT(audioFile.length() > 0);
    QString extension = QFileInfo(audioFile).completeSuffix();
    if(isNewAction){
        QString destDir = UBApplication::documentController->selectedDocument()->persistencePath() + "/" + UBPersistenceManager::audioDirectory;
        QString destFile = destDir + "/" + QUuid::createUuid().toString() + "." + extension;
        if(!QDir(destDir).exists())
            QDir(UBApplication::documentController->selectedDocument()->persistencePath()).mkdir(destDir);
        QFile(audioFile).copy(destFile);
        qDebug() << "from " << audioFile << " to " << destFile;
        mAudioPath = destFile;
    }
    else
        mAudioPath = UBApplication::documentController->selectedDocument()->persistencePath() + "/" + audioFile;
}

UBGraphicsItemPlayAudioAction::~UBGraphicsItemPlayAudioAction()
{
    if(!mMediaObject && mMediaObject->state() == Phonon::PlayingState)
        mMediaObject->stop();
}

void UBGraphicsItemPlayAudioAction::play()
{
    if(!mMediaObject){
        mAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        mMediaObject = new Phonon::MediaObject(this);
        Phonon::createPath(mMediaObject, mAudioOutput);
        mMediaObject->setCurrentSource(Phonon::MediaSource(mAudioPath));
    }
    else{
        mMediaObject->stop();
        mMediaObject->seek(0);
    }
    mMediaObject->play();
}


QString UBGraphicsItemPlayAudioAction::save()
{
    QString documentPath = UBApplication::documentController->selectedDocument()->persistencePath() + "/" ;
    return mAudioPath.replace(documentPath,"");
}

void UBGraphicsItemPlayAudioAction::actionRemoved()
{
    QFile(mAudioPath).remove();
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
    case eMoveToNextPage:
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
