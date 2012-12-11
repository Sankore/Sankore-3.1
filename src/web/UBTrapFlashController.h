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


#ifndef UBTRAPFLASHCONTROLLER_H_
#define UBTRAPFLASHCONTROLLER_H_

#include <QtGui>
#include "UBWebKitUtils.h"
#include "web/browser/WBTrapWebPageContent.h"

namespace Ui
{
    class trapFlashDialog;
}


class UBTrapWebPageContentController : public QObject
{
    Q_OBJECT

    enum importDestination
    {
        library = 0,
        board
    };


    public:
        UBTrapWebPageContentController(QWidget* parent = 0);
        virtual ~UBTrapWebPageContentController();

        void showTrapContent();
        void hideTrapContent();

        void setLastWebHitTestResult(const QWebHitTestResult &result);

    public slots:
        void updateTrapContentFromPage(QWebFrame* pCurrentWebFrame);
        void text_Changed(const QString &);
        void text_Edited(const QString &);
        void addItemToLibrary();
        void addItemToBoard();
        void addLinkToLibrary();
        void addLinkToBoard();

    private slots:
        void selectHtmlObject(int pObjectIndex);
        void selectedItemReady(bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QUrl destinationUrl, QString pContentTypeHeader, QByteArray pData, QSize pSize);

        void importItemInLibrary(QString pSourceDir);

    private:

        void addLink(bool isOnLibrary);
        void prepareCurrentItemForImport(bool bUseAsLink);
        void updateListOfContents(const QList<UBWebKitUtils::HtmlObject>& objects);

        QString widgetNameForUrl(QString pObjectUrl);

        QString generateFullPageHtml(const QUrl &srcUrl, const QString& pDirPath, bool pGenerateFile);
        void generatePreview(const UBWebKitUtils::HtmlObject& pObject, const QString& pDirPath, bool pGenerateFile);

        QString generateIcon(const QString& pDirPath);

        void generateConfig(int pWidth, int pHeight, const QString& pDestinationPath);


        QWidget* mParentWidget;
        QWebFrame* mCurrentWebFrame;
        QList<UBWebKitUtils::HtmlObject> mAvaliableObjects;
        QMap<int, int> mObjectNoByTrapWebComboboxIndex;

        WBTrapWebPageContentWindow *mTrapWebContentDialog;
        QWebHitTestResult mLastWebHitTestResult;

        QUrl mCurrentItemUrl;
        importDestination mCurrentItemImportDestination;
};


#endif /* UBTRAPFLASHCONTROLLER_H_ */
