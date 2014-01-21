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


#include "UBDrawingFillPropertiesPalette.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

UBDrawingFillPropertiesPalette::UBDrawingFillPropertiesPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();

    // ColorPicker button
    mBtnColorPicker = new UBColorPickerButton(this);
    mBtnColorPicker->setToolTip(tr("Select and set fill color"));
    mBtnColorPicker->setColor(UBApplication::boardController->shapeFactory().fillFirstColor());
    mBtnColorPicker->setIconSize(QSize(32,32));
    mBtnColorPicker->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    layout()->addWidget(mBtnColorPicker);
    connect(mBtnColorPicker, SIGNAL(clicked()), this, SLOT(onBtnSelectFillColor()));

    // Transparent color button
    mBtnColorTransparent = new UBColorPickerButton(this);
    mBtnColorTransparent->setToolTip(tr("Set transparent fill color"));
    mBtnColorTransparent->setColor(Qt::transparent);
    mBtnColorTransparent->setIconSize(QSize(32,32));
    mBtnColorTransparent->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    layout()->addWidget(mBtnColorTransparent);
    connect(mBtnColorTransparent, SIGNAL(clicked()), this, SLOT(onBtnFillColorTransparent()));


    adjustSizeAndPosition();
}

UBDrawingFillPropertiesPalette::~UBDrawingFillPropertiesPalette()
{

}

void UBDrawingFillPropertiesPalette::onBtnSelectFillColor()
{
    QColorDialog colorPicker(this);
    if ( colorPicker.exec() )
    {
        QColor selectedColor = colorPicker.selectedColor();

        UBApplication::boardController->shapeFactory().setFillingColor(selectedColor);
        mBtnColorPicker->setColor(selectedColor); // udpate Color icon in palette.
    }
}

void UBDrawingFillPropertiesPalette::onBtnFillColorTransparent()
{
    UBApplication::boardController->shapeFactory().setFillingColor(Qt::transparent);
}
