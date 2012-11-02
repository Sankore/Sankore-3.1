/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBActionPalette.h"

#include "core/memcheck.h"

UBActionPalette::UBActionPalette(QList<QAction*> actions, Qt::Orientation orientation, QWidget * parent)
    : UBFloatingPalette(Qt::TopRightCorner, parent)
{
    init(orientation);
    setActions(actions);
}


UBActionPalette::UBActionPalette(Qt::Orientation orientation, QWidget * parent)
     : UBFloatingPalette(Qt::TopRightCorner, parent)
{
    init(orientation);
}


UBActionPalette::UBActionPalette(QWidget * parent)
     : UBFloatingPalette(Qt::TopRightCorner, parent)
{
    init(Qt::Vertical);
}


UBActionPalette::UBActionPalette(Qt::Corner corner, QWidget * parent, Qt::Orientation orient)
     : UBFloatingPalette(corner, parent)
{
    init(orient);
}


void UBActionPalette::init(Qt::Orientation orientation)
{
    m_customCloseProcessing = false;

    mButtonSize = QSize(32, 32);
    mIsClosable = false;
    mAutoClose = false;
    mButtonGroup = 0;
    mToolButtonStyle = Qt::ToolButtonIconOnly;
    mButtons.clear();

    if (orientation == Qt::Horizontal)
        new QHBoxLayout(this);
    else
        new QVBoxLayout(this);

    mButtonGroup = new QButtonGroup(this);

    connect(mButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));

    updateLayout();
}

void UBActionPalette::setActions(QList<QAction*> actions)
{
    mMapActionToButton.clear();

    foreach(QAction* action, actions)
    {
        addAction(action);
    }

    actionChanged();
}

UBActionPaletteMultiStateButton* UBActionPalette::createPaletteButton(QList<QAction*> actions, QWidget *parent)
{
    UBActionPaletteMultiStateButton *button = new UBActionPaletteMultiStateButton(actions, parent);

    button->setIconSize(mButtonSize);
    button->setToolButtonStyle(mToolButtonStyle);

    mButtons << button;

    mMapActionToButton[actions[0]] = button;

    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(actions[0], SIGNAL(changed()), this, SLOT(actionChanged()));

    return button;

}

UBActionPaletteButton* UBActionPalette::createPaletteButton(QAction* action, QWidget *parent)
{
    UBActionPaletteButton* button = new UBActionPaletteButton(action, parent);
    button->setIconSize(mButtonSize);
    button->setToolButtonStyle(mToolButtonStyle);

    mButtons << button;

    mMapActionToButton[action] = button;

    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(action, SIGNAL(changed()), this, SLOT(actionChanged()));

    return button;
}


void UBActionPalette::addActions(QList<QAction*> actions, bool addToGroup)
{

    UBActionPaletteMultiStateButton* button = createPaletteButton(actions, this);

    if(!mButtonGroup)
        mButtonGroup = new QButtonGroup(this);

    if (addToGroup && mButtonGroup)
        mButtonGroup->addButton(button);

    layout()->addWidget(button);

    mActions << actions;

}

void UBActionPalette::addAction(QAction* action, bool addToGroup)
{
    UBActionPaletteButton* button = createPaletteButton(action, this);

    if(!mButtonGroup)
        mButtonGroup = new QButtonGroup(this);

    if (addToGroup && mButtonGroup)
        mButtonGroup->addButton(button);

    layout()->addWidget(button);

    mActions << action;
}
void UBActionPalette::buttonClicked(int id)
{
    if (mButtonGroup)
    {
        int checked = mButtonGroup->checkedId();

        foreach (QAbstractButton *button, mButtonGroup->buttons())
        {      
            if (id != mButtonGroup->id(button))
            {
                QAbstractButton *currentButton = mButtonGroup->button(mButtonGroup->checkedId());
                if (currentButton)
                {
                    foreach(QAction *buttonAction, currentButton->actions())
                    {
                     //   buttonAction->setChecked(false);
                    }
                }
            }
        }
    }
}

void UBActionPalette::buttonClicked()
{
    if (mAutoClose)
    {
        close();
    }
}

QList<QAction*> UBActionPalette::actions()
{
    return mActions;
}


UBActionPalette::~UBActionPalette()
{
    qDeleteAll(mButtons.begin(), mButtons.end());
    mButtons.clear();
}


void UBActionPalette::setButtonIconSize(const QSize& size)
{
    foreach(QToolButton* button, mButtons)
        button->setIconSize(size);

    mButtonSize = size;
}


void UBActionPalette::groupActions()
{
    mButtonGroup = new QButtonGroup(this);
    int i = 0;
    foreach(QToolButton* button, mButtons)
    {
        mButtonGroup->addButton(button, i);
        ++i;
    }

    connect(mButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(buttonGroupClicked(int)));
}


