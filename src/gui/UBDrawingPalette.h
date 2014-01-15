#ifndef UBDRAWINGPALETTE_H
#define UBDRAWINGPALETTE_H

#include <QMap>
#include <QButtonGroup>

#include "UBAbstractSubPalette.h"
#include "UBShapesPalette.h"

class UBDrawingPalette : public UBActionPalette
{
    Q_OBJECT

    public:

        enum Actions // The order here defines the order on the palette
        {
            ShapesAction,
            PolygonAction,
            FillAction
        };

        UBDrawingPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);

        virtual ~UBDrawingPalette();

        void mouseMoveEvent(QMouseEvent *event);

        void initPosition();
        void connectButtons();
        void setupSubPalettes(QWidget* parent, Qt::Orientation);
        void connectDrawingActions();
        void connectSubPalettes();
        void changeVisibility(bool checked);

        virtual void initSubPalettesPosition();
        void updateSubPalettesPosition(const QPoint &delta);

        void stackUnder(QWidget *w);

        static const int PRESS_DURATION;

    private slots:

        void drawingToolPressed();
        void drawingToolReleased();
        void updateActions();

    signals:
        void pressed(int);

    private:
        int mLastSelectedId;
        QMap<int, UBAbstractSubPalette*> mSubPalettes;

        QTime mActionButtonPressedTime;
        bool mPendingActionButtonPressed;

    public slots:
        void raise();
};


#endif // UBDRAWINGPALETTE_H
