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


#ifndef UBGRAPHICSITEMSACTIONS_H
#define UBGRAPHICSITEMSACTIONS_H

#include <QObject>


enum eUBGraphicsItemMovePageAction {
    eMoveToFirstPage = 0,
    eMoveToLastPage,
    eMoveToPreviousPage,
    eMoveToNextNextPage,
    eMoveToPage
} ;

enum eUBGraphicsItemLinkType
{
    eLinkToAudio = 0,
    eLinkToPage,
    eLinkToWebUrl
};

class UBGraphicsItemAction : public QObject
{
    Q_OBJECT
public:
    UBGraphicsItemAction(eUBGraphicsItemLinkType linkType,QObject* parent = 0);
    virtual void play() = 0;
    virtual QString save() = 0;
    eUBGraphicsItemLinkType linkType() { return mLinkType;}

signals:

public slots:
private:
    eUBGraphicsItemLinkType mLinkType;

};


class UBGraphicsItemPlayAudioAction : public UBGraphicsItemAction
{
    Q_OBJECT

public:
    UBGraphicsItemPlayAudioAction(QString audioFile, QObject* parent = 0);
    void play();
    QString save();

private:
    QString mAudioPath;
};


class UBGraphicsItemMoveToPageAction : public UBGraphicsItemAction
{
    Q_OBJECT

public:
    UBGraphicsItemMoveToPageAction(eUBGraphicsItemMovePageAction actionType, int page = 0, QObject* parent = 0);
    void play();
    QString save();

private:
    eUBGraphicsItemMovePageAction mActionType;
    int mPage;

};

class UBGraphicsItemLinkToWebPageAction : public UBGraphicsItemAction
{
    Q_OBJECT

public:
    UBGraphicsItemLinkToWebPageAction(QString url, QObject* parent = 0);
    void play();
    QString save();

private:
    QString mUrl;
};

#endif // UBGRAPHICSITEMSACTIONS_H
