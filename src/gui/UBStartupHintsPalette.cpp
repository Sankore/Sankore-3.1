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
#include "core/UBSettings.h"
#include "UBStartupHintsPalette.h"

UBStartupHintsPalette::UBStartupHintsPalette(QWidget *parent) :
    UBFloatingPalette(Qt::BottomRightCorner,parent)
{
    if(UBSettings::settings()->appStartupHintsEnabled->get().toBool()){
        setFixedSize(320,240);
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(10,28,10,10);
        setLayout(layout);
        QWebView* webView = new QWebView(this);
        webView->setHtml("<html><page><h1>pippo</h1></page></html>");
        layout->addWidget(webView);
        QHBoxLayout* buttonLayout = new QHBoxLayout(this);
        layout->addLayout(buttonLayout);
        mShowNextTime = new QCheckBox(tr("Visible next time"),this);
        mShowNextTime->setCheckState(Qt::Checked);
        connect(mShowNextTime,SIGNAL(stateChanged(int)),this,SLOT(onShowNextTimeStateChanged(int)));
        buttonLayout->addStretch();
        buttonLayout->addWidget(mShowNextTime);
        updatePosition();
    }
    else
        hide();
}

void UBStartupHintsPalette::updatePosition()
{
    //TODO
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

int UBStartupHintsPalette::border()
{
    return 40;
}
