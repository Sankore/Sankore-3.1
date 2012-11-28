/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBCreateLinkPalette.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QMouseEvent>

#include "customWidgets/UBGraphicsItemAction.h"


UBCreateLinkPalette::UBCreateLinkPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopRightCorner, parent)
{
    setObjectName("UBCreateLinkPalette");
    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(10,28,10,10);
    setLayout(mLayout);
    mStackedWidget = new QStackedWidget(this);
    mLayout->addWidget(mStackedWidget);
    init();
    hide();
}


UBCreateLinkPalette::~UBCreateLinkPalette()
{

}

QToolButton* UBCreateLinkPalette::addInitiaWidgetButton(QString textButton, QString iconPath)
{
    QToolButton* button = new QToolButton(mInitialWidget);
    button->setIcon(QIcon(iconPath));
    button->setIconSize(QSize(100,100));
    button->setText(textButton);
    button->setStyleSheet("background-color:transparent; board : none;");
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    return button;
}

void UBCreateLinkPalette::init()
{
    mInitialWidget = new QWidget(this);
    QHBoxLayout* initialWidgetLayout = new QHBoxLayout(mInitialWidget);
    mInitialWidget->setLayout(initialWidgetLayout);
    QToolButton* actionPlayAudio = addInitiaWidgetButton(tr("Play an audio file"),QString(":images/toolbar/plusBlackOn.png"));
    QToolButton* actionAddLinkToPage = addInitiaWidgetButton(tr("Add Link to Page"),":images/toolbar/plusBlackOn.png");

    QToolButton* actionAddLinkToWeb = addInitiaWidgetButton(tr("Add Link to a Web page"),":images/toolbar/plusBlackOn.png");

    initialWidgetLayout->addWidget(actionPlayAudio);
    initialWidgetLayout->addWidget(actionAddLinkToPage);
    initialWidgetLayout->addWidget(actionAddLinkToWeb);
    mStackedWidget->addWidget(mInitialWidget);


    mAudioWidget = new QWidget(this);
    QVBoxLayout* audioWidgetLayout = new QVBoxLayout(mAudioWidget);
    mAudioWidget->setLayout(audioWidgetLayout);
    QHBoxLayout* audioBackButtonLayout = new QHBoxLayout();
    QPushButton* audioBackButton = new QPushButton(mAudioWidget);
    audioBackButton->setIcon(QIcon(":images/toolbar/undoOn.png"));
    audioBackButtonLayout->addWidget(audioBackButton);
    audioBackButtonLayout->addStretch();
    audioWidgetLayout->addLayout(audioBackButtonLayout);
    connect(audioBackButton,SIGNAL(clicked()),this,SLOT(onBackButtonClicked()));
    audioWidgetLayout->addWidget(new QLabel(tr("Drag and drop the audio file from the library in this box"),mAudioWidget));
    QHBoxLayout* audioOkButtonLayout = new QHBoxLayout();
    audioOkButtonLayout->addStretch();
    QPushButton* audioOkButton = new QPushButton(tr("Ok"),mAudioWidget);
    audioOkButtonLayout->addWidget(audioOkButton);
    audioWidgetLayout->addLayout(audioOkButtonLayout);
    connect(audioOkButton,SIGNAL(clicked()),this,SLOT(onOkAudioClicked()));
    mStackedWidget->addWidget(mAudioWidget);
    connect(actionPlayAudio,SIGNAL(clicked()),this,SLOT(onPlayAudioClicked()));


    mPageLinkWidget = new QWidget(this);
    QVBoxLayout* pageLinkWidgetLayout = new QVBoxLayout(mPageLinkWidget);
    mPageLinkWidget->setLayout(pageLinkWidgetLayout);
    QHBoxLayout* pageLinkBackButtonLayout = new QHBoxLayout();
    QPushButton* pageLinkBackButton = new QPushButton(mPageLinkWidget);
    pageLinkBackButton->setIcon(QIcon(":images/toolbar/undoOn.png"));
    pageLinkBackButtonLayout->addWidget(pageLinkBackButton);
    pageLinkBackButtonLayout->addStretch();
    pageLinkWidgetLayout->addLayout(pageLinkBackButtonLayout);
    connect(pageLinkBackButton,SIGNAL(clicked()),this,SLOT(onBackButtonClicked()));
    QHBoxLayout* pageLinkOkButtonLayout = new QHBoxLayout();
    pageLinkOkButtonLayout->addStretch();
    QPushButton* pageLinkOkButton = new QPushButton(tr("Ok"), mPageLinkWidget);
    pageLinkOkButtonLayout->addWidget(pageLinkOkButton);
    pageLinkWidgetLayout->addLayout(pageLinkOkButtonLayout);
    connect(pageLinkOkButton,SIGNAL(clicked()),this,SLOT(onOkLinkToPageClicked()));
    mStackedWidget->addWidget(mPageLinkWidget);
    connect(actionAddLinkToPage,SIGNAL(clicked()),this,SLOT(onAddLinkToPageClicked()));


    mUrlLinkWidget = new QWidget(this);
    QVBoxLayout* urlLinkWidgetLayout = new QVBoxLayout(mUrlLinkWidget);
    mUrlLinkWidget->setLayout(urlLinkWidgetLayout);
    QHBoxLayout* urlLinkBackButtonLayout = new QHBoxLayout();
    QPushButton* urlLinkBackButton = new QPushButton(mUrlLinkWidget);
    urlLinkBackButton->setIcon(QIcon(":images/toolbar/undoOn.png"));
    urlLinkBackButtonLayout->addWidget(urlLinkBackButton);
    urlLinkBackButtonLayout->addStretch();
    urlLinkWidgetLayout->addLayout(urlLinkBackButtonLayout);
    connect(urlLinkBackButton,SIGNAL(clicked()),this,SLOT(onBackButtonClicked()));
    mUrlLineEdit = new QLineEdit(mUrlLinkWidget);
    mUrlLineEdit->setPlaceholderText(tr("Insert url text here"));
    mUrlLineEdit->setAcceptDrops(true);
    urlLinkWidgetLayout->addWidget(mUrlLineEdit);
    QHBoxLayout* urlLinkOkButtonLayout = new QHBoxLayout();
    urlLinkOkButtonLayout->addStretch();
    QPushButton* urlLinkOkButton = new QPushButton(tr("Ok"), mUrlLinkWidget);
    urlLinkOkButtonLayout->addWidget(urlLinkOkButton);
    urlLinkWidgetLayout->addLayout(urlLinkOkButtonLayout);
    connect(urlLinkOkButton,SIGNAL(clicked()),this,SLOT(onOkLinkToWebClicked()));
    mStackedWidget->addWidget(mUrlLinkWidget);
    connect(actionAddLinkToWeb,SIGNAL(clicked()),this,SLOT(onAddLinkToWebClicked()));

}

