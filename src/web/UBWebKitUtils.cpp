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
        
            bool ok;

            int width = element.attribute("width").toInt(&ok);
            if (width == 0 || !ok)
            {
                qDebug() << "Width is not defined in pixel. 640 will be used";
                width = 640;
            }

            int heigth = element.attribute("height").toInt(&ok);
            if (heigth == 0 || !ok)
            {
                qDebug() << "Height is not defined in pixel. 480 will be used";
                heigth = 480;
            }

            QUrl baseUrl = frame->url();
            QUrl relativeUrl = QUrl(element.attribute("src"));

            QString source = baseUrl.resolved(relativeUrl).toString();

            if (source.trimmed().length() == 0)
                continue;

            UBWebKitUtils::HtmlObject obj(source, tagName, width, heigth);
            if (!htmlObjects.contains(obj))
                htmlObjects << obj;
        }
    }

    return htmlObjects;
}


