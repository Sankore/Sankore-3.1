#include "WBTrapWebPageContent.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"

WBTrapBar::WBTrapBar(QWidget *parent)
    : UBActionPalette(Qt::Vertical, parent)
{
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    layout()->setAlignment(Qt::AlignTop);
    setButtonIconSize(QSize(128, 128));    
}

void WBTrapBar::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void WBTrapBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

WBTrapWebPageContentWindow::WBTrapWebPageContentWindow(QWidget *parent)
    : QDialog(parent)
{
    mTrapApplicationHLayout = new QHBoxLayout(this);
    setLayout(mTrapApplicationHLayout);

    mTrapApplicationVLayout = new QVBoxLayout();
    QSizePolicy selfSizePolicy = sizePolicy();
    selfSizePolicy.setHeightForWidth(true);
    setSizePolicy(selfSizePolicy);

    mTrapActionsBar = new WBTrapBar();

    mTrapActionsBar->addAction(UBApplication::mainWindow->actionWebTrapToLibrary);
    mTrapActionsBar->addAction(UBApplication::mainWindow->actionWebTrapToCurrentPage);
    mTrapActionsBar->addAction(UBApplication::mainWindow->actionWebTrapLinkToLibrary);
    mTrapActionsBar->addAction(UBApplication::mainWindow->actionWebTrapLinkToPage);

    mTrapApplicationHLayout->addWidget(mTrapActionsBar);
    mTrapApplicationHLayout->addLayout(mTrapApplicationVLayout);

    mSelectContentLayout = new QHBoxLayout();
    mSelectContentLabel = new QLabel(tr("Select content to trap:"));
    mSelectContentCombobox = new QComboBox(this);
    mSelectContentCombobox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); 
    
    mSelectContentLayout->addWidget(mSelectContentLabel);
    mSelectContentLayout->addWidget(mSelectContentCombobox);

    mTrapApplicationVLayout->addLayout(mSelectContentLayout);


    mTrapContentPreview = new QWidget();
    mTrapContentPreview->setMinimumSize(QSize(640, 480));
    mTrapContentPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); 
    mTrapApplicationVLayout->addWidget(mTrapContentPreview);
    
    mApplicationNameLayout = new QHBoxLayout();
    mApplicationNameLabel = new QLabel(tr("Application name"));
    mApplicationNameEdit = new QLineEdit();

    mApplicationNameLayout->addWidget(mApplicationNameLabel);
    mApplicationNameLayout->addWidget(mApplicationNameEdit);
    mTrapApplicationVLayout->addLayout(mApplicationNameLayout);

    mTrapApplicationVLayout->addLayout(mApplicationNameLayout);
}

WBTrapWebPageContentWindow::~WBTrapWebPageContentWindow()
{

}