void UBActionPalette::setToolButtonStyle(Qt::ToolButtonStyle tbs)
{
    foreach(QToolButton* button, mButtons)
        button->setToolButtonStyle(tbs);

    mToolButtonStyle = tbs;

    updateLayout();

}

void UBActionPalette::updateLayout()
{
    if (mToolButtonStyle == Qt::ToolButtonIconOnly)
    {
        layout()->setContentsMargins (sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border()
                , sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border());
    }
    else
    {
        layout()->setContentsMargins (sLayoutContentMargin  + border(), sLayoutContentMargin  + border()
                , sLayoutContentMargin  + border(), sLayoutContentMargin + border());

    }
   update();
}


void UBActionPalette::setClosable(bool pClosable)
{
    mIsClosable = pClosable;

    updateLayout();
}


int UBActionPalette::border()
{
    if (mIsClosable)
        return 10;
    else
        return 5;
}


void UBActionPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    if (mIsClosable)
    {
        QPainter painter(this);
        painter.drawPixmap(0, 0, QPixmap(":/images/close.svg"));
    }
}


void UBActionPalette::close()
{
    if(!m_customCloseProcessing)
        hide();

    emit closed();
}


void UBActionPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (mIsClosable && event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        close();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}


void UBActionPalette::actionChanged()
{
    for(int i = 0; i < mActions.length() && i < mButtons.length(); i++)
    {
        mButtons.at(i)->setVisible(mActions.at(i)->isVisible());
    }
}

void UBActionPalette::clearLayout()
{
    QLayout* pLayout = layout();
    if(NULL != pLayout)
    {
        while(!pLayout->isEmpty())
        {
            QLayoutItem* pItem = pLayout->itemAt(0);
            QWidget* pW = pItem->widget();
            pLayout->removeItem(pItem);
            delete pItem;
            pLayout->removeWidget(pW);
            delete pW;
        }

        mActions.clear();
        mButtons.clear();
    }
}

UBActionPaletteButton::UBActionPaletteButton(QAction* action, QWidget * parent)
    : QToolButton(parent)
{
    setIconSize(QSize(32, 32));
    setDefaultAction(action);
    setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));

    setFocusPolicy(Qt::NoFocus);

    setObjectName("ubActionPaletteButton");
}


UBActionPaletteButton::~UBActionPaletteButton()
{

}

void UBActionPaletteButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit doubleClicked();
}

QSize UBActionPalette::buttonSize()
{
    return mButtonSize;
}

/**
 * \brief Returns the button related to the given action
 * @param action as the given action
 */
UBActionPaletteButton* UBActionPalette::getButtonFromAction(QAction *action)
{
    UBActionPaletteButton* pButton = NULL;

    pButton = mMapActionToButton.value(action);

    return pButton;
}

bool UBActionPaletteButton::hitButton(const QPoint &pos) const
{
    Q_UNUSED(pos);
    return true;
}



UBActionPaletteMultiStateButton::UBActionPaletteMultiStateButton(QList<QAction*> actions, QWidget *parent)
    : UBActionPaletteButton(actions[0], parent)
    , mActions(actions)
{
    mActionsGroup = new QActionGroup(this);
    for (int i = 1; i < mActions.count(); i++)
    {
        QToolButton::addAction(actions[i]);
        mActionsGroup->addAction(actions[i]);
        connect(mActionsGroup, SIGNAL(triggered(QAction*)), this, SLOT(setActiveAction(QAction*)));
    }
}

UBActionPaletteMultiStateButton::~UBActionPaletteMultiStateButton()
{

}

void UBActionPaletteMultiStateButton::nextCheckState()
{
    int id = -1;
    int selfGroupId = -1; 

    UBActionPalette *palette = qobject_cast<UBActionPalette *>(parent());
    if (palette)
    {
        QButtonGroup *group = palette->buttonGroup();
        if (group)
        {
            id = group->checkedId();
            selfGroupId = group->id(this);
        }

        if (this->isChecked() && id == selfGroupId)
        {
            int newActionIndex = (mActions.indexOf(defaultAction())+1)%mActions.count();
            QAction *newAction = mActions[newActionIndex];
            setDefaultAction(newAction);
            group->button(selfGroupId)->setChecked(true);
            newAction->trigger();
        }

        if (!this->isChecked() && id != selfGroupId)
        {
            foreach (QAction *action, mActionsGroup->actions())
            {
                if(action->isChecked())
                    action->trigger();
            }
           
            group->button(selfGroupId)->setChecked(true);
            defaultAction()->toggle();
        }

        if (id != selfGroupId)
        {
            group->button(selfGroupId)->setChecked(false);
        }
    }
}

void UBActionPaletteMultiStateButton::setActiveAction(QAction* action)
{
    //setChecked(true);
   // setDefaultAction(action);
}