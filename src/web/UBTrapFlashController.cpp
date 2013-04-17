/*
 * Copyright (C) 2012 Webdoc SA
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



#include "UBTrapFlashController.h"

#include <QtXml>


#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "gui/UBMainWindow.h"

#include "network/UBNetworkAccessManager.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "board/UBBoardPaletteManager.h"
#include "board/UBBoardController.h"

#include "frameworks/UBPlatformUtils.h"

#include "ui_trapFlash.h"

#include "core/memcheck.h"

#include "web/UBWebController.h"
#include "browser/WBBrowserWindow.h"

UBTrapWebPageContentController::UBTrapWebPageContentController(QWidget* parent)
    : QObject(parent)
    , mParentWidget(parent)
    , mCurrentWebFrame(0)
    , mTrapWebContentDialog(NULL)
{
    // NOOP
}


UBTrapWebPageContentController::~UBTrapWebPageContentController()
{
    // NOOP
}

void UBTrapWebPageContentController::text_Changed(const QString &newText)
{
    QString new_text = newText;

#ifdef Q_WS_WIN // Defined on Windows.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_WS_QWS // Defined on Qt for Embedded Linux.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_WS_MAC // Defined on Mac OS X.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_WS_X11 // Defined on X11.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

    if(new_text.indexOf(regExp) > -1)
    {
        new_text.remove(regExp);
        mTrapWebContentDialog->applicationNameLineEdit()->setText(new_text);
        QToolTip::showText(mTrapWebContentDialog->applicationNameLineEdit()->mapToGlobal(QPoint()), "Application name can`t contain any of the following characters:\r\n"+illegalCharList);
    }
}

void UBTrapWebPageContentController::text_Edited(const QString &newText)
{
    Q_UNUSED(newText);
}


void UBTrapWebPageContentController::showTrapContent()
{
    if (!mTrapWebContentDialog)
    {
        mTrapWebContentDialog = new WBTrapWebPageContentWindow(this, mParentWidget);
        mTrapWebContentDialog->webView()->page()->setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        mTrapWebContentDialog->webView()->settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    }

    mTrapWebContentDialog->show();
    mTrapWebContentDialog->setUrl(UBApplication::webController->currentPageUrl());
}
void UBTrapWebPageContentController::hideTrapContent()
{
    if (mTrapWebContentDialog)
        mTrapWebContentDialog->hide();
}


void UBTrapWebPageContentController::setLastWebHitTestResult(const QWebHitTestResult &result)
{
    mLastWebHitTestResult = result;
}

void UBTrapWebPageContentController::updateListOfContents(const QList<UBWebKitUtils::HtmlObject>& objects)
{
    if (mTrapWebContentDialog)
    {
        mAvaliableObjects = QList<UBWebKitUtils::HtmlObject>() << objects;
        QComboBox *combobox = mTrapWebContentDialog->itemsComboBox();

        disconnect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));

        mObjectNoToTrapByTrapWebComboboxIndex.clear();
        combobox->clear();
        combobox->addItem(tr("Whole page"));
        mObjectNoToTrapByTrapWebComboboxIndex.insert(combobox->count(), 0);

        for(int i = 1; i < objects.count(); i++)
        {
            UBWebKitUtils::HtmlObject wrapper = objects.at(i);

            if ("separator"== wrapper.tagName)
                combobox->insertSeparator(combobox->count());
            else
            {
                if (wrapper.objectMimeType.contains("html"))
                    continue;

                mObjectNoToTrapByTrapWebComboboxIndex.insert(combobox->count(), i);
                combobox->addItem(wrapper.objectName);
            }
        }

        for(int i = 1; i <= objects.count(); i++)
        {
            UBWebKitUtils::HtmlObject wrapper = objects.at(i-1);
            if (UBApplication::webController->isOEmbedable(QUrl(wrapper.source)))
            {
                UBHttpGet *getEmbed = new UBHttpGet(this);
                getEmbed->get(QUrl("http://www.youtube.com/oembed?url="+wrapper.source+"&format=xml"));
                connect(getEmbed, SIGNAL(downloadFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)), this, SLOT(oEmbedRequestFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)));
            }
        }

        connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));
    }
}

void UBTrapWebPageContentController::selectHtmlObject(int pObjectIndex)
{
    if (pObjectIndex == 0)
    {
        mTrapWebContentDialog->webView()->setHtml(generateFullPageHtml(mCurrentWebFrame->url()));
        QVariant res = mCurrentWebFrame->evaluateJavaScript("window.document.title");
        mTrapWebContentDialog->applicationNameLineEdit()->setText(res.toString().trimmed());
        UBApplication::mainWindow->actionWebTrapToLibrary->setDisabled(true);
        UBApplication::mainWindow->actionWebTrapToCurrentPage->setDisabled(true);
    }
    else if (pObjectIndex > 0 && pObjectIndex <= mAvaliableObjects.size())
    {
        UBWebKitUtils::HtmlObject currentObject;

        currentObject = mAvaliableObjects.at(mObjectNoToTrapByTrapWebComboboxIndex.value(pObjectIndex));
        generatePreview(currentObject);

        mTrapWebContentDialog->applicationNameLineEdit()->setText(currentObject.objectName);

        UBApplication::mainWindow->actionWebTrapToLibrary->setDisabled(!currentObject.embedCode.isEmpty());
        UBApplication::mainWindow->actionWebTrapToCurrentPage->setDisabled(!currentObject.embedCode.isEmpty());
    }
}

void UBTrapWebPageContentController::oEmbedRequestFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
    Q_UNUSED(pContentTypeHeader)
    Q_UNUSED(pPos)
    Q_UNUSED(pSize)
    Q_UNUSED(isBackground)

    if (pSuccess)
    {
        QDomDocument response;
        
        response.setContent(pData);

        QString sResponse = response.toString();
        QString sSourceUrl = sourceUrl.toString().remove("http://www.youtube.com/oembed?url=").remove("&format=xml");

        UBWebKitUtils::HtmlObject obj(sSourceUrl, sResponse);
        mAvaliableObjects << obj;

        QComboBox *combobox = mTrapWebContentDialog->itemsComboBox();

        mObjectNoToTrapByTrapWebComboboxIndex.insert(combobox->count(), mAvaliableObjects.count()-1);
        combobox->addItem(tr("Embed ") + obj.objectName);
    }
}
void UBTrapWebPageContentController::prepareCurrentItemForImport(bool sendToBoard)
{
    int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();
    UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(mObjectNoToTrapByTrapWebComboboxIndex.value(selectedIndex));

    QSize currentItemSize(selectedObject.width, selectedObject.height);

    sDownloadFileDesc desc;
    desc.isBackground = false;
    desc.modal = false;
    desc.dest = sendToBoard ? sDownloadFileDesc::board : sDownloadFileDesc::library;
    desc.name = mTrapWebContentDialog->applicationNameLineEdit()->text();
    desc.srcUrl = selectedObject.source;
    desc.size = currentItemSize;
    desc.contentTypeHeader = selectedObject.objectMimeType;
    UBDownloadManager::downloadManager()->addFileToDownload(desc);
}

void UBTrapWebPageContentController::addLink(bool isOnLibrary)
{
    int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();
    UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(mObjectNoToTrapByTrapWebComboboxIndex.value(selectedIndex));
    QSize size(selectedObject.width,selectedObject.height);
    if(isOnLibrary)
    {
        if(selectedIndex == 0)
            UBApplication::boardController->paletteManager()->featuresWidget()->createBookmark(mTrapWebContentDialog->applicationNameLineEdit()->text(),selectedObject.source);
        else
            UBApplication::boardController->paletteManager()->featuresWidget()->createLink(mTrapWebContentDialog->applicationNameLineEdit()->text(), selectedObject.source,size, selectedObject.objectMimeType, selectedObject.embedCode);
    }
    else
        UBApplication::boardController->addLinkToPage(selectedObject.source, size, QPointF(), selectedObject.embedCode);
}

void UBTrapWebPageContentController::addItemToLibrary()
{
    mTrapWebContentDialog->setReadyForTrap(false);
    mCurrentItemImportDestination = library;
    prepareCurrentItemForImport(false);
    UBApplication::applicationController->showBoard();
    hideTrapContent();
}

void UBTrapWebPageContentController::addItemToBoard()
{
    mTrapWebContentDialog->setReadyForTrap(false);
    mCurrentItemImportDestination = board;
    prepareCurrentItemForImport(true);
    UBApplication::applicationController->showBoard();
    hideTrapContent();
}

void UBTrapWebPageContentController::addLinkToLibrary()
{
    mTrapWebContentDialog->setReadyForTrap(false);
    mCurrentItemImportDestination = library;
    addLink(true);
    UBApplication::applicationController->showBoard();
    hideTrapContent();
    mTrapWebContentDialog->setReadyForTrap(true);

}

void UBTrapWebPageContentController::addLinkToBoard()
{
    mTrapWebContentDialog->setReadyForTrap(false);
    mCurrentItemImportDestination = board;
    addLink(false);
    UBApplication::applicationController->showBoard();
    hideTrapContent();
    mTrapWebContentDialog->setReadyForTrap(true);
}

void UBTrapWebPageContentController::updateTrapContentFromPage(QWebFrame* pCurrentWebFrame)
{
    if (pCurrentWebFrame && (mTrapWebContentDialog && mTrapWebContentDialog->isVisible()))
    {
        QList<UBWebKitUtils::HtmlObject> list;
        if (mTrapWebContentDialog)
        {
            list << UBWebKitUtils::HtmlObject(pCurrentWebFrame->baseUrl().toString(), widgetNameForUrl(pCurrentWebFrame->title()), QString(),"Whole Page", 800, 600);
            list << UBWebKitUtils::HtmlObject(QString(),QString(),QString(),"separator",0,0);
            //list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "object");
            //list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "img");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "audio");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "video");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "source");
            //list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "iframe");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "frame");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "a");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "embed");
            list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "link");
        }

        mCurrentWebFrame = pCurrentWebFrame;
        updateListOfContents(list);

        mTrapWebContentDialog->applicationNameLineEdit()->setText(mCurrentWebFrame->title());

        mTrapWebContentDialog->setReadyForTrap(!list.isEmpty());
        UBApplication::mainWindow->actionWebTrapToLibrary->setDisabled(true);
        UBApplication::mainWindow->actionWebTrapToCurrentPage->setDisabled(true);
    }
}


QString UBTrapWebPageContentController::generateIcon(const QString& pDirPath)
{
    QDesktopWidget* desktop = QApplication::desktop();
    QPoint webViewPosition = mTrapWebContentDialog->webView()->mapToGlobal(mTrapWebContentDialog->webView()->pos());
    QSize webViewSize = mTrapWebContentDialog->webView()->size();
    QPixmap capture = QPixmap::grabWindow(desktop->winId(), webViewPosition.x(), webViewPosition.y()
            , webViewSize.width() - 10, webViewSize.height() -10);

    QPixmap widgetIcon(75,75);
    widgetIcon.fill(Qt::transparent);

    QPainter painter(&widgetIcon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(180,180,180)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(0, 0, 75, 75, 10, 10);
    QPixmap icon = capture.scaled(65, 65);
    painter.drawPixmap(5,5,icon);

    QString iconFile = pDirPath + "/icon.png";
    widgetIcon.save(iconFile, "PNG");

    return iconFile;
}


void UBTrapWebPageContentController::generateConfig(int pWidth, int pHeight, const QString& pDestinationPath)
{
    QFile configFile(pDestinationPath + "/" + "config.xml");

    if (configFile.exists())
    {
        configFile.remove(configFile.fileName());
    }

    if (!configFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open file " << configFile.fileName();
        return;
    }

    QTextStream out(&configFile);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    out << "<widget xmlns=\"http://www.w3.org/ns/widgets\"" << endl;
    out << "        xmlns:ub=\"http://uniboard.mnemis.com/widgets\"" << endl;
    out << "        identifier=\"http://uniboard.mnemis.com/" << mTrapWebContentDialog->applicationNameLineEdit()->text() << "\"" <<endl;

    out << "        version=\"1.0\"" << endl;
    out << "        width=\"" << pWidth << "\"" << endl;
    out << "        height=\"" << pHeight << "\"" << endl;
    out << "        ub:resizable=\"true\">" << endl;

    out << "    <name>" << mTrapWebContentDialog->applicationNameLineEdit()->text() << "</name>" << endl;
    out << "    <content src=\"" << mTrapWebContentDialog->applicationNameLineEdit()->text() << ".html\"/>" << endl;

    out << "</widget>" << endl;


    configFile.close();
}


QString UBTrapWebPageContentController::generateFullPageHtml(const QUrl &srcUrl)
{
    QString htmlContentString;

    htmlContentString = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n";
    htmlContentString += "<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n";
    htmlContentString += "    <head>\r\n";
    htmlContentString += "        <meta http-equiv=\"refresh\" content=\"0; " + srcUrl.toString() + "\">\r\n";
    htmlContentString += "    </head>\r\n";
    htmlContentString += "    <body>\r\n";
    htmlContentString += "        Redirect to target...\r\n";
    htmlContentString += "    </body>\r\n";
    htmlContentString += "</html>\r\n";

    return htmlContentString;
}


void UBTrapWebPageContentController::generatePreview(const UBWebKitUtils::HtmlObject& pObject, bool bTryToEmbed)
{
    QUrl objectUrl(pObject.source);
    QString objectFullUrl = pObject.source;
    qDebug() << objectFullUrl;
    if (!objectUrl.isValid())
    {
        qWarning() << "invalid URL " << pObject.source;
        return;
    }

    QString mimeType = (UBFileSystemUtils::mimeTypeFromFileName(objectFullUrl).length() == 0) ? pObject.objectMimeType : UBFileSystemUtils::mimeTypeFromFileName(objectFullUrl);

    if(mimeType.contains("x-shockwave-flash")){
        QUrl url = QUrl::fromLocalFile(UBGraphicsW3CWidgetItem::createNPAPIWrapperInDir(objectFullUrl, QDir::tempPath(), mimeType, QSize(800,600), QUuid::createUuid().toString()) + "/index.htm");
        mTrapWebContentDialog->webView()->load(url);
        return;
    }

    QString htmlContentString;

    htmlContentString = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">";
    htmlContentString += "<html>";
    htmlContentString += "    <head>";
    htmlContentString += "        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">";
    htmlContentString += "    </head>";

    htmlContentString += "  <body bgcolor=\"rgb(180,180,180)\">";

    htmlContentString += "      <div align='center'>";


    if (!pObject.embedCode.isEmpty())
    {
        htmlContentString += pObject.embedCode;
    }
    else if(mimeType.contains("image")){
        htmlContentString += "    <img src=\"" + objectFullUrl + "\">";
    }
    else if (mimeType.contains("video")){
        htmlContentString += "    <video src=\"" + objectFullUrl + "\" controls=\"controls\">";
    }
    else if (mimeType.contains("audio")){
        htmlContentString += "    <audio src=\"" + objectFullUrl + "\" controls=\"controls\">";
    }
    else if (mimeType.contains("html"))
    {
        if (bTryToEmbed)
        {
            QString videoId;
            
            int videoIdStartPos = -1;
            if (objectFullUrl.contains("watch?v="))
            {
                videoIdStartPos = objectFullUrl.indexOf("watch?v=") + QString("watch?v=").size();
            }
            else if (objectFullUrl.contains("/embed/"))
            {
                videoIdStartPos = objectFullUrl.indexOf("/embed/") + QString("/embed/").size();
            }
            
            videoId = objectFullUrl.right(objectFullUrl.size() - videoIdStartPos);

            if (!videoId.isNull())
            {
                htmlContentString += "<iframe width=\""+QString("%1").arg((640 > pObject.width) ? 640 : pObject.width)+"\" height=\""+QString("%1").arg((480 > pObject.width) ? 480 : pObject.width)+"\" src=\"http://www.youtube.com/embed/"+videoId+"\" frameborder=\"0\" allowfullscreen></iframe>";
            }
        }
        else
        {
            htmlContentString = generateFullPageHtml(objectFullUrl);
            mTrapWebContentDialog->webView()->setHtml(htmlContentString);
            return;
            //htmlContentString +="        <iframe width=\"" + QString("%1").arg(mCurrentWebFrame->geometry().width()) + "\" height=\"" + QString("%1").arg(mCurrentWebFrame->geometry().height()) + "\" frameborder=0 src=\""+objectFullUrl+"\">";
        }
    }
    else if (mCurrentWebFrame->url().toString().contains("youtube")){
        QVariant res = mCurrentWebFrame->evaluateJavaScript("window.document.getElementById('embed_code').value");

        //force windowsless mode

        QString s = res.toString();
        htmlContentString += s.replace("></embed>", " wmode='opaque'></embed>");
    }
    else
        qWarning() << "not supported";

    htmlContentString += "        </div>";
    htmlContentString += "</body>";
    htmlContentString += "</html>";

    qDebug() << htmlContentString;
    mTrapWebContentDialog->webView()->setHtml(htmlContentString);
}

QString UBTrapWebPageContentController::widgetNameForUrl(QString pObjectUrl)
{
    QString url = pObjectUrl;
    int parametersIndex = url.indexOf("?");
    if(parametersIndex != -1)
        url = url.left(parametersIndex);
    int lastSlashIndex = url.lastIndexOf("/");

    QString result = url.right(url.length() - lastSlashIndex);
    result = UBFileSystemUtils::cleanName(result);

    return result;
}
