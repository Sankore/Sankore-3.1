#ifndef UBDRAWINGPALETTE_H
#define UBDRAWINGPALETTE_H

#include <QButtonGroup>

#include "UBActionPalette.h"
#include "UBEllipsePalette.h"


class UBDrawingPalette : public UBActionPalette
{
    Q_OBJECT

    public:
        UBDrawingPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);

        virtual ~UBDrawingPalette();

        void mouseMoveEvent(QMouseEvent *event);

        void initPosition();
        void toggleEllipsePalette();
        void setupSubPalettes(Qt::Orientation);
        void updateSubPalettesPosition(QMouseEvent *event);

        static const int PRESS_DURATION;

    private slots:

        void drawingToolPressed();
        void drawingToolReleased();

    private:
        int mLastSelectedId;
        UBEllipsePalette* mEllipsePalette;

    signals:
        void drawingToolPressed(int tool);
        void drawingToolReleased(int tool);
};

#endif // UBDRAWINGPALETTE_H
