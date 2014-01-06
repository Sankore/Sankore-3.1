#ifndef UBELLIPSEPALETTE_H
#define UBELLIPSEPALETTE_H

#include <QButtonGroup>

#include "UBAbstractSubPalette.h"

class UBEllipsePalette : public UBAbstractSubPalette
{
    Q_OBJECT

    public:
        UBEllipsePalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBEllipsePalette(Qt::Orientation orient, QWidget *parent = 0 );

        virtual ~UBEllipsePalette();

        void mouseMoveEvent(QMouseEvent *event);


        void togglePalette();

    private slots:

        void ellipseToolPressed();
        void ellipseToolReleased();

    private:
        int mLastSelectedId;

    signals:
        void ellipseToolPressed(int tool);
        void ellipseToolReleased(int tool);
};

#endif // UBELLIPSEPALETTE_H
