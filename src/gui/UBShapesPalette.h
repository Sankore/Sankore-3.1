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
        void buttonClicked();
};

#endif // UBSHAPESPALETTE_H
