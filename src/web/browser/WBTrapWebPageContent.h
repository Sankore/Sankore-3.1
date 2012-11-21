#ifndef WB_TRAP_WEB_PAGE_CONTENT_H
#define WB_TRAP_WEB_PAGE_CONTENT_H

#include <QtGui>

#include "gui/UBActionPalette.h"

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
    WBTrapWebPageContentWindow(QWidget *parent = NULL);
    virtual ~WBTrapWebPageContentWindow();

private:
    QHBoxLayout *mTrapApplicationHLayout;
    QVBoxLayout *mTrapApplicationVLayout;

    WBTrapBar *mTrapActionsBar;

    QHBoxLayout *mSelectContentLayout;
    QLabel *mSelectContentLabel;
    QComboBox *mSelectContentCombobox;

    QWidget *mTrapContentPreview;

    QHBoxLayout *mApplicationNameLayout;
    QLabel *mApplicationNameLabel;
    QLineEdit *mApplicationNameEdit;
};

#endif //WB_TRAP_WEB_PAGE_CONTENT_H