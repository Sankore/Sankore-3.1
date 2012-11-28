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


#ifndef UBCREATELINKPALETTE_H
#define UBCREATELINKPALETTE_H

class QVBoxLayout;
class QStackedWidget;
class QToolButton;
class QWidget;
class UBGraphicsItemAction;
class QLineEdit;

#include <QMap>
#include "UBFloatingPalette.h"

class UBCreateLinkPalette : public UBFloatingPalette
{
    Q_OBJECT;
public:
    explicit UBCreateLinkPalette(QWidget *parent = 0);
    ~UBCreateLinkPalette();
signals:
    void definedAction(UBGraphicsItemAction* action);

public slots:

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void showEvent(QShowEvent *event);

private:
    void close();
    void adjustGeometry();
    void init();
    QToolButton* addInitiaWidgetButton(QString textButton, QString iconPath);
    QWidget* addBasicFunctionaliltiesToWidget(QWidget* centralWidget);

    QVBoxLayout* mLayout;
    QStackedWidget* mStackedWidget;
    QWidget* mInitialWidget;
    QWidget* mAudioWidget;
    QWidget* mPageLinkWidget;
    QWidget* mUrlLinkWidget;

    QLineEdit* mUrlLineEdit;

private slots:
    void onBackButtonClicked();
    void onPlayAudioClicked();
    void onAddLinkToPageClicked();
    void onAddLinkToWebClicked();

    void onOkAudioClicked();
    void onOkLinkToPageClicked();
    void onOkLinkToWebClicked();
};

#endif // UBCREATELINKPALETTE_H
