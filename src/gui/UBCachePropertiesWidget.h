#ifndef UBCACHEPROPERTIESWIDGET_H
#define UBCACHEPROPERTIESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVector>

#include "customWidgets/UBDockPaletteWidget.h"
#include "tools/UBGraphicsCache.h"

#define MIN_SHAPE_WIDTH     100
#define MAX_SHAPE_WIDTH     400

class UBCachePreviewWidget : public QWidget
{
public:
    UBCachePreviewWidget(QWidget *parent = NULL);
    
    void setHoleSize(QSize size);
    void setMaskColor(QColor color);
private:
    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QSize mHoleSize;
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
};

#endif // UBCACHEPROPERTIESWIDGET_H
