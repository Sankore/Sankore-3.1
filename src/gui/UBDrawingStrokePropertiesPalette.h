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


#ifndef UBDRAWINGSTROKEPROPERTIESPALETTE_H
#define UBDRAWINGSTROKEPROPERTIESPALETTE_H

#include "UBAbstractSubPalette.h"
#include <QPen>

class UBDrawingStrokePropertiesPalette : public UBAbstractSubPalette
{
    Q_OBJECT

public:
    UBDrawingStrokePropertiesPalette(Qt::Orientation orient, QWidget *parent = 0);
    virtual ~UBDrawingStrokePropertiesPalette();

private slots:
    void onBtnSelectStrokeColor();
    void onBtnSelectThickness();
    void onBtnSelectStyle();

private:
    UBColorPickerButton * mBtnColorPicker;
    QList<UBActionPaletteButton*> mBtnStrokeThickness;
    QMap<UBActionPaletteButton*, int> mMapBtnStrokeThickness;
    QButtonGroup * mButtonGroupStrokeThickness;
    QMap<UBActionPaletteButton*, Qt::PenStyle> mMapBtnStrokeStyle;
    QButtonGroup * mButtonGroupStrokeStyle;
};

#endif // UBDRAWINGSTROKEPROPERTIESPALETTE_H
