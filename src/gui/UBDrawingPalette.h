#ifndef UBDRAWINGPALETTE_H
#define UBDRAWINGPALETTE_H

#include <QButtonGroup>

#include "UBAbstractSubPalette.h"
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
        void setupSubPalettes(QWidget* parent, Qt::Orientation);

        void initSubPalettesPosition(const QPointF& drawingPaletteTopLeft);
        void mousePressEvent(QMouseEvent *event);
        void updateCheckedId(int);
        void updateSubPalettesPosition(const QPoint &delta);

        static const int PRESS_DURATION;

    private slots:

        void drawingToolPressed();
        void drawingToolReleased();

    signals:
        void pressed(int);

    private:
        int mLastSelectedId;
        QList<UBAbstractSubPalette*> mSubPalettes;

        QTime mActionButtonPressedTime;
        bool mPendingActionButtonPressed;
};

#endif // UBDRAWINGPALETTE_H
