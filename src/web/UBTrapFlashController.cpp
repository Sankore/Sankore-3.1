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


#include "UBTrapFlashController.h"

#include <QtXml>


#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

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
    freeTemp();
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
        mAvaliableObjects = objects;

        if (mTrapWebContentDialog)
        {
            QComboBox *combobox = mTrapWebContentDialog->itemsComboBox();

            disconnect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));

            mObjectNoByTrapWebComboboxIndex.clear();
            combobox->clear();
            combobox->addItem(tr("Whole page")); 

            QString lastTagName;
            for(int i = 0; i < mAvaliableObjects.count(); i++)
            {
                UBWebKitUtils::HtmlObject wrapper = mAvaliableObjects.at(i);

                if (lastTagName != wrapper.tagName)
                {
                    lastTagName = wrapper.tagName;
                    combobox->insertSeparator(combobox->count());
                }
                mObjectNoByTrapWebComboboxIndex.insert(combobox->count(), i);
                combobox->addItem(widgetNameForUrl(wrapper.source));
            }
            connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));
        }
    }
}

void UBTrapWebPageContentController::selectHtmlObject(int pObjectIndex)
{
    if (pObjectIndex == 0)
    {
        mTrapWebContentDialog->webView()->setHtml(generateFullPageHtml(mCurrentWebFrame->url(), "", false));
        QVariant res = mCurrentWebFrame->evaluateJavaScript("window.document.title");
        mTrapWebContentDialog->applicationNameLineEdit()->setText(res.toString().trimmed());
   
    }
    else if (pObjectIndex > 0 && pObjectIndex <= mAvaliableObjects.size())
    {
        UBWebKitUtils::HtmlObject currentObject = mAvaliableObjects.at(mObjectNoByTrapWebComboboxIndex.value(pObjectIndex));

        mTrapWebContentDialog->webView()->setHtml(generateHtml(currentObject, "", false));
        mTrapWebContentDialog->applicationNameLineEdit()->setText(widgetNameForUrl(currentObject.source));
    }
}

QUrl UBTrapWebPageContentController::itemUrlFromSelectedContent(bool bUseAsLink)
{
    int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();
    UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(mObjectNoByTrapWebComboboxIndex.value(selectedIndex));
    QString tempDir = UBFileSystemUtils::createTempDir("TrapWebContentRendering");
    mTempDirectoriesUsed.insert(tempDir);

    if (selectedIndex == 0 || UBApplication::webController->isOEmbedable(QUrl(selectedObject.source)) || bUseAsLink)
    {
        QDir widgetDir(tempDir + "/" + mTrapWebContentDialog->applicationNameLineEdit()->text() + ".wgt");
        if (widgetDir.exists() && !UBFileSystemUtils::deleteDir(widgetDir.path()))
        {
            qWarning() << "Cannot delete " << widgetDir.path();
        }

        widgetDir.mkpath(widgetDir.path());

        if (UBApplication::webController->isOEmbedable(QUrl(selectedObject.source)))
        {
            QString embedWidgetPath = UBApplication::boardController->paletteManager()->featuresWidget()->getFeaturesController()->getFeaturePathByName("Sel video");
            if (UBFileSystemUtils::copyDir(embedWidgetPath, widgetDir.path()))
            {
                UBGraphicsW3CWidgetItem *widget = new UBGraphicsW3CWidgetItem(QUrl::fromLocalFile(embedWidgetPath), 0);
                if (widget)
                {
                    widget->setPreference("oembedUrl", selectedObject.source);
                }
            }
        }
        else if (selectedIndex == 0)
        {        
            generateFullPageHtml(mCurrentWebFrame->url(), widgetDir.path(), true);
            generateConfig(800, 600, widgetDir.path());
        }
        else if (bUseAsLink)
        {
            generateFullPageHtml(QUrl(selectedObject.source), widgetDir.path(), true);
            generateConfig(selectedObject.width, selectedObject.height, widgetDir.path());
        }

        generateIcon(widgetDir.path());

        return QUrl::fromLocalFile(widgetDir.path());
       
    }
    else
        return QUrl(selectedObject.source);

}


