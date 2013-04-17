/*
 * Copyright (C) 2012 Webdoc SA
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



#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QColorDialog>

#include "UBCachePropertiesWidget.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "globals/UBGlobals.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

static QVector<UBGraphicsCache*> mCaches;

UBCachePreviewWidget::UBCachePreviewWidget(QWidget *parent)
    : QWidget(parent)
    , mHoleSize(QSize())
    , mShape(eMaskShape_Circle)
{
}

void UBCachePreviewWidget::setHoleSize(QSize size)
{
    mHoleSize = size;
    update();
}

void UBCachePreviewWidget::setShape(eMaskShape shape)
{
    mShape = shape;
}

void UBCachePreviewWidget::setMaskColor(QColor color)
{
    mMaskColor = color;
    update();
}

QSize UBCachePreviewWidget::sizeHint() const
{
    return size();
}

void UBCachePreviewWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(mMaskColor);
    painter.setBrush(mMaskColor);
    painter.drawRect(rect());
    painter.setBrush(QColor(Qt::white));

    UBBoardView *view = UBApplication::boardController->controlView();
    qreal scaleRatio = static_cast<qreal>(rect().width())/static_cast<qreal>(view->width());
    qreal holeWidth = mHoleSize.width()*scaleRatio;
    qreal holeHeight = mHoleSize.height()*scaleRatio;

    if (eMaskShape_Circle == mShape)
        painter.drawEllipse(rect().center(), static_cast<int>(holeWidth/2), static_cast<int>(holeHeight/2));

    if (eMaskShap_Rectangle == mShape)
        painter.drawRect(rect().center().x() - holeWidth/2, rect().center().y() - holeHeight/2, holeWidth, holeHeight);
}

void UBCachePreviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    UBBoardView *view = UBApplication::boardController->controlView();
    qreal aspectRatio = static_cast<qreal>(view->height())/static_cast<qreal>(view->width());

    QSize newSize(width(),width()*aspectRatio);
    updateGeometry();
    resize(newSize);


    QWidget::resizeEvent(event);
}

UBCachePropertiesWidget::UBCachePropertiesWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
  , mpLayout(NULL)
  , mpCachePropertiesLabel(NULL)
  , mpColorLabel(NULL)
  , mpShapeLabel(NULL)
  , mpWidthLabel(NULL)
  , mpSelectColorButton(NULL)
  , mpSquareButton(NULL)
  , mpCircleButton(NULL)
  , mpCloseButton(NULL)
  , mpWidthSlider(NULL)
  , mpColorLayout(NULL)
  , mpShapeLayout(NULL)
  , mpCloseLayout(NULL)
  , mpProperties(NULL)
  , mpSizeLayout(NULL)
  , mpPropertiesLayout(NULL)
  , mActualShape(eMaskShape_Circle)
  , mpCurrentCache(NULL)
  , mKeepAspectRatio(true)
  , mOtherSliderUsed(false)
{
    setObjectName(name);

    SET_STYLE_SHEET();

    mName = "CachePropWidget";
    mVisibleState = false;
    setContentsMargins(10, 10, 10, 10);

    // Build the UI
    mpLayout = new QVBoxLayout(0);
    setLayout(mpLayout);

    // Title
    mpCachePropertiesLabel = new QLabel(tr("Cache Properties"), this);
    mpCachePropertiesLabel->setObjectName("DockPaletteWidgetTitle");
    mpLayout->addWidget(mpCachePropertiesLabel, 0);

    // Properties Box
    mpProperties = new QWidget(0);
    mpProperties->setObjectName("DockPaletteWidgetBox");
    mpLayout->addWidget(mpProperties, 1);
    mpPropertiesLayout = new QVBoxLayout(0);
    mpProperties->setLayout(mpPropertiesLayout);

    // Color and Alpha
    mpColorLayout = new QHBoxLayout(mpProperties);
    mpColorLabel = new QLabel(tr("Color:"), mpProperties);
    mpSelectColorButton = new QPushButton(mpProperties);
    mpSelectColorButton->setObjectName("DockPaletteWidgetButton");
    mpColorLayout->addWidget(mpColorLabel, 0);
    mpColorLayout->addWidget(mpSelectColorButton, 0);
    mpAlphaLabel = new QLabel(tr("Alpha:"), mpProperties);
    mpAplhaSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpAplhaSlider->setMinimumHeight(20);
    mpAplhaSlider->setMinimum(178);
    mpAplhaSlider->setMaximum(255);
    mpAplhaSlider->setValue(255);
    mpColorLayout->addWidget(mpAlphaLabel, 0);
    mpColorLayout->addWidget(mpAplhaSlider, 1);
    mpPropertiesLayout->addLayout(mpColorLayout, 0);

    // Shape
    mpShapeLayout = new QHBoxLayout(0);
    mpShapeLabel = new QLabel(tr("Shape:"), mpProperties);
    mpSquareButton = new QPushButton(mpProperties);
    mpSquareButton->setIcon(QIcon(":images/cache_square.png"));
    mpSquareButton->setObjectName("DockPaletteWidgetButton");
    mpSquareButton->setCheckable(true);
    mpCircleButton = new QPushButton(mpProperties);
    mpCircleButton->setIcon(QIcon(":images/cache_circle.png"));
    mpCircleButton->setObjectName("DockPaletteWidgetButton");
    mpCircleButton->setCheckable(true);
    mpShapeLayout->addWidget(mpShapeLabel, 0);
    mpShapeLayout->addWidget(mpSquareButton, 0);
    mpShapeLayout->addWidget(mpCircleButton, 0);
    mpShapeLayout->addStretch(1);
    mpPropertiesLayout->addLayout(mpShapeLayout, 0);
    mpCircleButton->setChecked(true);

    // Shape Size
    connect(UBApplication::boardController->controlView(), SIGNAL(resized(QResizeEvent *)), this, SLOT(onControlViewResized(QResizeEvent *)));
    connect(UBApplication::boardController, SIGNAL(zoomChanged(qreal)), this, SLOT(onZoomChanged(qreal)));
    minimumShapeSize = QSize(100,100);

    mpSizeLayout = new QVBoxLayout(0);
    mpGeometryLabel = new QLabel(tr("Geometry:"), mpProperties);
    mpSizeLayout->addWidget(mpGeometryLabel, 1);
    mKeepAspectRatio = UBSettings::settings()->cacheKeepAspectRatio->get().toBool();

    mpWidthLabel = new QLabel(tr("Width: "), mpProperties);
    mpWidthSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpWidthSlider->setMinimumHeight(20);
    mpWidthSlider->setMinimum(minimumShapeSize.width());
    mpWidthSlider->setMaximum(maximumShapeSize.width());
    mpWidthSlider->setValue(minimumShapeSize.width());
    mpSizeLayout->addWidget(mpWidthLabel, 0);
    mpSizeLayout->addWidget(mpWidthSlider, 1);

    mpHeightLabel = new QLabel(tr("Height:"), mpProperties);
    mpHeightSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpHeightSlider->setMinimumHeight(20);
    mpHeightSlider->setMinimum(minimumShapeSize.height());
    mpHeightSlider->setMaximum(maximumShapeSize.height());
    mpHeightSlider->setValue(minimumShapeSize.height());
    mpSizeLayout->addWidget(mpHeightLabel, 0);
    mpSizeLayout->addWidget(mpHeightSlider, 1);

    mpKeepAspectRatioCheckbox = new QCheckBox(tr("Keep proportions"), mpProperties);
    mpKeepAspectRatioCheckbox->setTristate(false);
    mpKeepAspectRatioCheckbox->setChecked(mKeepAspectRatio);
    mpSizeLayout->addWidget(mpKeepAspectRatioCheckbox, 0);
    mpPropertiesLayout->addLayout(mpSizeLayout, 0);

    // Mode
    mpModeLayout = new QVBoxLayout(0);
    mpModeLabel = new QLabel(tr("Mode:"), mpProperties);
    mpModeComboBox = new QComboBox(mpProperties);
    mpModeComboBox->setObjectName("CacheModeSelectionComboBox");
    mpModeComboBox->addItem(tr("On Click"), UBGraphicsCache::OnClick);
    mpModeComboBox->addItem(tr("Persistent"), UBGraphicsCache::Persistent);
    mpModeLayout->addWidget(mpModeLabel);
    mpModeLayout->addWidget(mpModeComboBox);

    mpPropertiesLayout->addLayout(mpModeLayout, 0);

    // Preview
    mpPreviewLayout = new QVBoxLayout(0);
    mpPreviewLabel = new QLabel(tr("Preview:"), mpProperties);
    mpPreviewWidget = new UBCachePreviewWidget(mpProperties);
    QVBoxLayout *previewWidgetLayout = new QVBoxLayout(0);
    previewWidgetLayout->addStretch(1);
    mpPreviewWidget->setHoleSize(mOldHoleSize);
    mpPreviewLayout->addWidget(mpPreviewLabel);
    mpPreviewLayout->addWidget(mpPreviewWidget,0);
    mpPropertiesLayout->addLayout(mpPreviewLayout,0);

    // Close
    mpCloseLayout =  new QHBoxLayout(0);
    mpCloseButton = new QPushButton(tr("Close cache"), mpProperties);
    mpCloseButton->setObjectName("DockPaletteWidgetButton");
    mpCloseLayout->addWidget(mpCloseButton, 0);
    mpCloseLayout->addStretch(1);
    mpPropertiesLayout->addLayout(mpCloseLayout, 0);

    // Fill the empty space
    mpPropertiesLayout->addStretch(1);

    // Connect signals / slots
    connect(mpCloseButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
    connect(mpSelectColorButton, SIGNAL(clicked()), this, SLOT(onColorClicked()));
    connect(mpCircleButton, SIGNAL(clicked()), this, SLOT(updateShapeButtons()));
    connect(mpSquareButton, SIGNAL(clicked()), this, SLOT(updateShapeButtons()));
    connect(mpWidthSlider, SIGNAL(valueChanged(int)), this, SLOT(onWidthChanged(int)));
    connect(mpHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onHeightChanged(int)));
    connect(mpKeepAspectRatioCheckbox, SIGNAL(stateChanged(int)), this, SLOT(onKeepAspectRatioChanged(int)));
    connect(mpModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));
    connect(mpAplhaSlider, SIGNAL(valueChanged(int)), this, SLOT(onAlphaChanged(int)));
    connect(UBApplication::boardController, SIGNAL(pageChanged()), this, SLOT(updateCurrentCache()));
    connect(UBApplication::boardController, SIGNAL(cacheEnabled()), this, SLOT(onCacheEnabled()));

    mOldHoleSize = QSize(mpWidthSlider->value(), mpHeightSlider->value());
}

UBCachePropertiesWidget::~UBCachePropertiesWidget()
{

}

void UBCachePropertiesWidget::onCloseClicked()
{
    if(!mCaches.empty())
    {
        // Remove the current cache from the list
        mCaches.remove(mCaches.indexOf(mpCurrentCache));

        // Remove the cache from the board
        UBApplication::boardController->activeScene()->removeItem(mpCurrentCache);
        mpCurrentCache = NULL;

        if(mCaches.empty())
        {
            emit cacheListEmpty();
        }

        emit hideTab(this);
    }
}

void UBCachePropertiesWidget::syncCacheColor(QColor color)
{
    mActualColor = color;
    mpAplhaSlider->setValue(color.alpha());

    //  Update the color on the color button
    QPixmap pix(32, 32);
    QPainter p;

    p.begin(&pix);

    p.setBackground(Qt::transparent);
    p.setBrush(color);      // The current color
    p.drawRect(0, 0, 32, 32);

    p.end();

    mpSelectColorButton->setIcon(QIcon(pix));

    if(NULL != mpCurrentCache)
    {
        mpCurrentCache->setMaskColor(mActualColor);
    }

    mpPreviewWidget->setMaskColor(mActualColor);
    UBSettings::settings()->cacheColor->set(QString("%1 %2 %3 %4").arg(mActualColor.red()).arg(mActualColor.green()).arg(mActualColor.blue()).arg(mActualColor.alpha()));
}

void UBCachePropertiesWidget::onColorClicked()
{
    // Show the color picker
    QColor newColor = QColorDialog::getColor(mActualColor,this);
    if (QColor::Invalid	 != newColor.spec())
        syncCacheColor(newColor);
}

void UBCachePropertiesWidget::updateShapeButtons()
{
    if(mpCircleButton->hasFocus())
    {
        mActualShape = eMaskShape_Circle;
        mpSquareButton->setChecked(false);
    }
    else if(mpSquareButton->hasFocus())
    {
        mActualShape = eMaskShap_Rectangle;
        mpCircleButton->setChecked(false);
    }

    if(NULL != mpCurrentCache)
    {
        mpCurrentCache->setMaskShape(mActualShape);
    }

    if (NULL != mpPreviewWidget)
    {
        mpPreviewWidget->setShape(mActualShape);
    }
}

void UBCachePropertiesWidget::updateCurrentCache()
{
    bool isBoardMode = false;
    // this widget can work only on Board mode
    if( UBApplication::applicationController != NULL )
    {
        // if app controller is available, and current mode is Board, and no show desktop, than all ok, just process
        if( UBApplication::applicationController->displayMode() == UBApplicationController::Board &&
            !UBApplication::applicationController->isShowingDesktop())
            isBoardMode = true;
    }
    // if app controller == null, than we do not know what mode now, so just process
    else
        isBoardMode = true;

    if(isBoardMode)
    {
        // Get the current page cache
        QList<QGraphicsItem*> items = UBApplication::boardController->activeScene()->items();
        foreach(QGraphicsItem* it, items)
        {
            if("Cache" == it->data(Qt::UserRole).toString())
            {
                setEnabled(true);
                emit showTab(this);
                mpCurrentCache = dynamic_cast<UBGraphicsCache*>(it);
                if((NULL != mpCurrentCache) && (!mCaches.contains(mpCurrentCache)))
                    mCaches.append(mpCurrentCache);
                else
                    return

                // Update the values of the cache properties
                mpWidthSlider->setValue(mpCurrentCache->holeWidth());
                mpHeightSlider->setValue(mpCurrentCache->holeHeight());
                syncCacheColor(mpCurrentCache->maskColor());
                mpPreviewWidget->setHoleSize(QSize(mpWidthSlider->value(), mpHeightSlider->value()));
                mpCurrentCache->setMode(UBSettings::settings()->cacheMode->get().toInt());

                mActualShape = mpCurrentCache->maskshape();
                switch(mActualShape)
                {
                    case eMaskShape_Circle:
                        mpCircleButton->setChecked(true);
                        mpSquareButton->setChecked(false);
                        break;
                    case eMaskShap_Rectangle:
                        mpCircleButton->setChecked(false);
                        mpSquareButton->setChecked(true);
                        break;
                }


                mpPreviewWidget->setShape(mActualShape);
                return;

            }
        }
    }

    // If we fall here, that means:
    // 1 - that this page has no cache
    // 2 - we are not in Board mode
    // 3 - we are in Board mode, but show desktop (as really - Desktop mode)
    emit hideTab(this);
    mpCurrentCache = NULL;
    setDisabled(true);
}

void UBCachePropertiesWidget::onWidthChanged(int newSize)
{
    if(NULL != mpCurrentCache)
    {
        if(mKeepAspectRatio)
        {
            if(!mOtherSliderUsed)
            {
                mOtherSliderUsed = true;
                mpHeightSlider->setValue(mpHeightSlider->value()*newSize/mOldHoleSize.width());
            }
            mOldHoleSize.setHeight(mpHeightSlider->value());
        }
        else
            mpCurrentCache->setHoleWidth(newSize);

        mOldHoleSize.setWidth(newSize);
        mpPreviewWidget->setHoleSize(mOldHoleSize);
        mpCurrentCache->setHoleSize(mOldHoleSize);
        mOtherSliderUsed = false;
    }
}

void UBCachePropertiesWidget::onHeightChanged(int newSize)
{
    if(NULL != mpCurrentCache)
    {
        if (mKeepAspectRatio)
        {
            if(!mOtherSliderUsed)
            {
                mOtherSliderUsed = true;
                mpWidthSlider->setValue(mpWidthSlider->value()*newSize/mOldHoleSize.height());
            }
             mOldHoleSize.setWidth(mpWidthSlider->value());
        }
        else
            mpCurrentCache->setHoleHeight(newSize);

        mOldHoleSize.setHeight(newSize);
        mpPreviewWidget->setHoleSize(mOldHoleSize);
        mpCurrentCache->setHoleSize(mOldHoleSize);
        mOtherSliderUsed = false;
    }
}


void UBCachePropertiesWidget::onKeepAspectRatioChanged(int state)
{
    Qt::CheckState cur_state = static_cast<Qt::CheckState>(state);
    mKeepAspectRatio = Qt::Checked == cur_state;

    UBSettings::settings()->cacheKeepAspectRatio->set(mKeepAspectRatio);

}

void UBCachePropertiesWidget::onCacheEnabled()
{
    emit showTab(this);
}

void UBCachePropertiesWidget::onModeChanged(int mode)
{
    mpCurrentCache->setMode(mode);
    UBSettings::settings()->cacheMode->set(mode);
}

void UBCachePropertiesWidget::onAlphaChanged(int alpha)
{
    mActualColor.setAlpha(alpha);
    syncCacheColor(mActualColor);
}

void UBCachePropertiesWidget::onControlViewResized(QResizeEvent *event)
{
    Q_UNUSED(event);
    maximumShapeSize = UBApplication::boardController->controlView()->size();
    if (mKeepAspectRatio)
    {
        int maxSize = qMax(maximumShapeSize.width(), maximumShapeSize.height());
        mpWidthSlider->setMaximum(maxSize);
        mpHeightSlider->setMaximum(maxSize);
    }
    else
    {
        mpWidthSlider->setMaximum(maximumShapeSize.width());
        mpHeightSlider->setMaximum(maximumShapeSize.height());
    }
}

void UBCachePropertiesWidget::onZoomChanged(qreal newZoom)
{
    Q_UNUSED(newZoom)

    mpWidthSlider->setMaximum(maximumShapeSize.width());
    mpHeightSlider->setMaximum(maximumShapeSize.height());
    if (mpCurrentCache)
        mpCurrentCache->setHoleSize(QSize(mpWidthSlider->value(), mpHeightSlider->value()));
}
