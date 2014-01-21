#ifndef UBDRAWINGPOLYGONPALETTE_H
#define UBDRAWINGPOLYGONPALETTE_H

#include "UBAbstractSubPalette.h"

class UBDrawingPolygonPalette : public UBAbstractSubPalette
{
    Q_OBJECT

public:
    UBDrawingPolygonPalette(Qt::Orientation orient, QWidget *parent = 0);
    virtual ~UBDrawingPolygonPalette();

private:
    UBActionPaletteButton *addActionButton(QAction *action);

private slots:
    void buttonClicked();
};

#endif // UBDRAWINGPOLYGONPALETTE_H
