#ifndef IDOCKABLE_H
#define IDOCKABLE_H

#include "customWidgets/UBDockPaletteWidget.h"

class IDockable{
public:
    virtual ~IDockable(){}
    virtual UBDockPaletteWidget* dockableWidget()=0;
};

Q_DECLARE_INTERFACE(IDockable, "org.open-sankore.interfaces.IDockable")

#endif // IDOCKABLE_H
