#ifndef UBDRAWINGPALETTE_H
#define UBDRAWINGPALETTE_H

#include <QMap>
#include <QButtonGroup>

#include "UBAbstractSubPalette.h"
#include "UBShapesPalette.h"
#include <QActionGroup>

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
        void setupSubPalettes(QWidget* parent);
        void connectDrawingActions();
        void connectSubPalettes();
        void setVisible(bool checked);

        virtual void initSubPalettesPosition();
        void updateSubPalettesPosition(const QPoint &delta);

        void stackUnder(QWidget *w);

        static const int PRESS_DURATION;

    private slots:

        //void drawingToolPressed();
        //void drawingToolReleased();
        void updateActions();
        void buttonClicked();

        //for testing
        void changeProperty(bool ok);

    signals:
        void pressed(int);

    private:
        int mLastSelectedId;
        QMap<int, UBAbstractSubPalette*> mSubPalettes;
        QMap<UBActionPaletteButton*, UBAbstractSubPalette*> mSubPalettes2;

        QTime mActionButtonPressedTime;
        bool mPendingActionButtonPressed;

        void monInit();
        UBActionPaletteButton *addActionButton(QAction *action);
public slots:
        void raise();

protected:
        UBActionPaletteButton *addButtonSubPalette(UBAbstractSubPalette *subPalette, QAction *action = 0);
};


#endif // UBDRAWINGPALETTE_H
