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

#include "board/UBBoardController.h"

#include "frameworks/UBPlatformUtils.h"

#include "ui_trapFlash.h"

#include "core/memcheck.h"

#include "web/UBWebController.h"
#include "browser/WBBrowserWindow.h"

UBTrapWebPageContentController::UBTrapWebPageContentController(QWidget* parent)
    : QObject(parent)
    , mTrapFlashUi(0)
    , mTrapFlashDialog(0)
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


void UBTrapWebPageContentController::showTrapFlash()
{
    if (!mTrapFlashDialog)
    {
        Qt::WindowFlags flag = Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint ;
        flag &= ~ Qt::WindowContextHelpButtonHint;
        flag &= ~ Qt::WindowMinimizeButtonHint;

        mTrapFlashDialog = new QDialog(mParentWidget, flag);
        mTrapFlashUi = new Ui::trapFlashDialog();
        mTrapFlashUi->setupUi(mTrapFlashDialog);

        mTrapFlashUi->webView->page()->setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());
        int viewWidth = mParentWidget->width() / 2;
        int viewHeight = mParentWidget->height() * 2. / 3.;
        mTrapFlashDialog->setGeometry(
                (mParentWidget->width() - viewWidth) / 2
                , (mParentWidget->height() - viewHeight) / 2
                , viewWidth
                , viewHeight);

        connect(mTrapFlashUi->flashCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));
        connect(mTrapFlashUi->widgetNameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(text_Changed(const QString &)));
        connect(mTrapFlashUi->widgetNameLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(text_Edited(const QString &)));
        connect(mTrapFlashUi->createWidgetButton, SIGNAL(clicked(bool)), this, SLOT(createWidget()));
    }

    mTrapFlashDialog->show();
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

void UBTrapWebPageContentController::hideTrapFlash()
{
    if (mTrapFlashDialog)
    {
        mTrapFlashDialog->hide();
    }
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
    if (mTrapFlashDialog || mTrapWebContentDialog)
    {
        mAvaliableObjects = objects;

        if (mTrapWebContentDialog)
        {
            QComboBox *combobox = mTrapWebContentDialog->itemsComboBox();

            disconnect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));

            combobox->clear();
            combobox->addItem(tr("Whole page"));

            QString lastTagName;
            foreach(UBWebKitUtils::HtmlObject wrapper, mAvaliableObjects)
            {
                if (lastTagName != wrapper.tagName)
                {
                    lastTagName = wrapper.tagName;
                    combobox->insertSeparator(combobox->count());
                }
                combobox->addItem(widgetNameForObject(wrapper));
            }

            connect(combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectHtmlObject(int)));
        }

    }
}


void UBTrapWebPageContentController::selectHtmlObject(int pObjectIndex)
{
    if (pObjectIndex == 0)
    {
        mTrapWebContentDialog->webView()->setHtml(generateFullPageHtml("", false));
        QVariant res = mCurrentWebFrame->evaluateJavaScript("window.document.title");
        mTrapWebContentDialog->applicationNameLineEdit()->setText(res.toString().trimmed());

    }
    else if (pObjectIndex > 0 && pObjectIndex <= mAvaliableObjects.size())
    {
        UBWebKitUtils::HtmlObject currentObject = mAvaliableObjects.at(pObjectIndex - 1);
        mTrapWebContentDialog->webView()->setHtml(generateHtml(currentObject, "", false));
        mTrapWebContentDialog->applicationNameLineEdit()->setText(widgetNameForObject(currentObject));
    }
}


void UBTrapWebPageContentController::createWidget()
{
    int selectedIndex = mTrapWebContentDialog->itemsComboBox()->currentIndex();

    if (selectedIndex == 0)
    {
        // full page widget
        QString tempDir = UBFileSystemUtils::createTempDir("TrapWebContentRendering");
        QDir widgetDir(tempDir + "/" + mTrapWebContentDialog->applicationNameLineEdit()->text() + ".wgt");

        if (widgetDir.exists() && !UBFileSystemUtils::deleteDir(widgetDir.path()))
        {
            qWarning() << "Cannot delete " << widgetDir.path();
        }

        widgetDir.mkpath(widgetDir.path());

        generateFullPageHtml(widgetDir.path(), true);

        generateIcon(widgetDir.path());
        generateConfig(800, 600, widgetDir.path());

        //generateDefaultPng(width, height, widgetDir.path());

        importWidgetInLibrary(widgetDir);

        UBFileSystemUtils::deleteDir(tempDir);
    }
    else
    {
        // flash widget
        UBWebKitUtils::HtmlObject selectedObject = mAvaliableObjects.at(selectedIndex - 1);
        UBApplication::applicationController->showBoard();
        UBApplication::boardController->downloadURL(QUrl(selectedObject.source), QString(), QPoint(0, 0), QSize(selectedObject.width, selectedObject.height));
    }

    QString freezedWidgetPath = UBPlatformUtils::applicationResourcesDirectory() + "/etc/freezedWidgetWrapper.html";
    mTrapWebContentDialog->webView()->load(QUrl::fromLocalFile(freezedWidgetPath));

    mTrapWebContentDialog->hide();
}


