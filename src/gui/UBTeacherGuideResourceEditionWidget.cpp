#include "UBTeacherGuideResourceEditionWidget.h"

#include "UBTeacherGuideConstantes.h"
#include "core/UBApplication.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "board/UBBoardController.h"

UBTeacherGuideResourceEditionWidget::UBTeacherGuideResourceEditionWidget(QWidget *parent, const char* name):
    QWidget(parent)
{
    mIsModified = false;

    setObjectName(name);
    mpLayout = new QVBoxLayout(this);

    mpTreeWidget = new QTreeWidget(this);
    mpTreeWidget->setStyleSheet("selection-background-color:transparent; padding-bottom:5px; padding-top:5px;");
    mpLayout->addWidget(mpTreeWidget);

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
    mpTreeWidget->setRootIsDecorated(false);
    mpTreeWidget->setIndentation(0);
    mpTreeWidget->setDropIndicatorShown(false);
    mpTreeWidget->header()->close();
    mpTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpTreeWidget->setColumnCount(2);
    mpTreeWidget->header()->setStretchLastSection(false);
    mpTreeWidget->header()->setResizeMode(0, QHeaderView::Stretch);
    mpTreeWidget->header()->setResizeMode(1, QHeaderView::Fixed);
    mpTreeWidget->header()->setDefaultSectionSize(18);
    mpTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);

    mpAddAMediaItem = new UBAddItem(tr("Add a media"), eUBTGAddSubItemWidgetType_Media, mpTreeWidget);
    mpAddALinkItem = new UBAddItem(tr("Add a link"), eUBTGAddSubItemWidgetType_Url, mpTreeWidget);

    mpRootWidgetItem->addChild(mpAddAMediaItem);
    mpRootWidgetItem->addChild(mpAddALinkItem);

    connect(mpTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onAddItemClicked(QTreeWidgetItem*,int)));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
    UBSvgSubsetAdaptor::addElementToBeStored(QString("resourcesGuide"), this);

}

void UBTeacherGuideResourceEditionWidget::onAddItemClicked(QTreeWidgetItem* widget, int column, QDomElement *element)
{
    mIsModified = true;
    int addSubItemWidgetType = widget->data(column, Qt::UserRole).toInt();
    if (addSubItemWidgetType != eUBTGAddSubItemWidgetType_None) {
        QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(widget);
        newWidgetItem->setData(column, Qt::UserRole, eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setData(1, Qt::UserRole, eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setIcon(1, QIcon(":images/close.svg"));

        switch (addSubItemWidgetType) {
        case eUBTGAddSubItemWidgetType_Media: {
            UBTGMediaWidget* mediaWidget = new UBTGMediaWidget(widget);
            if (element)
                mediaWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem,0, mediaWidget);
            break;
        }
        case eUBTGAddSubItemWidgetType_Url: {
            UBTGUrlWidget* urlWidget = new UBTGUrlWidget();
            if (element)
                urlWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem, 0, urlWidget);
            break;
        }
        default:
            delete newWidgetItem;
            qCritical() << "onAddItemClicked no action set";
            return;
        }

        if (addSubItemWidgetType != eUBTGAddSubItemWidgetType_None && !widget->isExpanded())
            widget->setExpanded(true);
        else {
            //to update the tree and subtrees
            widget->setExpanded(false);
            widget->setExpanded(true);
        }
    }
    else if (column == 1 && addSubItemWidgetType == eUBTGAddSubItemWidgetType_None) {
        UBTGMediaWidget* media = dynamic_cast<UBTGMediaWidget*>(mpTreeWidget->itemWidget(widget, 0));
        if (media)
            media->removeSource();
        int index = mpTreeWidget->currentIndex().row();
        QTreeWidgetItem* toBeDeletedWidgetItem = widget->parent()->takeChild(index);
        delete toBeDeletedWidgetItem;
    }
}

UBTeacherGuideResourceEditionWidget::~UBTeacherGuideResourceEditionWidget()
{

}


