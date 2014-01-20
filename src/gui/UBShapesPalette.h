#ifndef UBSHAPESPALETTE_H
#define UBSHAPESPALETTE_H

#include <QButtonGroup>

#include "UBAbstractSubPalette.h"

class UBShapesPalette : public UBAbstractSubPalette
{
    Q_OBJECT

    public:
        UBShapesPalette(Qt::Orientation orient, QWidget *parent = 0 );

        virtual ~UBShapesPalette();

    private slots:

        void shapesToolPressed();
        void shapesToolReleased();

        void buttonClicked();
private:
        int mLastSelectedId;

    signals:
        void shapesToolPressed(int tool);
        void shapesToolReleased(int tool);
        void newMainAction();
};

#endif // UBSHAPESPALETTE_H
