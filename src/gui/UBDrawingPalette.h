#ifndef UBDRAWINGPALETTE_H
#define UBDRAWINGPALETTE_H

#include <map>
#include <QButtonGroup>

#include "UBAbstractSubPalette.h"
#include "UBEllipsePalette.h"

class UBDrawingPalette : public UBActionPalette
{
    Q_OBJECT

    public:

        enum Actions // The order here defines the order on the palette
        {
            EllipseAction,
            PolygonAction,
            FillAction
        };

        UBDrawingPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);

        virtual ~UBDrawingPalette();

        void mouseMoveEvent(QMouseEvent *event);

        void initPosition();
        void toggleEllipsePalette();
        void connectButtons();
        void setupSubPalettes(QWidget* parent, Qt::Orientation);
        void connectSubPalettes();
        void changeVisibility(bool checked);

        void initSubPalettesPosition(const QPointF& drawingPaletteTopLeft);
        void updateSubPalettesPosition(const QPoint &delta);

        static const int PRESS_DURATION;

    private slots:

        void drawingToolPressed();
        void drawingToolReleased();
        void updateActions();

    signals:
        void pressed(int);

    private:
        int mLastSelectedId;
        std::map<int, UBAbstractSubPalette*> mSubPalettes;

        QTime mActionButtonPressedTime;
        bool mPendingActionButtonPressed;
};


#endif // UBDRAWINGPALETTE_H
