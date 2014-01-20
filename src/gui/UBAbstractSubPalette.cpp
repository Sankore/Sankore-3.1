#include "UBAbstractSubPalette.h"

UBAbstractSubPalette::UBAbstractSubPalette(QWidget *parent, Qt::Orientation orient) :
    UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mMainAction(0)
{
    mCustomPosition = true;
    setFocusPolicy(Qt::StrongFocus); // in order to detect focus loss.
}

UBAbstractSubPalette::UBAbstractSubPalette(Qt::Orientation orient, QWidget *parent) :
    UBActionPalette(orient, parent)
    , mMainAction(0)
{
    mCustomPosition = true;
}

void UBAbstractSubPalette::togglePalette()
{
    show();
    setFocus(); // explicitely give focus to the subPalette, to detect when it loses focus, and then hide it .
}


void UBAbstractSubPalette::focusOutEvent(QFocusEvent *)
{
    hide(); // hide subPalette when it loses focus (for exemple : clic anywhere else)
}

void UBAbstractSubPalette::mouseMoveEvent(QMouseEvent *)
{
    // User can't move subPalette with the mouse.
}

UBColorPickerButton::UBColorPickerButton(QWidget *parent)
    :QPushButton(parent)
{
}

void UBColorPickerButton::paintEvent(QPaintEvent * pe)
{
    QPainter painter(this);
    painter.setBrush(color());
    painter.drawRect(0, 0, 20, 20);

    // If transparent color, draw a cross in the rect :
    if (color() == Qt::transparent)
    {
        painter.drawLine(0,0, 20,20);
        painter.drawLine(20,0, 0,20);
    }
}
