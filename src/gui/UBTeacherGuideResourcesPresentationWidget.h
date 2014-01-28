#ifndef UBTEACHERGUIDERESOURCESPRESENTATIONWIDGET_H
#define UBTEACHERGUIDERESOURCESPRESENTATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>

#include "UBTeacherGuideWidgetsTools.h"

class UBTeacherGuideResourcesPresentationWidget : public QWidget
{
    Q_OBJECT

public:
    UBTeacherGuideResourcesPresentationWidget(QWidget *parent = 0);
    ~UBTeacherGuideResourcesPresentationWidget();

    void showData(QVector<tUBGEElementNode*> data);
    void createMediaButtonItem();
    void cleanData();

public slots:
    void onActiveSceneChanged();
    void onAddItemClicked(QTreeWidgetItem* widget,int column);

private:
    bool eventFilter(QObject* object, QEvent* event);

    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonLayout;
    QPushButton* mpModePushButton;
    UBTGDraggableTreeItem* mpTreeWidget;
    QTreeWidgetItem* mpRootWidgetItem;
    QTreeWidgetItem* mpMediaSwitchItem;
    QFrame *mpSeparator;
    QLabel* mpPageNumberLabel;

#ifdef Q_WS_MACX
private slots:
    void onSliderMoved(int size);
#endif
};

#endif // UBTEACHERGUIDERESOURCESPRESENTATIONWIDGET_H
