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



#ifndef UBCACHEPROPERTIESWIDGET_H
#define UBCACHEPROPERTIESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVector>

#include "UBDockPaletteWidget.h"
#include "tools/UBGraphicsCache.h"

class UBCachePreviewWidget : public QWidget
{
public:
    UBCachePreviewWidget(QWidget *parent = NULL);
    
    void setHoleSize(QSize size);
    void setShape(eMaskShape shape);
    void setMaskColor(QColor color);

private:
    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QSize mHoleSize;
    eMaskShape mShape;
    QColor mMaskColor;
};

class UBCachePropertiesWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBCachePropertiesWidget(QWidget* parent=0, const char* name="UBCachePropertiesWidget");
    ~UBCachePropertiesWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return mode == eUBDockPaletteWidget_BOARD;
    }

    QPixmap iconToLeft() const {return QPixmap(":images/cache_open.png");}
    QPixmap iconToRight() const {return QPixmap(":images/cache_open.png");}

public slots:
    void updateCurrentCache();

signals:
    void cacheListEmpty();

private slots:
    void onCloseClicked();
    void syncCacheColor(QColor color);
    void onColorClicked();
    void updateShapeButtons();
    void onWidthChanged(int newSize);
    void onHeightChanged(int newSize);
    void onKeepAspectRatioChanged(int state);
    void onCacheEnabled();
    void onModeChanged(int mode);
    void onAlphaChanged(int alpha);
    void onControlViewResized(QResizeEvent *event);
    void onZoomChanged(qreal newZoom);

private:
    QVBoxLayout* mpLayout;
    QLabel* mpCachePropertiesLabel;
    QLabel* mpColorLabel;
    QLabel* mpAlphaLabel;
    QLabel* mpShapeLabel;
    QLabel *mpGeometryLabel;
    QLabel *mpWidthLabel;
    QLabel *mpHeightLabel;
    QLabel *mpModeLabel;
    QLabel *mpPreviewLabel;
    QCheckBox *mpKeepAspectRatioCheckbox;
    QPushButton* mpSelectColorButton;
    QPushButton* mpSquareButton;
    QPushButton* mpCircleButton;
    QPushButton* mpCloseButton;
    QSlider *mpAplhaSlider;
    QSlider* mpWidthSlider;
    QSlider* mpHeightSlider;
    QHBoxLayout* mpColorLayout;
    QHBoxLayout* mpShapeLayout;
    QHBoxLayout* mpCloseLayout;
    QWidget* mpProperties;
    UBCachePreviewWidget *mpPreviewWidget;
    QVBoxLayout* mpSizeLayout;
    QVBoxLayout* mpPropertiesLayout;
    QVBoxLayout *mpModeLayout;
    QVBoxLayout *mpPreviewLayout;
    QComboBox *mpModeComboBox;
    QColor mActualColor;
    eMaskShape mActualShape;
    UBGraphicsCache* mpCurrentCache;
    bool mKeepAspectRatio;
    bool mOtherSliderUsed;
    QSize mOldHoleSize;
    QSize minimumShapeSize;
    QSize maximumShapeSize;
};

#endif // UBCACHEPROPERTIESWIDGET_H
