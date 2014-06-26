#ifndef UBDOCKRESOURCESWIDGET_H
#define UBDOCKRESOURCESWIDGET_H

#include "UBTeacherGuideWidget.h"
#include "UBDockPaletteWidget.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "gui/UBLeftPalette.h"

#include <QVBoxLayout>

class UBDockResourcesWidget : public UBDockPaletteWidget
{
    Q_OBJECT

public:
    UBDockResourcesWidget(QWidget *parent = 0);


    bool visibleInMode(eUBDockPaletteWidgetMode mode, int currentPage)
    {
        return mode == eUBDockPaletteWidget_BOARD && currentPage > 0;
    }

    QPixmap iconToLeft() const {return QPixmap(":images/teacherGuideResources_close.png");}
    QPixmap iconToRight() const {return QPixmap(":images/teacherGuideResources_open.png");}

    bool isModified();
    bool hasUserDataInTeacherGuide();

private:
    UBTeacherResources *mTeacherResourceWidget;
    UBLeftPalette *mLeftPalette;
};

#endif // UBDOCKRESOURCESWIDGET_H