QVector<tIDataStorage*> UBTeacherGuideResourceEditionWidget::save(int pageIndex)
{
    QVector<tIDataStorage*> result;

    if(pageIndex == 0 || pageIndex != UBApplication::boardController->currentPage())
        return result;

    QList<QTreeWidgetItem*> children;

    children << getChildrenList(mpAddAMediaItem);
    foreach(QTreeWidgetItem* widgetItem, children) {
        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidget->itemWidget( widgetItem, 0))->saveData();
        if (node) {
            tIDataStorage *data = new tIDataStorage(node->name, eElementType_UNIQUE);

            foreach(QString currentKey, node->attributes.keys())
                data->attributes.insert(currentKey, node->attributes.value(currentKey));

            result << data;
        }
    }

    //For the link we have to add the "student" attribute
    children = getChildrenList(mpAddALinkItem);
    foreach(QTreeWidgetItem* widgetItem, children) {
        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidget->itemWidget( widgetItem, 0))->saveData();
        if (node) {
            tIDataStorage *data = new tIDataStorage(node->name, eElementType_UNIQUE);

            foreach(QString currentKey, node->attributes.keys())
                data->attributes.insert(currentKey, node->attributes.value(currentKey));

            data->attributes.insert("student", "true");

            result << data;
        }
    }


    return result;
}

QVector<tUBGEElementNode*> UBTeacherGuideResourceEditionWidget::getData()
{
    QVector<tUBGEElementNode*> result;
    QList<QTreeWidgetItem*> children;

    children << getChildrenList(mpAddAMediaItem);
    children << getChildrenList(mpAddALinkItem);

    foreach(QTreeWidgetItem* widgetItem, children) {
        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidget->itemWidget( widgetItem, 0))->saveData();
        if (node)
            result << node;
    }

    return result;
}

QList<QTreeWidgetItem*> UBTeacherGuideResourceEditionWidget::getChildrenList( QTreeWidgetItem* widgetItem)
{
    QList<QTreeWidgetItem*> result;
    for (int i = 0; i < widgetItem->childCount(); i += 1)
        result << widgetItem->child(i);
    return result;
}

void UBTeacherGuideResourceEditionWidget::cleanData()
{
    QList<QTreeWidgetItem*> children;

    children << mpAddAMediaItem->takeChildren();
    children << mpAddALinkItem->takeChildren();

    foreach(QTreeWidgetItem* item, children) {
        delete item;
    }
}

void UBTeacherGuideResourceEditionWidget::onActiveSceneChanged()
{
    int activeSceneIndex = UBApplication::boardController->activeSceneIndex();
    qDebug() << activeSceneIndex;

    if (UBApplication::boardController->pageFromSceneIndex(activeSceneIndex) != 0)
        load(UBSvgSubsetAdaptor::readTeacherGuideNode(activeSceneIndex));
}

void UBTeacherGuideResourceEditionWidget::onActiveDocumentChanged()
{
    int activeSceneIndex = UBApplication::boardController->activeSceneIndex();
    if (UBApplication::boardController->pageFromSceneIndex(activeSceneIndex) != 0)
        load(UBSvgSubsetAdaptor::readTeacherGuideNode(activeSceneIndex));
}

void UBTeacherGuideResourceEditionWidget::load(QDomDocument doc)
{
    cleanData();

    for (QDomElement element = doc.documentElement().firstChildElement();
         !element.isNull(); element = element.nextSiblingElement()) {
        QString tagName = element.tagName();
        if (tagName == "media")
            onAddItemClicked(mpAddAMediaItem, 0, &element);
        else if (tagName == "link" && element.attribute("student", "false") == "true")
            onAddItemClicked(mpAddALinkItem, 0, &element);
    }
}

bool UBTeacherGuideResourceEditionWidget::isModified()
{
    return mIsModified;
}

#ifdef Q_WS_MAC
void UBTeacherGuideResourceEditionWidget::onSliderMoved(int size)
{
    Q_UNUSED(size);
    if(mpAddAMediaItem)
        mpAddAMediaItem->setExpanded(true);
}
#endif
