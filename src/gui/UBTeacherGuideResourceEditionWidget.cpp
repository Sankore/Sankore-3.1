#include "UBTeacherGuideResourceEditionWidget.h"

#include "UBTeacherGuideConstantes.h"
#include "core/UBApplication.h"
#include "adaptors/UBSvgSubsetAdaptor.h"
#include "board/UBBoardController.h"
#include "globals/UBGlobals.h"

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

    //N/C - NNE - 20140401
    mpTreeWidget->header()->setDefaultSectionSize(40);

    mpTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    mpTreeWidget->setExpandsOnDoubleClick(false);

    mpAddAMediaItem = new UBAddItem(tr("Add a media"), eUBTGAddSubItemWidgetType_Media, mpTreeWidget);    
    mpAddAFileItem = new UBAddItem(tr("Add a file"), eUBTGAddSubItemWidgetType_File, mpTreeWidget); //Issue 1716 - ALTI/AOU - 20140128
    mpAddALinkItem = new UBAddItem(tr("Add a link"), eUBTGAddSubItemWidgetType_Url, mpTreeWidget);

    mpRootWidgetItem->addChild(mpAddAMediaItem);
    mpRootWidgetItem->addChild(mpAddAFileItem); //Issue 1716 - ALTI/AOU - 20140128
    mpRootWidgetItem->addChild(mpAddALinkItem);

#ifdef Q_WS_MAC
    // on mac and with the custom qt the widget on the tree are not automatically relocated when using the vertical scrollbar. To relocate them we link the valueChange signal of the vertical scrollbar with a local signal to trig a change and a repaint of the tree widget
    connect(mpTreeWidget->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(onSliderMoved(int)));
#endif

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
        //newWidgetItem->setIcon(1, QIcon(":images/close.svg"));

        UBTGActionColumn *actionColumn = new UBTGActionColumn(newWidgetItem);

        mpTreeWidget->setItemWidget(newWidgetItem, 1, actionColumn);

        switch (addSubItemWidgetType) {
        case eUBTGAddSubItemWidgetType_Media: {
            UBTGMediaWidget* mediaWidget = new UBTGMediaWidget(newWidgetItem);
            if (element)
                mediaWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem,0, mediaWidget);
            connect(actionColumn, SIGNAL(clickOnUp()), mediaWidget, SLOT(onUpButton()));
            connect(actionColumn, SIGNAL(clickOnDown()), mediaWidget, SLOT(onDownButton()));
            connect(actionColumn, SIGNAL(clickOnClose()), mediaWidget, SLOT(onClose()));
            break;
        }
        case eUBTGAddSubItemWidgetType_Url: {
            UBTGUrlWidget* urlWidget = new UBTGUrlWidget(newWidgetItem);
            if (element)
                urlWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem, 0, urlWidget);
            connect(actionColumn, SIGNAL(clickOnUp()), urlWidget, SLOT(onUpButton()));
            connect(actionColumn, SIGNAL(clickOnDown()), urlWidget, SLOT(onDownButton()));
            connect(actionColumn, SIGNAL(clickOnClose()), urlWidget, SLOT(onClose()));
            break;
        }
        case eUBTGAddSubItemWidgetType_File: { //Issue 1716 - ALTI/AOU - 20140128
            UBTGFileWidget* fileWidget = new UBTGFileWidget(newWidgetItem);
            if (element)
                fileWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem, 0, fileWidget);
            connect(actionColumn, SIGNAL(clickOnUp()), fileWidget, SLOT(onUpButton()));
            connect(actionColumn, SIGNAL(clickOnDown()), fileWidget, SLOT(onDownButton()));
            connect(actionColumn, SIGNAL(clickOnClose()), fileWidget, SLOT(onClose()));
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

    //For the links and files, we have to add the "student" attribute
    children = getChildrenList(mpAddALinkItem);
    children << getChildrenList(mpAddAFileItem); //Issue 1716 - ALTI/AOU - 20140128
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

    // Remove empty ExternalFiles from Tree :
    for(int i=0; i<mpAddAFileItem->childCount();)
    {
        QTreeWidgetItem* item = mpAddAFileItem->child(i);
        UBTGFileWidget* fileItem = dynamic_cast<UBTGFileWidget*>(mpTreeWidget->itemWidget(item, 0));
        if (fileItem && fileItem->titreFichier().trimmed().isEmpty() && fileItem->path().isEmpty()){ // if no title nor file choosen ...
            QTreeWidgetItem * itemtoBeDeleted = mpAddAFileItem->takeChild(i); // remove item from tree...
            DELETEPTR(itemtoBeDeleted); // and destroy item.
        }
        else{
            i++;
        }
    }

    children << getChildrenList(mpAddAMediaItem);
    children << getChildrenList(mpAddALinkItem);
    children << getChildrenList(mpAddAFileItem); //Issue 1716 - ALTI/AOU - 20140128

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
    children << mpAddAFileItem->takeChildren(); //Issue 1716 - ALTI/AOU - 20140128

    foreach(QTreeWidgetItem* item, children) {
        delete item;
    }
}

void UBTeacherGuideResourceEditionWidget::onActiveSceneChanged()
{
    int activeSceneIndex = UBApplication::boardController->activeSceneIndex();

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
        else if (tagName == "file" && element.attribute("student", "false") == "true") //Issue 1716 - ALTI/AOU - 20140128
            onAddItemClicked(mpAddAFileItem, 0, &element);
    }

    setIsModified(false);
}

bool UBTeacherGuideResourceEditionWidget::isModified()
{
    return mIsModified;
}

void UBTeacherGuideResourceEditionWidget::setIsModified(bool isModified /* = true */)
{
    mIsModified = isModified;
}

bool UBTeacherGuideResourceEditionWidget::hasUserDataInTeacherGuide()
{
    return (mpAddAMediaItem->childCount() > 0)
        || (mpAddALinkItem->childCount() > 0)
        || (mpAddAFileItem->childCount() > 0);
}

#ifdef Q_WS_MAC
void UBTeacherGuideResourceEditionWidget::onSliderMoved(int size)
{
    Q_UNUSED(size);
    if(mpAddAMediaItem)
        mpAddAMediaItem->setExpanded(true);
}
#endif
