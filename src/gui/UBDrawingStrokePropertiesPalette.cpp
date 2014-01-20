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


#include "UBColorPicker.h"
#include "UBDrawingStrokePropertiesPalette.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"
#include "UBMainWindow.h"
#include "UBGlobals.h"


UBDrawingStrokePropertiesPalette::UBDrawingStrokePropertiesPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();

    // Color button
    mBtnColorPicker = new UBColorPickerButton(this);
    mBtnColorPicker->setIconSize(QSize(32,32));
    mBtnColorPicker->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    layout()->addWidget(mBtnColorPicker);
    connect(mBtnColorPicker, SIGNAL(clicked()), this, SLOT(onBtnSelectStrokeColor()));

    // Thickness buttons :

    mBtnStrokeThickness.append(new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyThickness0, this));
    mMapBtnStrokeThickness[mBtnStrokeThickness.last()] = 3;
    mBtnStrokeThickness.append(new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyThickness1, this));
    mMapBtnStrokeThickness[mBtnStrokeThickness.last()] = 5;
    mBtnStrokeThickness.append(new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyThickness2, this));
    mMapBtnStrokeThickness[mBtnStrokeThickness.last()] = 10;

    mButtonGroupStrokeThickness = new QButtonGroup(this);

    foreach(UBActionPaletteButton* button, mBtnStrokeThickness)
    {
        mButtonGroupStrokeThickness->addButton(button);
        layout()->addWidget(button);
        connect(button, SIGNAL(clicked()), this, SLOT(onBtnSelectThickness()));
    }

    // Style buttons :
    mMapBtnStrokeStyle[new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyStyleSolidLine, this)] = Qt::SolidLine;
    mMapBtnStrokeStyle[new UBActionPaletteButton(UBApplication::mainWindow->actionStrokePropertyStyleDotLine, this)] = Qt::DotLine;

    mButtonGroupStrokeStyle = new QButtonGroup(this);

    foreach(UBActionPaletteButton* button, mMapBtnStrokeStyle.keys())
    {
        mButtonGroupStrokeStyle->addButton(button);
        layout()->addWidget(button);
        connect(button, SIGNAL(clicked()), this, SLOT(onBtnSelectStyle()));
    }


    adjustSizeAndPosition();
}



UBDrawingStrokePropertiesPalette::~UBDrawingStrokePropertiesPalette()
{
    DELETEPTR(mBtnColorPicker);

    foreach(UBActionPaletteButton * button, mMapBtnStrokeThickness.keys()){
        DELETEPTR(button);
    }
    DELETEPTR(mButtonGroupStrokeThickness);

    foreach(UBActionPaletteButton * button, mMapBtnStrokeStyle.keys()){
        DELETEPTR(button);
    }
    DELETEPTR(mButtonGroupStrokeStyle);
}

void UBDrawingStrokePropertiesPalette::onBtnSelectStrokeColor()
{
    //setFocusPolicy(Qt::NoFocus);
    QColorDialog colorPicker(this);
    if ( colorPicker.exec() )
    {
        QColor selectedColor = colorPicker.selectedColor();

        UBApplication::boardController->shapeFactory().setStrokeColor(selectedColor);
        mBtnColorPicker->setColor(selectedColor); // udpate Color icon in palette.
    }
}

void UBDrawingStrokePropertiesPalette::onBtnSelectThickness()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button && mMapBtnStrokeThickness.contains(button))
    {
        int thickness = mMapBtnStrokeThickness.value(button);
        UBApplication::boardController->shapeFactory().setThickness(thickness);
    }
}

void UBDrawingStrokePropertiesPalette::onBtnSelectStyle()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button && mMapBtnStrokeStyle.contains(button))
    {
        Qt::PenStyle style = mMapBtnStrokeStyle.value(button);
        UBApplication::boardController->shapeFactory().setStrokeStyle(style);
    }
}

