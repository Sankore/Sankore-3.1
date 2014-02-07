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

#ifndef UBIMAGEUTILS_H
#define UBIMAGEUTILS_H

#include <QRectF>
#include <QImage>

class UBImageUtils
{

    public:

        /*! \brief Transforms a QImage into a mosaic of the QImage using the geometry provided
         *  (wanted size and top-left position)
         *	\param[in] the image source used to paint the mosaic
         *	\param[in] the geometry used to know the area to paint on.
         *
         *  @pre geometry should give at least a size greater than the image source size to let
         *  the mosaic effect appear
         *
         *	\return the the result of the transformation painted on a QImage
         */
        static QImage toMosaic(const QImage& srcImage, const QRectF &geometry);


        static void drawGreyRect(QPainter *painter, const qreal& m11, bool darkBackground);

};




#endif // UBIMAGEUTILS_H