void UBCreateLinkPalette::onBackButtonClicked()
{
    mStackedWidget->setCurrentIndex(0);
    adjustGeometry();
}

void UBCreateLinkPalette::onPlayAudioClicked()
{
    mStackedWidget->setCurrentIndex(1);
    adjustGeometry();
}

void UBCreateLinkPalette::onAddLinkToPageClicked()
{
    mStackedWidget->setCurrentIndex(2);
    adjustGeometry();
}

void UBCreateLinkPalette::onAddLinkToWebClicked()
{
    mStackedWidget->setCurrentIndex(3);
    adjustGeometry();
}

void UBCreateLinkPalette::onOkAudioClicked()
{
    emit definedAction(new UBGraphicsItemPlayAudioAction("/home/claudio.wav"));
    close();
}

void UBCreateLinkPalette::onOkLinkToPageClicked()
{
    emit definedAction(new UBGraphicsItemMoveToPageAction(eMoveToNextNextPage));
    close();
}

void UBCreateLinkPalette::onOkLinkToWebClicked()
{
    emit definedAction(new UBGraphicsItemLinkToWebPageAction (mUrlLineEdit->text()));
    close();
}

void UBCreateLinkPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, QPixmap(":/images/close.svg"));
}


void UBCreateLinkPalette::close()
{
    hide();
}


void UBCreateLinkPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        close();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}


void UBCreateLinkPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustGeometry();
}

void UBCreateLinkPalette::adjustGeometry()
{
    adjustSizeAndPosition();
    move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 5);
}