void UBTrapWebPageContentController::freeTemp()
{
    foreach(QString tempDir, mTempDirectoriesUsed)
    {
        UBFileSystemUtils::deleteDir(tempDir);
    }
}

void UBTrapWebPageContentController::addItemToLibrary()
{
    importItemInLibrary(itemUrlFromSelectedContent(false).toString());
    freeTemp();
    UBApplication::applicationController->showBoard();
}

void UBTrapWebPageContentController::addItemToBoard()
{
    int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();
    UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(mObjectNoByTrapWebComboboxIndex.value(selectedIndex));
    UBApplication::boardController->downloadURL(itemUrlFromSelectedContent(false), QString(), QPoint(0, 0), QSize(selectedObject.width, selectedObject.height));
    freeTemp();
    UBApplication::applicationController->showBoard();
}

void UBTrapWebPageContentController::addLinkToLibrary()
{


}

void UBTrapWebPageContentController::addLinkToBoard()
{
    int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();
    UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(mObjectNoByTrapWebComboboxIndex.value(selectedIndex));
    UBApplication::boardController->downloadURL(itemUrlFromSelectedContent(true), QString(), QPoint(0, 0), QSize(selectedObject.width, selectedObject.height));
    freeTemp();
    UBApplication::applicationController->showBoard();
}

void UBTrapWebPageContentController::importItemInLibrary(QString pSourceDir)
{
    if (pSourceDir.startsWith("file://") || pSourceDir.startsWith("/"))
    {    
        QString widgetLibraryPath = UBApplication::boardController->paletteManager()->featuresWidget()->importFromUrl(QUrl::fromUserInput(pSourceDir));   
    }
    else
    {
        int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();
        UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(mObjectNoByTrapWebComboboxIndex.value(selectedIndex));

        sDownloadFileDesc desc;
        desc.isBackground = false;
        desc.modal = false;
        desc.dest = sDownloadFileDesc::library;
        desc.name = widgetNameForUrl(pSourceDir);
        qDebug() << desc.name;
        desc.srcUrl = pSourceDir;
        desc.size = QSize(selectedObject.width, selectedObject.height);
        UBDownloadManager::downloadManager()->addFileToDownload(desc);

    }
}


void UBTrapWebPageContentController::updateTrapContentFromPage(QWebFrame* pCurrentWebFrame)
{
    if (pCurrentWebFrame && (mTrapWebContentDialog && mTrapWebContentDialog->isVisible()))
    {
        QList<UBWebKitUtils::HtmlObject> list;
        if (mTrapWebContentDialog)
        {
             list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "embed");
             list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "img");
             list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "image");
             list << UBWebKitUtils::objectsInFrameByTag(pCurrentWebFrame, "audio");
        }

        mCurrentWebFrame = pCurrentWebFrame;
        updateListOfContents(list);
        
        mTrapWebContentDialog->applicationNameLineEdit()->setText(mCurrentWebFrame->title());

        mTrapWebContentDialog->setReadyForTrap(!list.isEmpty());
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


QString UBTrapWebPageContentController::generateFullPageHtml(const QUrl &srcUrl, const QString& pDirPath, bool pGenerateFile)
{
    QString htmlContentString;

    htmlContentString = 
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n \
    <html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n \
        <head>\r\n \
            <meta http-equiv=\"refresh\" content=\"0; " + srcUrl.toString() + "\">\r\n \
        </head>\r\n \
        <body>\r\n \
            Redirect to target...\r\n \
        </body>\r\n \
    </html>\r\n";

    if (!pGenerateFile)
    {
        return htmlContentString;
    }
    else
    {
        QString fileName = mTrapWebContentDialog->applicationNameLineEdit()->text();
        const QString fullHtmlFileName = pDirPath + "/" + fileName + ".html";
        QDir dir(pDirPath);
        if (!dir.exists())
        {
            dir.mkpath(dir.path());
        }
        QFile widgetHtmlFile(fullHtmlFileName);
        if (widgetHtmlFile.exists())
        {
            widgetHtmlFile.remove(widgetHtmlFile.fileName());
        }
        if (!widgetHtmlFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file " << widgetHtmlFile.fileName();
            return "";
        }
        QTextStream out(&widgetHtmlFile);
        out << htmlContentString;

        widgetHtmlFile.close();
        return widgetHtmlFile.fileName();
    }
}


