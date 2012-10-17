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

#define MIN_SHAPE_WIDTH     100
#define MAX_SHAPE_WIDTH     400

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
    void updateCacheColor(QColor color);
    void onColorClicked();
    void updateShapeButtons();
    void onWidthChanged(int newSize);
    void onHeightChanged(int newSize);
    void onKeepAspectRatioChanged(int state);
    void onCacheEnabled();
    void onModeChanged(int mode);

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
    QCheckBox *mpKeepAspectRatioCheckbox;
    QPushButton* mpColor;
    QPushButton* mpSquareButton;
    QPushButton* mpCircleButton;
    QPushButton* mpCloseButton;
    QSlider *mpAplhaSlider;
    QSlider* mpWidthSlider;
    QSlider* mpHeightSlider;
    QHBoxLayout* mpColorLayout;
    QHBoxLayout* mpShapeLayout;
    QHBoxLayout *mpWidthLayout;
    QHBoxLayout *mpHeightLayout;
    QHBoxLayout* mpCloseLayout;
    QWidget* mpProperties;
    QVBoxLayout* mpSizeLayout;
    QVBoxLayout* mpPropertiesLayout;
    QVBoxLayout *mpModeLayout;
    QComboBox *mpModeComboBox;
    QColor mActualColor;
    eMaskShape mActualShape;
    UBGraphicsCache* mpCurrentCache;
    bool mKeepAspectRatio;
    bool mOtherSliderUsed;
    QSize mOldHoleSize;
};

#endif // UBCACHEPROPERTIESWIDGET_H
