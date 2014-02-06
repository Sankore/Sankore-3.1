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


#include "UBImageUtils.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

#include <QColor>
#include <QPainter>


QImage UBImageUtils::toMosaic(const QImage& srcImage, const QRectF& geometry)
{
    QImage destImage = QImage(geometry.width(), geometry.height(), QImage::Format_ARGB32_Premultiplied);
    destImage.fill(QColor(Qt::transparent).rgba());
    QPointF destPos = geometry.topLeft();
    QPainter painter(&destImage);
    while (destPos.y() < geometry.height())
    {
        while (destPos.x() < geometry.width())
        {
            painter.drawImage(destPos, srcImage);
            destPos += QPoint(srcImage.width(), 0);
        }
        destPos.setX(0);
        destPos += QPoint(0, srcImage.height());
    }
    painter.end();

    return destImage;
}


void UBImageUtils::drawGreyRect(QPainter *painter, const qreal& m11, bool darkBackground)
{
    UBGraphicsScene* scene = UBApplication::boardController->activeScene();

    if (scene)
    {
        QSize pageNominalSize = scene->nominalSize ();

          if (pageNominalSize.isValid())
          {
              qreal penWidth = 8.0 / m11;

              QRectF pageRect (pageNominalSize.width() / -2, pageNominalSize.height() / -2
                             , pageNominalSize.width(), pageNominalSize.height());

              pageRect.adjust (-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2);

              QColor docSizeColor;

              if (darkBackground)
                docSizeColor = UBSettings::documentSizeMarkColorDarkBackground;
              else
                docSizeColor = UBSettings::documentSizeMarkColorLightBackground;

              QPen pen (docSizeColor);
              pen.setWidth (penWidth);
              painter->setPen (pen);
              painter->drawRect (pageRect);
          }
    }
}
