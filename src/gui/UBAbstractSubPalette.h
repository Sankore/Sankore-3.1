#ifndef UBABSTRACTSUBPALETTE_H
#define UBABSTRACTSUBPALETTE_H

#include "UBActionPalette.h"
#include <QAction>

class UBAbstractSubPalette : public UBActionPalette
{
    public :
        UBAbstractSubPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBAbstractSubPalette(Qt::Orientation orient, QWidget *parent = 0 );

        virtual void togglePalette() =0;
        QAction* mainAction() { return actions().at(mMainAction); }
    protected :
        int mMainAction;
};
#endif // UBABSTRACTSUBPALETTE_H
