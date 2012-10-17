#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QColorDialog>

#include "UBCachePropertiesWidget.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "globals/UBGlobals.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

static QVector<UBGraphicsCache*> mCaches;

UBCachePropertiesWidget::UBCachePropertiesWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
  , mpLayout(NULL)
  , mpCachePropertiesLabel(NULL)
  , mpColorLabel(NULL)
  , mpShapeLabel(NULL)
  , mpWidthLabel(NULL)
  , mpColor(NULL)
  , mpSquareButton(NULL)
  , mpCircleButton(NULL)
  , mpCloseButton(NULL)
  , mpWidthSlider(NULL)
  , mpColorLayout(NULL)
  , mpShapeLayout(NULL)
  , mpSizeLayout(NULL)
  , mpCloseLayout(NULL)
  , mpProperties(NULL)
  , mpPropertiesLayout(NULL)
  , mpCurrentCache(NULL)
  , mKeepAspectRatio(true)
  , mOtherSliderUsed(false)
{
    setObjectName(name);

    SET_STYLE_SHEET();

    mName = "CachePropWidget";
    mVisibleState = false;
    mIconToLeft = QPixmap(":images/cache_open.png");
    mIconToRight = QPixmap(":images/cache_close.png");
    setContentsMargins(10, 10, 10, 10);

    // Build the UI
    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    // Title
    mpCachePropertiesLabel = new QLabel(tr("Cache Properties"), this);
    mpCachePropertiesLabel->setObjectName("DockPaletteWidgetTitle");
    mpLayout->addWidget(mpCachePropertiesLabel, 0);

    // Properties Box
    mpProperties = new QWidget(this);
    mpProperties->setObjectName("DockPaletteWidgetBox");
    mpLayout->addWidget(mpProperties, 1);
    mpPropertiesLayout = new QVBoxLayout();
    mpProperties->setLayout(mpPropertiesLayout);


    // Color and Alpha
    mpColorLayout = new QHBoxLayout();
    mpColorLabel = new QLabel(tr("Color:"), mpProperties);
    mpColor = new QPushButton(mpProperties);
    mpColor->setObjectName("DockPaletteWidgetButton");
    updateCacheColor(Qt::black);
    mpColorLayout->addWidget(mpColorLabel, 0);
    mpColorLayout->addWidget(mpColor, 0);

    mpAlphaLabel = new QLabel(tr("Alpha:"), mpProperties);
    mpAplhaSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpAplhaSlider->setMinimumHeight(20);
    mpAplhaSlider->setMinimum(0);
    mpAplhaSlider->setMaximum(255);
    mpColorLayout->addWidget(mpAlphaLabel, 0);
    mpColorLayout->addWidget(mpAplhaSlider, 1);

    mpPropertiesLayout->addLayout(mpColorLayout, 0);

    // Shape
    mpShapeLayout = new QHBoxLayout();
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
    mpSizeLayout = new QVBoxLayout();

    mpGeometryLabel = new QLabel(tr("Geometry:"), mpProperties);
    mpSizeLayout->addWidget(mpGeometryLabel, 1);

    mKeepAspectRatio = UBSettings::settings()->cacheKeepAspectRatio->get().toBool();

    mpWidthLayout = new QHBoxLayout();
    mpWidthLabel = new QLabel(tr("Width: "), mpProperties);
    mpWidthSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpWidthSlider->setMinimumHeight(20);
    mpWidthSlider->setMinimum(MIN_SHAPE_WIDTH);
    mpWidthSlider->setMaximum(MAX_SHAPE_WIDTH);
    mpWidthSlider->setValue(MIN_SHAPE_WIDTH);
    mpWidthLayout->addWidget(mpWidthLabel, 0);
    mpWidthLayout->addWidget(mpWidthSlider, 1);
    mpSizeLayout->addLayout(mpWidthLayout, 0);

    mpHeightLayout = new QHBoxLayout();
    mpHeightLabel = new QLabel(tr("Height:"), mpProperties);
    mpHeightSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpHeightSlider->setMinimumHeight(20);
    mpHeightSlider->setMinimum(MIN_SHAPE_WIDTH);
    mpHeightSlider->setMaximum(MAX_SHAPE_WIDTH);
    mpHeightSlider->setValue(MIN_SHAPE_WIDTH);
    mpHeightLayout->addWidget(mpHeightLabel, 0);
    mpHeightLayout->addWidget(mpHeightSlider, 1);
    mpSizeLayout->addLayout(mpHeightLayout, 0);

    mpKeepAspectRatioCheckbox = new QCheckBox(tr("Keep proportions"), mpProperties);
    mpKeepAspectRatioCheckbox->setTristate(false);
    mpKeepAspectRatioCheckbox->setChecked(mKeepAspectRatio);

    mpSizeLayout->addWidget(mpKeepAspectRatioCheckbox, 0);

    mpPropertiesLayout->addLayout(mpSizeLayout, 0);

    // Close
    mpCloseLayout =  new QHBoxLayout();
    mpCloseButton = new QPushButton(tr("Close"), mpProperties);
    mpCloseButton->setObjectName("DockPaletteWidgetButton");
    mpCloseLayout->addWidget(mpCloseButton, 0);
    mpCloseLayout->addStretch(1);
    mpPropertiesLayout->addLayout(mpCloseLayout, 0);

    // Fill the empty space
    mpPropertiesLayout->addStretch(1);

    // Connect signals / slots
    connect(mpCloseButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
    connect(mpColor, SIGNAL(clicked()), this, SLOT(onColorClicked()));
    connect(mpCircleButton, SIGNAL(clicked()), this, SLOT(updateShapeButtons()));
    connect(mpSquareButton, SIGNAL(clicked()), this, SLOT(updateShapeButtons()));
    connect(mpWidthSlider, SIGNAL(valueChanged(int)), this, SLOT(onWidthChanged(int)));
    connect(mpHeightSlider, SIGNAL(valueChanged(int)), this, SLOT(onHeightChanged(int)));
    connect(mpKeepAspectRatioCheckbox, SIGNAL(stateChanged(int)), this, SLOT(onKeepAspectRatioChanged(int)));    
    connect(UBApplication::boardController, SIGNAL(pageChanged()), this, SLOT(updateCurrentCache()));
    connect(UBApplication::boardController, SIGNAL(cacheEnabled()), this, SLOT(onCacheEnabled()));

    mOldHoleSize = QSize(mpWidthSlider->value(), mpHeightSlider->value());
}

UBCachePropertiesWidget::~UBCachePropertiesWidget()
{
    if(NULL != mpCachePropertiesLabel)
    {
        delete mpCachePropertiesLabel;
        mpCachePropertiesLabel = NULL;
    }
    if(NULL != mpColorLabel)
    {
        delete mpColorLabel;
        mpColorLabel = NULL;
    }
    if(NULL != mpShapeLabel)
    {
        delete mpShapeLabel;
        mpShapeLabel = NULL;
    }
    if(NULL != mpWidthLabel)
    {
        delete mpWidthLabel;
        mpWidthLabel = NULL;
    }
    if(NULL != mpColor)
    {
        delete mpColor;
        mpColor = NULL;
    }
    if(NULL != mpSquareButton)
    {
        delete mpSquareButton;
        mpSquareButton = NULL;
    }
    if(NULL != mpCircleButton)
    {
        delete mpCircleButton;
        mpCircleButton = NULL;
    }
    if(NULL != mpCloseButton)
    {
        delete mpCloseButton;
        mpCloseButton = NULL;
    }
    if(NULL != mpWidthSlider)
    {
        delete mpWidthSlider;
        mpWidthSlider = NULL;
    }
    if(NULL != mpColorLayout)
    {
        delete mpColorLayout;
        mpColorLayout = NULL;
    }
    if(NULL != mpShapeLayout)
    {
        delete mpShapeLayout;
        mpShapeLayout = NULL;
    }
    if(NULL != mpSizeLayout)
    {
        delete mpSizeLayout;
        mpSizeLayout = NULL;
    }
    if(NULL != mpCloseLayout)
    {
        delete mpCloseLayout;
        mpCloseLayout = NULL;
    }
    if(NULL != mpPropertiesLayout)
    {
        delete mpPropertiesLayout;
        mpPropertiesLayout = NULL;
    }
    if(NULL != mpProperties)
    {
        delete mpProperties;
        mpProperties = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
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

void UBCachePropertiesWidget::updateCacheColor(QColor color)
{
    mActualColor = color;

    //  Update the color on the color button
    QPixmap pix(32, 32);
    QPainter p;

    p.begin(&pix);

    p.setBackground(Qt::transparent);
    p.setBrush(color);      // The current color
    p.drawRect(0, 0, 32, 32);

    p.end();

    mpColor->setIcon(QIcon(pix));

    if(NULL != mpCurrentCache)
    {
        mpCurrentCache->setMaskColor(mActualColor);
    }
}

void UBCachePropertiesWidget::onColorClicked()
{
    // Show the color picker
    QColor newColor = QColorDialog::getColor(mActualColor,this);
    updateCacheColor(newColor);
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
                {
                    mCaches.append(mpCurrentCache);
                }

                // Update the values of the cache properties
                mpWidthSlider->setValue(mpCurrentCache->holeWidth());
                mpHeightSlider->setValue(mpCurrentCache->holeHeight());
                updateCacheColor(mpCurrentCache->maskColor());
                switch(mpCurrentCache->maskshape())
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
                mpWidthSlider->setValue(mpWidthSlider->value()*newSize/mOldHoleSize.width());
            }
             mOldHoleSize.setWidth(mpWidthSlider->value());
        }
        else
            mpCurrentCache->setHoleHeight(newSize);
        
        mOldHoleSize.setHeight(newSize);
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

