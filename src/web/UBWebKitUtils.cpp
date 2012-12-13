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


#include "UBWebKitUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/memcheck.h"

UBWebKitUtils::UBWebKitUtils()
{
    // NOOP
}

UBWebKitUtils::~UBWebKitUtils()
{
    // NOOP
}

QList<UBWebKitUtils::HtmlObject> UBWebKitUtils::objectsInFrameByTag(QWebFrame* frame, QString tagName)
{
    QList<UBWebKitUtils::HtmlObject> htmlObjects;

    if (frame)
    {
        QWebElementCollection elements = frame->documentElement().findAll(tagName);
        for (int i = 0; i < elements.count(); i++)
        {
            QWebElement element = elements.at(i);
            QUrl baseUrl = frame->url();

            QString mimeType("");
            QString name("");
            bool ok;

            int width = element.attribute("width").toInt(&ok);
            if (width == 0 || !ok)
                width = element.geometry().width() ? element.geometry().width() : 640;

            int heigth = element.attribute("height").toInt(&ok);
            if (heigth == 0 || !ok)
                heigth = element.geometry().height()? element.geometry().height(): 480;

            if(tagName == "embed" || tagName == "object" || tagName.contains("frame")){
                QStringList urlsToCheck = getUrls(baseUrl,element);
                foreach(QString url, urlsToCheck){
                    QString name = widgetNameFromUrl(url);
                    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(url);
                    if(!name.isEmpty() && !mimeType.isEmpty()){
                        UBWebKitUtils::HtmlObject obj(url, name, mimeType, tagName, width, heigth);
                        if (!htmlObjects.contains(obj))
                            htmlObjects << obj;
                    }
                }
            }


            QUrl relativeUrl = QUrl(element.attribute("src"));

            // for object
            if(relativeUrl.toString().isEmpty())
                relativeUrl =  QUrl(element.attribute("href"));

            // for object
            if(relativeUrl.toString().isEmpty())
                relativeUrl =  QUrl(element.attribute("data"));
            QString source = baseUrl.resolved(relativeUrl).toString();

            if (source.trimmed().length() == 0)
                continue;

            mimeType = element.attribute("type");
            if(mimeType.isEmpty()){
                QString tmp = source;
                tmp = (tmp.indexOf("?") != -1) ? tmp.left(tmp.indexOf("?")) : tmp;
                mimeType = UBFileSystemUtils::mimeTypeFromFileName(tmp);
            }
            if(mimeType.isEmpty())
                continue;

            name = element.attribute("alt");
            if(name.isEmpty())
                name = element.toPlainText();
            if(name.isEmpty())
                name = widgetNameFromUrl(source);
            if(name.isEmpty())
                continue;


            UBWebKitUtils::HtmlObject obj(source, name, mimeType, tagName, width, heigth);
            if (!htmlObjects.contains(obj))
                htmlObjects << obj;
        }
        if(htmlObjects.count() != 0)
            htmlObjects << UBWebKitUtils::HtmlObject(QString(),QString(),QString(),"separator",0,0);
    }

    return htmlObjects;
}


QString  UBWebKitUtils::widgetNameFromUrl(QString pObjectUrl)
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

QStringList UBWebKitUtils::getArgumentsValues(QString& string)
{
    QStringList result;
    QStringList arguments = string.split("?");
    foreach(QString eachValue, arguments){
        QStringList keyValueString = eachValue.split("=");
        if(keyValueString.count() == 2)
            result << keyValueString.at(1);
    }

    return result;
}

QStringList UBWebKitUtils::validUrl(QUrl& baseUrl,QStringList& list)
{
    QStringList result;
    foreach(QString eachString, list){
        QUrl url(eachString);
        if(url.isValid())
            result << eachString;
        if(baseUrl.resolved(url).isValid())
            result << baseUrl.resolved(url).toString();
    }
    return result;
}

QStringList UBWebKitUtils::getUrls(QUrl& baseUrl,QWebElement& element)
{
    QStringList result;
    QString src = element.attribute("src");
    if(!src.isEmpty()){
        QStringList values = getArgumentsValues(src);
        result << validUrl(baseUrl,values);
    }
    QString href = element.attribute("href");
    if(!href.isEmpty()){
        QStringList values = getArgumentsValues(href);
        result << validUrl(baseUrl,values);
    }
    return result;
}
