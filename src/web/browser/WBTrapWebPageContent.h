#ifndef WB_TRAP_WEB_PAGE_CONTENT_H
#define WB_TRAP_WEB_PAGE_CONTENT_H

#include <QtGui>
#include <QWebView>

#include "gui/UBActionPalette.h"
#include "WBWebView.h"


class WBTrapBar : public UBActionPalette
{
    Q_OBJECT;

public:
    WBTrapBar(QWidget *parent = NULL);

private:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);

};


class WBTrapWebPageContentWindow : public QDialog
{
public:
    WBTrapWebPageContentWindow(QObject *controller, QWidget *parent = NULL);
    virtual ~WBTrapWebPageContentWindow();

    void setUrl(const QUrl &url);
    QWebView *webView() const {return mTrapContentPreview;}
    QComboBox *itemsComboBox() const {return mSelectContentCombobox;}
    QLineEdit *applicationNameLineEdit() const {return mApplicationNameEdit;}

private:
    QObject *mController;

    QHBoxLayout *mTrapApplicationHLayout;
    QVBoxLayout *mTrapApplicationVLayout;

    WBTrapBar *mTrapActionsBar;

    QHBoxLayout *mSelectContentLayout;
    QLabel *mSelectContentLabel;
    QComboBox *mSelectContentCombobox;

    WBWebView *mTrapContentPreview;
    UBWebTrapMouseEventMask *mTrapingWidgetMask;

    QHBoxLayout *mApplicationNameLayout;
    QLabel *mApplicationNameLabel;
    QLineEdit *mApplicationNameEdit;

    QToolButton *mShowDisclaimerToolButton;

    QWebHitTestResult mLastHitTestResult;
};

#endif //WB_TRAP_WEB_PAGE_CONTENT_H