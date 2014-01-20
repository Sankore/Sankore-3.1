#include "UBDrawingPolygonPalette.h"

#include "core/UBApplication.h"
#include "UBMainWindow.h"
#include "board/UBBoardController.h"
#include "domain/UBShapeFactory.h"

#include <QFocusEvent>

#include "board/UBBoardPaletteManager.h"
#include "gui/UBStylusPalette.h"


UBDrawingPolygonPalette::UBDrawingPolygonPalette(Qt::Orientation orient, QWidget *parent)
    :UBAbstractSubPalette(orient, parent)
{
    hide();


    QList<QAction*> actions;
    actions<<UBApplication::mainWindow->actionPolygon;
    actions<<UBApplication::mainWindow->actionSmartLine;
    actions<<UBApplication::mainWindow->actionSmartPen;
    //actions<<UBApplication::mainWindow->actionRectangle;
    //actions<<UBApplication::mainWindow->actionSquare;

    setActions(actions);
    //setButtonIconSize(QSize(28, 28));
    groupActions();

    adjustSizeAndPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }

/*
    UBActionPaletteButton * btn1 = addActionButton(UBApplication::mainWindow->actionPolygon);
    UBActionPaletteButton * btn2 = addActionButton(UBApplication::mainWindow->actionLine);

    UBActionPaletteButton * btn3 = addActionButton(UBApplication::mainWindow->actionSmartLine);
    UBActionPaletteButton * btn4 = addActionButton(UBApplication::mainWindow->actionSmartPen);

    adjustSizeAndPosition();

    // Grouper les boutons :
    QButtonGroup * groupe1 = new QButtonGroup;
    groupe1->addButton(btn1);
    groupe1->addButton(btn2);

    QButtonGroup * groupe2 = new QButtonGroup;
    groupe2->addButton(btn3);
    groupe2->addButton(btn4);

    connect(btn1, SIGNAL(clicked()), this, SLOT(createPolygon()));
    connect(btn2, SIGNAL(clicked()), this, SLOT(createLine()));
    connect(btn3, SIGNAL(clicked()), this, SLOT(createRectangle()));
    connect(btn4, SIGNAL(clicked()), this, SLOT(createSquare()));
*/
}

UBActionPaletteButton * UBDrawingPolygonPalette::addActionButton(QAction * action)
{
    UBActionPaletteButton * button = new UBActionPaletteButton(action, this);
    button->setToolButtonStyle(mToolButtonStyle);
    //button->setIconSize(QSize(42, 42));
    mActions.append(action);
    layout()->addWidget(button);
    return button;
}

UBDrawingPolygonPalette::~UBDrawingPolygonPalette()
{

}

void UBDrawingPolygonPalette::buttonClicked()
{
    UBActionPaletteButton * button = dynamic_cast<UBActionPaletteButton *>(sender());
    if (button)
    {
        QAction * action = button->defaultAction();

        if (action)
        {
            if (action == UBApplication::mainWindow->actionPolygon){
                UBApplication::boardController->shapeFactory().createPolygon(true);
            }
            else if (action == UBApplication::mainWindow->actionSmartLine){
                UBApplication::boardController->shapeFactory().createLine(true);
            }
            else if (action == UBApplication::mainWindow->actionSmartPen){
                UBApplication::boardController->shapeFactory().createPen(true);
            }

            actionPaletteButtonParent()->setDefaultAction(action);
        }
    }
/*
    // Deselect tool from Stylus Palette
    UBStylusPalette* stylusPalette = UBApplication::boardController->paletteManager()->stylusPalette();
    QAbstractButton * checkedButton = stylusPalette->buttonGroup()->checkedButton();
    if (checkedButton){
        stylusPalette->buttonGroup()->setExclusive(false);

        QToolButton * toolButton = dynamic_cast<QToolButton*>(checkedButton);
        if (toolButton && toolButton->defaultAction()){
            //toolButton->defaultAction()->setChecked(false);
            toolButton->defaultAction()->toggle();
        }

        //stylusPalette->buttonGroup()->checkedButton()->setChecked(false);
        stylusPalette->buttonGroup()->setExclusive(true);

    }
*/
    hide();
}
/*
void UBDrawingPolygonPalette::createPolygon()
{
   UBApplication::boardController->shapeFactory().createPolygon(true);
   changeParentPaletteButtonAction(dynamic_cast<UBActionPaletteButton*>(sender()));
   hide();
}

void UBDrawingPolygonPalette::createLine()
{
   UBApplication::boardController->shapeFactory().createLine(true);
   changeParentPaletteButtonAction(dynamic_cast<UBActionPaletteButton*>(sender()));
   hide();
}

void UBDrawingPolygonPalette::createRectangle()
{
   UBApplication::boardController->shapeFactory().createRectangle(true);
   changeParentPaletteButtonAction(dynamic_cast<UBActionPaletteButton*>(sender()));
   hide();
}

void UBDrawingPolygonPalette::createSquare()
{
   UBApplication::boardController->shapeFactory().createSquare(true);
   changeParentPaletteButtonAction(dynamic_cast<UBActionPaletteButton*>(sender()));
   hide();
}
*/
void UBDrawingPolygonPalette::changeParentPaletteButtonAction(UBActionPaletteButton * button)
{
    if (button)
    {
        QAction * action = button->defaultAction();
        if (action){
            mActionPaletteButtonParent->setDefaultAction(action);
        }
    }
}
