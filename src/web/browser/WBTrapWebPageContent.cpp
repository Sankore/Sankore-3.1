#include "WBTrapWebPageContent.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "gui/UBMainWindow.h"

WBTrapBar::WBTrapBar(QWidget *parent)
    : UBActionPalette(Qt::Vertical, parent)
{
    layout()->setAlignment(Qt::AlignTop);

    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
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

WBTrapWebPageContentWindow::WBTrapWebPageContentWindow(QObject *controller, QWidget *parent)
    : QDialog(parent)
    , mController(controller)
{
    setModal(true);
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

    foreach (QAction *barAction, mTrapActionsBar->actions())
    {
        mTrapActionsBar->getButtonFromAction(barAction)->setStyleSheet(QString("QToolButton{color: black; font: bold 14px; font-family: Arial; background-color: transparent; border: none}"));
        barAction->setEnabled(false);
    }


    mTrapApplicationHLayout->addLayout(mTrapApplicationVLayout);
    mTrapApplicationHLayout->addWidget(mTrapActionsBar);
    mSelectContentLayout = new QHBoxLayout();
    mSelectContentLabel = new QLabel(tr("Select content to trap:"));
    mSelectContentCombobox = new QComboBox(this);
    mSelectContentCombobox->setMaxVisibleItems(15);
    mSelectContentCombobox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mSelectContentLayout->addWidget(mSelectContentLabel);
    mSelectContentLayout->addWidget(mSelectContentCombobox);

    mTrapApplicationVLayout->addLayout(mSelectContentLayout);

    mTrapContentPreview = new WBWebView();
    mTrapContentPreview->setMinimumSize(QSize(640, 480));
    mTrapContentPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
 //   mTrapContentPreview->setIsTrapping(true);

 /*   mTrapingWidgetMask = new UBWebTrapMouseEventMask(mTrapContentPreview);
    mTrapingWidgetMask->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    mTrapingWidgetMask->setAttribute(Qt::WA_TranslucentBackground, true);
    mTrapingWidgetMask->move(0,0);
    mTrapingWidgetMask->resize(mTrapContentPreview->size()-);
    mTrapingWidgetMask->setVisible(true);*/

    mTrapApplicationVLayout->addWidget(mTrapContentPreview);

    mApplicationNameLayout = new QHBoxLayout();
    mApplicationNameLabel = new QLabel(tr("Application name"));
    mApplicationNameEdit = new QLineEdit();

    mApplicationNameLayout->addWidget(mApplicationNameLabel);
    mApplicationNameLayout->addWidget(mApplicationNameEdit);
    mTrapApplicationVLayout->addLayout(mApplicationNameLayout);

    mTrapApplicationVLayout->addLayout(mApplicationNameLayout);

    connect(mTrapContentPreview, SIGNAL(pixmapCaptured(const QPixmap&, bool)), UBApplication::applicationController, SLOT(addCapturedPixmap(const QPixmap &, bool)));
    connect(mTrapContentPreview, SIGNAL(embedCodeCaptured(const QString&)), UBApplication::applicationController, SLOT(addCapturedEmbedCode(const QString&)));

    connect(mSelectContentCombobox, SIGNAL(currentIndexChanged(int)), mController, SLOT(selectHtmlObject(int)));
    connect(mApplicationNameEdit, SIGNAL(textChanged(const QString &)), mController, SLOT(text_Changed(const QString &)));
    connect(mApplicationNameEdit, SIGNAL(textEdited(const QString &)), mController, SLOT(text_Edited(const QString &)));

    connect(UBApplication::mainWindow->actionWebTrapToCurrentPage, SIGNAL(triggered()), mController, SLOT(addItemToBoard()));
    connect(UBApplication::mainWindow->actionWebTrapToLibrary, SIGNAL(triggered()), mController, SLOT(addItemToLibrary()));
    connect(UBApplication::mainWindow->actionWebTrapLinkToPage, SIGNAL(triggered()), mController, SLOT(addLinkToBoard()));
    connect(UBApplication::mainWindow->actionWebTrapLinkToLibrary, SIGNAL(triggered()), mController, SLOT(addLinkToLibrary()));
}

WBTrapWebPageContentWindow::~WBTrapWebPageContentWindow()
{

}

void WBTrapWebPageContentWindow::setUrl(const QUrl &url)
{
    mTrapContentPreview->setUrl(url);
}

void WBTrapWebPageContentWindow::setReadyForTrap(bool bReady)
{
    foreach (QAction *barAction, mTrapActionsBar->actions())
    {
        barAction->setEnabled(bReady);
    }
}