void UBTrapWebPageContentController::importWidgetInLibrary(QDir pSourceDir)
{
    const QString userWidgetPath = UBSettings::settings()->userInteractiveDirectory() + "/" + tr("Web");
    QDir userWidgetDir(userWidgetPath);

    if (!userWidgetDir.exists())
    {
        userWidgetDir.mkpath(userWidgetPath);
    }

    QString widgetLibraryPath = userWidgetPath + "/" + mTrapFlashUi->widgetNameLineEdit->text() + ".wgt";
    QDir widgetLibraryDir(widgetLibraryPath);

    if (widgetLibraryDir.exists())
    {
        if (!UBFileSystemUtils::deleteDir(widgetLibraryDir.path()))
        {
            qWarning() << "Cannot delete old widget " << widgetLibraryDir.path();
        }
    }

    qDebug() << "Widget imported in path " << widgetLibraryPath;
    UBFileSystemUtils::copyDir(pSourceDir.path(), widgetLibraryPath);

    // also add to current scene
    if (UBApplication::applicationController)
        UBApplication::applicationController->showBoard();

    if (UBApplication::boardController &&
        UBApplication::boardController->activeScene())
    {
        UBApplication::boardController->activeScene()->addWidget(QUrl::fromLocalFile(widgetLibraryPath));
    }
}


void UBTrapWebPageContentController::updateTrapContentFromPage(QWebFrame* pCurrentWebFrame)
{
    if (pCurrentWebFrame && (mTrapFlashDialog && mTrapFlashDialog->isVisible()) || (mTrapWebContentDialog && mTrapWebContentDialog->isVisible()))
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
    }
}


QString UBTrapWebPageContentController::generateIcon(const QString& pDirPath)
{
    QDesktopWidget* desktop = QApplication::desktop();
    QPoint webViewPosition = mTrapFlashUi->webView->mapToGlobal(mTrapFlashUi->webView->pos());
    QSize webViewSize = mTrapFlashUi->webView->size();
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
    out << "        identifier=\"http://uniboard.mnemis.com/" << mTrapFlashUi->widgetNameLineEdit->text() << "\"" <<endl;

    out << "        version=\"1.0\"" << endl;
    out << "        width=\"" << pWidth << "\"" << endl;
    out << "        height=\"" << pHeight << "\"" << endl;
    out << "        ub:resizable=\"true\">" << endl;

    out << "    <name>" << mTrapFlashUi->widgetNameLineEdit->text() << "</name>" << endl;
    out << "    <content src=\"" << mTrapFlashUi->widgetNameLineEdit->text() << ".html\"/>" << endl;

    out << "</widget>" << endl;


    configFile.close();
}


QString UBTrapWebPageContentController::generateFullPageHtml(const QString& pDirPath, bool pGenerateFile)
{

    QString htmlContentString;

    htmlContentString += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n";
    htmlContentString += "<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n";
    htmlContentString += "  <head>\r\n";
    htmlContentString += "    <meta http-equiv=\"refresh\" content=\"0; " + mCurrentWebFrame->url().toString() + "\">\r\n";
    htmlContentString += "  </head>\r\n";
    htmlContentString += "  <body>\r\n";
    htmlContentString += "    Redirect to target...\r\n";
    htmlContentString += "  </body>\r\n";
    htmlContentString += "</html>\r\n";

    if (!pGenerateFile)
    {
        return htmlContentString;
    }
    else
    {
        QString fileName = mTrapFlashUi->widgetNameLineEdit->text();
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

    htmlContentString += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
    htmlContentString += "<html>\n";
    htmlContentString += "<head>\n";
    htmlContentString += "    <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n";
    htmlContentString += "</head>\n";

    if (!pGenerateFile)
    {
        htmlContentString += "<body bgcolor=\"rgb(180,180,180)\">\n";
    }
    else
    {
        htmlContentString += "<body>\n";
    }

    htmlContentString += "        <div align='center'>\n";

    if (mCurrentWebFrame->url().toString().contains("youtube"))
    {
        QVariant res = mCurrentWebFrame->evaluateJavaScript("window.document.getElementById('embed_code').value");

        //force windowsless mode

        htmlContentString += res.toString().replace("></embed>", " wmode='opaque'></embed>");
    }
    else
    {
        htmlContentString += "            <object classid='clsid:D27CDB6E-AE6D-11cf-96B8-444553540000'\n";
        htmlContentString += "                codebase='http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0'\n";
        htmlContentString += "                width='" + QString::number(pObject.width)  + "' height='" + QString::number(pObject.height) + "'>\n";
        htmlContentString += "                <param name='movie' value='" + objectFullUrl + "'>\n";
        htmlContentString += "                <param name='quality' value='high'>\n";
        htmlContentString += "                <param name='wmode' value='opaque'>\n";
        htmlContentString += "                <embed src='" + objectFullUrl + "'\n";
        htmlContentString += "                    quality='high'\n";
        htmlContentString += "                    pluginspage='http://www.macromedia.com/go/getflashplayer'\n";
        htmlContentString += "                    type='application/x-shockwave-flash'\n";
        htmlContentString += "                     width='" + QString::number(pObject.width - 20) + "' height='" + QString::number(pObject.height - 20) + "' wmode='opaque'>\n";
        htmlContentString += "                </embed>\n";
        htmlContentString += "            </object>\n";
    }

    htmlContentString += "        </div>\n";
    htmlContentString += "</body>\n";
    htmlContentString += "</html>\n";

    if (!pGenerateFile)
    {
            return htmlContentString;
    }
    else
    {
        QString fileName = mTrapFlashUi->widgetNameLineEdit->text();
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

QString UBTrapWebPageContentController::widgetNameForObject(UBWebKitUtils::HtmlObject pObject)
{
    QString url = pObject.source;
    int parametersIndex = url.indexOf("?");
    if(parametersIndex != -1)
        url = url.left(parametersIndex);
    int lastSlashIndex = url.lastIndexOf("/");

    QString result = url.right(url.length() - lastSlashIndex);
    result = UBFileSystemUtils::cleanName(result);

    return result;
}

