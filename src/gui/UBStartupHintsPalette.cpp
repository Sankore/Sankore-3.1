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


#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QWebView>
#include <QCheckBox>
#include "UBStartupHintsPalette.h"

#include "globals/UBGlobals.h"
#include "core/UBSettings.h"

UBStartupHintsPalette::UBStartupHintsPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopRightCorner,parent)
{
    setObjectName("UBStartupHintsPalette");
    if(UBSettings::settings()->appStartupHintsEnabled->get().toBool()){
        setFixedSize(700,450);
        mLayout = new QVBoxLayout();
        mLayout->setContentsMargins(10,28,10,10);
        setLayout(mLayout);
        QString url = UBSettings::settings()->applicationStartupHintsDirectory() + "/index.html";
        QWebView* webView = new QWebView(this);
        webView->setUrl(QUrl::fromLocalFile(url));
        webView->setAcceptDrops(false);
        mLayout->addWidget(webView);
        mButtonLayout = new QHBoxLayout();
        mLayout->addLayout(mButtonLayout);
        mShowNextTime = new QCheckBox(tr("Visible next time"),this);
        mShowNextTime->setCheckState(Qt::Checked);
        connect(mShowNextTime,SIGNAL(stateChanged(int)),this,SLOT(onShowNextTimeStateChanged(int)));
        mButtonLayout->addStretch();
        mButtonLayout->addWidget(mShowNextTime);
    }
    else
        hide();
}

UBStartupHintsPalette::~UBStartupHintsPalette()
{
//    DELETEPTR(mButtonLayout);
//    DELETEPTR(mLayout);
}

void UBStartupHintsPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, QPixmap(":/images/close.svg"));
}


void UBStartupHintsPalette::close()
{
    hide();
}


void UBStartupHintsPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        close();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}

void UBStartupHintsPalette::onShowNextTimeStateChanged(int state)
{
    UBSettings::settings()->appStartupHintsEnabled->setBool(state == Qt::Checked);
}

void UBStartupHintsPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustSizeAndPosition();
    move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 5);
}


int UBStartupHintsPalette::border()
{
    return 40;
}
