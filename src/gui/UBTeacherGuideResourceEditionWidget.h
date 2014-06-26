#ifndef UBTEACHERGUIDERESOURCEEDITIONWIDGET_H
#define UBTEACHERGUIDERESOURCEEDITIONWIDGET_H

#include "interfaces/IDataStorage.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDomElement>

#include "UBTeacherGuideWidgetsTools.h"

class UBTeacherGuideResourceEditionWidget : public QWidget , public IDataStorage
{
    Q_OBJECT

public:
    explicit UBTeacherGuideResourceEditionWidget(QWidget *parent = 0, const char *name="UBTeacherGuideResourceEditionWidget");
    ~UBTeacherGuideResourceEditionWidget();

    QVector<tIDataStorage*> save(int pageIndex);
    void load(QDomDocument doc);

    QVector<tUBGEElementNode*> getData();

    QList<QTreeWidgetItem*> getChildrenList(QTreeWidgetItem* widgetItem);
    void cleanData();

    bool isModified();
    void setIsModified(bool isModified = true);
    bool hasUserDataInTeacherGuide();

public slots:
    void onAddItemClicked(QTreeWidgetItem* widget, int column, QDomElement *element = 0);
    void onActiveSceneChanged();

private slots:
    void onActiveDocumentChanged();
#ifdef Q_WS_MACX
    void onSliderMoved(int size);
#endif

private:
    QVBoxLayout* mpLayout;
    QLabel* mpDocumentTitle;
    QLabel* mpPageNumberLabel;
    QFrame* mpSeparator;
    QTreeWidget* mpTreeWidget;
    QTreeWidgetItem* mpRootWidgetItem;
    UBAddItem* mpAddAMediaItem;
    UBAddItem* mpAddALinkItem;
    UBAddItem* mpAddAFileItem; //Issue 1716 - ALTI/AOU - 20140128

    bool mIsModified;
};

#endif // UBTEACHERGUIDERESOURCEEDITIONWIDGET_H