QString UBTrapWebPageContentController::generateHtml(const UBWebKitUtils::HtmlObject& pObject,
        const QString& pDirPath, bool pGenerateFile)
{
    qDebug() << pObject.source;
    QUrl objectUrl(pObject.source);
    QString objectFullUrl = pObject.source;
    if (!objectUrl.isValid())
    {
        qWarning() << "invalid URL " << pObject.source;
        return "";
    }
    if (objectUrl.isRelative())
    {
        int lastSlashIndex = mCurrentWebFrame->url().toString().lastIndexOf("/");
        QString objectPath = mCurrentWebFrame->url().toString().left(lastSlashIndex);
        objectFullUrl =   objectPath+ "/" + pObject.source;
    }

    QString htmlContentString;

    htmlContentString = 
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n \
    <html>\n \
        <head>\n \
            <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n \
        </head>\n";

    if (!pGenerateFile)
    {
        htmlContentString += "  <body bgcolor=\"rgb(180,180,180)\">\n";
    }
    else
    {
        htmlContentString += "  <body>\n";
    }

    htmlContentString += "      <div align='center'>\n";

    if (mCurrentWebFrame->url().toString().contains("youtube"))
    {
        QVariant res = mCurrentWebFrame->evaluateJavaScript("window.document.getElementById('embed_code').value");

        //force windowsless mode

        QString s = res.toString();
        htmlContentString += s.replace("></embed>", " wmode='opaque'></embed>");
    }
    else
    {
        htmlContentString += "            <object classid='clsid:D27CDB6E-AE6D-11cf-96B8-444553540000'\n \
                        codebase='http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0'\n \
                        width='" + QString::number(pObject.width)  + "' height='" + QString::number(pObject.height) + "'>\n \
                        <param name='movie' value='" + objectFullUrl + "'>\n \
                        <param name='quality' value='high'>\n \
                        <param name='wmode' value='opaque'>\n \
                        <embed src='" + objectFullUrl + "'\n \
                            quality='high'\n \
                            pluginspage='http://www.macromedia.com/go/getflashplayer'\n \
                            type='application/x-shockwave-flash'\n \
                             width='" + QString::number(pObject.width - 20) + "' height='" + QString::number(pObject.height - 20) + "' wmode='opaque'>\n \
                        </embed>\n \
                    </object>\n";
    }

    htmlContentString += "        </div>\n \
    </body>\n \
    </html>\n";

    if (!pGenerateFile)
    {
            return htmlContentString;
    }
    else
    {
        QString fileName = mTrapWebContentDialog->applicationNameLineEdit()->text();
        const QString fullHtmlFileName = pDirPath + "/" + fileName + ".html";
        QDir dir(pDirPath);

        if (!dir.exists())
        {
            dir.mkpath(dir.path());
        }

        QFile widgetHtmlFile(fullHtmlFileName);

        if (widgetHtmlFile.exists())
        {
            widgetHtmlFile.remove(widgetHtmlFile.fileName());
        }

        if (!widgetHtmlFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file " << widgetHtmlFile.fileName();
            return "";
        }

        QTextStream out(&widgetHtmlFile);
        out << htmlContentString;

        widgetHtmlFile.close();
        return widgetHtmlFile.fileName();
    }
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