#ifndef UBABSTRACTSUBPALETTE_H
#define UBABSTRACTSUBPALETTE_H

#include "UBActionPalette.h"
#include <QAction>

class UBAbstractSubPalette : public UBActionPalette
{
    public :
        UBAbstractSubPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBAbstractSubPalette(Qt::Orientation orient, QWidget *parent = 0 );

        virtual void togglePalette();
        QAction* mainAction() { return actions().at(mMainAction); }

        UBActionPaletteButton * actionPaletteButtonParent() const {return mActionPaletteButtonParent;}
        void setActionPaletteButtonParent(UBActionPaletteButton * button){mActionPaletteButtonParent = button;}

    protected :
        int mMainAction;

        UBActionPaletteButton * mActionPaletteButtonParent; // button that opened this subPalette.

        // QWidget interface
protected:
        virtual void focusOutEvent(QFocusEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);
};

class UBColorPickerButton : public QPushButton
{
public:
    UBColorPickerButton(QWidget * parent = 0);

    QColor color() const {return mColor;}
    void setColor(QColor color) {mColor = color;}

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent * pe);

private:
    QColor mColor;
};


#endif // UBABSTRACTSUBPALETTE_H
