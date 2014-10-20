/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QPushButton>
#include <QDomDocument>
#include <QScrollArea>

#include "UBTeacherGuideWidget.h"

#include "adaptors/UBSvgSubsetAdaptor.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"

#include "globals/UBGlobals.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBStylusPalette.h"
#include "gui/UBActionPalette.h"
#include "gui/UBMainWindow.h"

#include "web/UBWebController.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "domain/UBGraphicsTextItem.h"

#include "core/memcheck.h"

#include "UBTeacherGuideConstantes.h"
#include "frameworks/UBWidgetUtils.h"




/***************************************************************************
 *               class    UBTeacherGuideEditionWidget                      *
 ***************************************************************************/

UBTeacherGuideEditionWidget::UBTeacherGuideEditionWidget(QWidget *parent, const char* name) :
    QWidget(parent)
  , mpLayout(NULL)
  , mpDocumentTitle(NULL)
  , mpPageNumberLabel(NULL)
  , mpPageTitle(NULL)
  , mpComment(NULL)
  , mpSeparator(NULL)
  , mpTreeWidget(NULL)
  , mpRootWidgetItem(NULL)
  , mpAddAnActionItem(NULL)
  , mIsModified(false)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");
    mpLayout->addWidget(mpPageNumberLabel);
    // tree basic configuration

    if (UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()) {
        mpDocumentTitle = new QLabel(this);
        mpDocumentTitle->setObjectName("UBTGPresentationDocumentTitle");
        mpLayout->addWidget(mpDocumentTitle);
    }

    mpPageTitle = new UBTGAdaptableText(0, this);
    mpPageTitle->setObjectName("UBTGEditionPageTitle");
    mpPageTitle->setPlaceHolderText(tr("Type title here ..."));
    mpPageTitle->setTabChangesFocus(true);
    mpLayout->addWidget(mpPageTitle);

    mpComment = new UBTGAdaptableText(0, this);
    mpComment->setObjectName("UBTGEditionComment");
    mpComment->setPlaceHolderText(tr("Type comment here ..."));
    mpComment->setTabChangesFocus(true);
    mpLayout->addWidget(mpComment);

    mpSeparator = new QFrame(this);
    mpSeparator->setObjectName("UBTGSeparator");
    mpSeparator->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);

    mpLayout->addWidget(mpSeparator);

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

    connect(mpTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onAddItemClicked(QTreeWidgetItem*,int)));

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));

    connect(mpPageTitle, SIGNAL(textChanged()), this, SLOT(setIsModified()));
    connect(mpComment, SIGNAL(textChanged()), this, SLOT(setIsModified()));

#ifdef Q_WS_MAC
    // on mac and with the custom qt the widget on the tree are not automatically relocated when using the vertical scrollbar. To relocate them we link the valueChange signal of the vertical scrollbar with a local signal to trig a change and a repaint of the tree widget
    connect(mpTreeWidget->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(onSliderMoved(int)));
#endif

    mpAddAnActionItem = new UBAddItem(tr("Add an action"), eUBTGAddSubItemWidgetType_Action, mpTreeWidget);
    mpAddAFileItem = new UBAddItem(tr("Add a file"), eUBTGAddSubItemWidgetType_File, mpTreeWidget); //Issue 1716 - ALTI/AOU - 20140128
    mpAddALinkItem = new UBAddItem(tr("Add a link"), eUBTGAddSubItemWidgetType_Url, mpTreeWidget);

    mpRootWidgetItem->addChild(mpAddAnActionItem);
    mpRootWidgetItem->addChild(mpAddAFileItem); //Issue 1716 - ALTI/AOU - 20140128
    mpRootWidgetItem->addChild(mpAddALinkItem);    

    if (UBSettings::settings()->teacherGuideLessonPagesActivated->get().toBool()) {
        UBSvgSubsetAdaptor::addElementToBeStored(QString("teacherGuide"), this);
        connect(UBApplication::boardController, SIGNAL(documentSet(UBDocumentProxy*)), this, SLOT(onActiveDocumentChanged()));
    }
}

UBTeacherGuideEditionWidget::~UBTeacherGuideEditionWidget()
{
    DELETEPTR(mpDocumentTitle);
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpPageTitle);
    DELETEPTR(mpComment);
    DELETEPTR(mpSeparator);
    DELETEPTR(mpAddAnActionItem);
    DELETEPTR(mpAddALinkItem);
    DELETEPTR(mpAddAFileItem);
    DELETEPTR(mpTreeWidget);
    DELETEPTR(mpLayout);
}

void UBTeacherGuideEditionWidget::showEvent(QShowEvent* event)
{
    //Issue 1454 - CFA - 20140306 : correction des raccourcis clavier fleches gauche et droite
    //setFocus();
    QWidget::showEvent(event);
}

void UBTeacherGuideEditionWidget::onActiveDocumentChanged()
{
    int activeSceneIndex = UBApplication::boardController->activeSceneIndex();
    if (UBApplication::boardController->pageFromSceneIndex(activeSceneIndex) != 0)
        load(UBSvgSubsetAdaptor::readTeacherGuideNode(activeSceneIndex));
}

void UBTeacherGuideEditionWidget::setIsModified(bool isModified /* = true */)
{
    mIsModified = isModified;
}

#ifdef Q_WS_MAC
void UBTeacherGuideEditionWidget::onSliderMoved(int size)
{
    if(mpAddAnActionItem)
        mpAddAnActionItem->setExpanded(true);
}
#endif

void UBTeacherGuideEditionWidget::load(QDomDocument doc)
{
    cleanData();
    for (QDomElement element = doc.documentElement().firstChildElement();
         !element.isNull(); element = element.nextSiblingElement()) {
        QString tagName = element.tagName();

        if (tagName == "title")
            mpPageTitle->setInitialText(element.attribute("value"));
        else if (tagName == "comment")
            mpComment->setInitialText(element.attribute("value"));
        else if (tagName == "action")
            onAddItemClicked(mpAddAnActionItem, 0, &element);
        else if (tagName == "link" && element.attribute("student", "false") == "false")
            onAddItemClicked(mpAddALinkItem, 0, &element);
        else if (tagName == "file" && element.attribute("student", "false") == "false") //Issue 1716 - ALTI/AOU - 20140128
            onAddItemClicked(mpAddAFileItem, 0, &element);
    }

    //backward compatibility
    if (doc.childNodes().length() == 1 && doc.childNodes().at(0).toElement().tagName() == "teacherBar"){
        QDomElement element = doc.childNodes().at(0).toElement();
        QDomNamedNodeMap nodeMap = element.attributes();
        for (int i= 0; i < nodeMap.size() ; i+=1){
            QDomNode node = nodeMap.item(i);
            QString attributeName = node.nodeName();
            QString value = element.attribute(attributeName);
            qDebug() << attributeName<< " " << value;
            if(attributeName.contains("title") && value.length())
                mpPageTitle->setInitialText(value);
            else if(attributeName.contains("Teacher") && value.length()){
                QDomElement teacherAction(element);
                teacherAction.setTagName("action");
                teacherAction.setAttribute("owner","0");
                teacherAction.setAttribute("task",value);
                onAddItemClicked(mpAddAnActionItem,0,&teacherAction);
            }
            else if (attributeName.contains("Student") && value.length()){
                QDomElement studentAction(element);
                studentAction.setTagName("action");
                studentAction.setAttribute("owner","1");
                studentAction.setAttribute("task",value);
                onAddItemClicked(mpAddAnActionItem,0,&studentAction);
            }
        }
    }

    setIsModified(false);
}

QVector<tIDataStorage*> UBTeacherGuideEditionWidget::save(int pageIndex)
{
    QVector<tIDataStorage*> result;
    if (pageIndex != UBApplication::boardController->currentPage())
        return result;
    if (pageIndex == 0) // Issue 1517 - ALTI/AOU - 20131209 : Dans cette fonction, on ne traite pas la page zero.
        return result;
    tIDataStorage* data;

    data = new tIDataStorage();
    data->name = "title";
    data->type = eElementType_UNIQUE;
    data->attributes.insert("value", mpPageTitle->text());
    if (mpPageTitle->text().length())
        result << data;

    data = new tIDataStorage();
    data->name = "comment";
    data->type = eElementType_UNIQUE;
    data->attributes.insert("value", mpComment->text());
    if (mpComment->text().length())
        result << data;

    QList<QTreeWidgetItem*> children = getChildrenList(mpAddAnActionItem);
    children += getChildrenList(mpAddALinkItem);
    children += getChildrenList(mpAddAFileItem); //Issue 1716 - ALTI/AOU - 20140128

    foreach(QTreeWidgetItem* widgetItem, children) {
        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidget->itemWidget( widgetItem, 0))->saveData();
        if (node) {
            data = new tIDataStorage();
            data->name = node->name;
            data->type = eElementType_UNIQUE;
            foreach(QString currentKey, node->attributes.keys())
                data->attributes.insert(currentKey, node->attributes.value(currentKey));
            result << data;
        }
    }
    return result;
}

void UBTeacherGuideEditionWidget::onActiveSceneChanged()
{
    int currentPage = UBApplication::boardController->currentPage();
    if (currentPage > 0) {
        cleanData();
        load( UBSvgSubsetAdaptor::readTeacherGuideNode( UBApplication::boardController->activeSceneIndex()));
        mpPageNumberLabel->setText(tr("Page: %0").arg(currentPage));
        UBDocumentProxy* documentProxy = UBApplication::boardController->selectedDocument();
        if (mpDocumentTitle)
            mpDocumentTitle->setText(documentProxy->metaData(UBSettings::sessionTitle).toString());
    }
    //Issue 1454 - CFA - 20140306 : correction des raccourcis clavier fleches gauche et droite
    UBApplication::boardController->controlView()->setFocus();
}

void UBTeacherGuideEditionWidget::cleanData()
{
    mpPageTitle->resetText();
    mpComment->resetText();
    QList<QTreeWidgetItem*> children = mpAddAnActionItem->takeChildren();
    children += mpAddALinkItem->takeChildren();
    children += mpAddAFileItem->takeChildren(); //Issue 1716 - ALTI/AOU - 20140128

    foreach(QTreeWidgetItem* item, children) {
        DELETEPTR(item);
    }
}

QList<QTreeWidgetItem*> UBTeacherGuideEditionWidget::getChildrenList( QTreeWidgetItem* widgetItem)
{
    QList<QTreeWidgetItem*> result;
    for (int i = 0; i < widgetItem->childCount(); i += 1)
        result << widgetItem->child(i);
    return result;
}

QVector<tUBGEElementNode*> UBTeacherGuideEditionWidget::getPageAndCommentData()
{
    QVector<tUBGEElementNode*> result;
    tUBGEElementNode* pageTitle = new tUBGEElementNode();
    pageTitle->name = "pageTitle";
    pageTitle->attributes.insert("value", mpPageTitle->text());
    result << pageTitle;

    tUBGEElementNode* comment = new tUBGEElementNode();
    comment->name = "comment";
    comment->attributes.insert("value", mpComment->text());
    result << comment;
    return result;
}

QVector<tUBGEElementNode*> UBTeacherGuideEditionWidget::getData()
{
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

    QVector<tUBGEElementNode*> result;
    QList<QTreeWidgetItem*> children = getChildrenList(mpAddAnActionItem);
    children += getChildrenList(mpAddALinkItem);
    children += getChildrenList(mpAddAFileItem); //Issue 1716 - ALTI/AOU - 20140128

    result << getPageAndCommentData();
    foreach(QTreeWidgetItem* widgetItem, children) {
        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidget->itemWidget( widgetItem, 0))->saveData();
        if (node)
            result << node;
    }
    return result;
}

void UBTeacherGuideEditionWidget::onAddItemClicked(QTreeWidgetItem* widget, int column, QDomElement *element)
{
    setIsModified();

    int addSubItemWidgetType = widget->data(column, Qt::UserRole).toInt();
    if (addSubItemWidgetType != eUBTGAddSubItemWidgetType_None) {
        QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(widget);
        newWidgetItem->setData(column, Qt::UserRole, eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setData(1, Qt::UserRole, eUBTGAddSubItemWidgetType_None);

        UBTGActionColumn *actionColumn = new UBTGActionColumn(newWidgetItem);
        mpTreeWidget->setItemWidget(newWidgetItem, 1, actionColumn);

        switch (addSubItemWidgetType) {
        case eUBTGAddSubItemWidgetType_Action: {
            UBTGActionWidget* actionWidget = new UBTGActionWidget(newWidgetItem);
            if (element)
                actionWidget->initializeWithDom(*element);

            mpTreeWidget->setItemWidget(newWidgetItem, 0, actionWidget);
            connect(actionColumn, SIGNAL(clickOnUp()), actionWidget, SLOT(onUpButton()));
            connect(actionColumn, SIGNAL(clickOnDown()), actionWidget, SLOT(onDownButton()));
            connect(actionColumn, SIGNAL(clickOnClose()), actionWidget, SLOT(onClose()));
            connect(actionWidget, SIGNAL(hasChanged()), this, SLOT(setIsModified()));
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

bool UBTeacherGuideEditionWidget::isModified()
{
    return mIsModified;
}

void UBTeacherGuideEditionWidget::teacherGuideChanged()
{
    setIsModified(true);
}

bool UBTeacherGuideEditionWidget::hasUserDataInTeacherGuide()
{
    return (mpPageTitle->text().length() > 0)
        || (mpComment->text().length() > 0)
        || (mpAddAnActionItem->childCount() > 0)
        || (mpAddALinkItem->childCount() > 0)
        || (mpAddAFileItem->childCount() > 0); //Issue 1716 - ALTI/AOU - 20140128
}

/***************************************************************************
 *           class    UBTeacherGuidePresentationWidget                     *
 ***************************************************************************/

UBTeacherGuidePresentationWidget::UBTeacherGuidePresentationWidget(QWidget *parent, const char *name) :
    QWidget(parent)
  , mpPageTitle(NULL)
  , mpComment(NULL)
  , mpLayout(NULL)
  , mpButtonTitleLayout(NULL)
  , mpDocumentTitle(NULL)
  , mpPageNumberLabel(NULL)
  , mpSeparator(NULL)
  , mpModePushButton(NULL)
  , mpTreeWidget(NULL)
  , mpRootWidgetItem(NULL)
  , mpMediaSwitchItem(NULL)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");

    mpLayout->addWidget(mpPageNumberLabel);

    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/teacherGuide/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);
    mpModePushButton->installEventFilter(this);

    connect(mpModePushButton, SIGNAL(clicked()), parentWidget(), SLOT(changeMode()));
    mpButtonTitleLayout->addWidget(mpModePushButton);

    if (UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()) {
        mpDocumentTitle = new QLabel(this);
        mpDocumentTitle->setObjectName("UBTGPresentationDocumentTitle");
        mpButtonTitleLayout->addWidget(mpDocumentTitle);
    }

    mpLayout->addLayout(mpButtonTitleLayout);

    mpPageTitle = new UBTGAdaptableText(0, this);
    mpPageTitle->setObjectName("UBTGPresentationPageTitle");
    mpPageTitle->setReadOnly(true);
    mpPageTitle->setStyleSheet("background-color:transparent");
    mpLayout->addWidget(mpPageTitle);

    mpComment = new UBTGAdaptableText(0, this);
    mpComment->setObjectName("UBTGPresentationComment");
    mpComment->setReadOnly(true);
    mpComment->setStyleSheet("background-color:transparent");
    mpLayout->addWidget(mpComment);

    mpSeparator = new QFrame(this);
    mpSeparator->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparator->setObjectName("UBTGSepartor");
    mpLayout->addWidget(mpSeparator);

    mpTreeWidget = new UBTGDraggableTreeItem(this);
    mpLayout->addWidget(mpTreeWidget);

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
    mpTreeWidget->setDragEnabled(true);
    mpTreeWidget->setRootIsDecorated(false);
    mpTreeWidget->setIndentation(0);
    mpTreeWidget->setDropIndicatorShown(false);
    mpTreeWidget->header()->close();
    mpTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpTreeWidget->setStyleSheet("selection-background-color:transparent; padding-bottom:5px; padding-top:5px; ");
    mpTreeWidget->setIconSize(QSize(24,24));
    connect(mpTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onAddItemClicked(QTreeWidgetItem*,int)));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));

#ifdef Q_WS_MAC
    // on mac and with the custom qt the widget on the tree are not automatically relocated when using the vertical scrollbar. To relocate them we link the valueChange signal of the vertical scrollbar witht a local signal to trig a change and a repaint of the tree widget
    connect(mpTreeWidget->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(onSliderMoved(int)));
#endif
}

UBTeacherGuidePresentationWidget::~UBTeacherGuidePresentationWidget()
{
    DELETEPTR(mpComment);
    DELETEPTR(mpPageTitle);
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpSeparator);
    DELETEPTR(mpMediaSwitchItem);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpDocumentTitle);
    DELETEPTR(mpButtonTitleLayout);
    DELETEPTR(mpTreeWidget);
    DELETEPTR(mpLayout);
}

#ifdef Q_WS_MAC
void UBTeacherGuidePresentationWidget::onSliderMoved(int size)
{
    Q_UNUSED(size);
    if(mpMediaSwitchItem)
        mpMediaSwitchItem->setExpanded(true);
}
#endif

bool UBTeacherGuidePresentationWidget::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
        return true;
    return false;
}

void UBTeacherGuidePresentationWidget::cleanData()
{
    mpPageTitle->showText("");
    mpComment->showText("");
    //tree clean
    QList<QTreeWidgetItem*> itemToRemove = mpRootWidgetItem->takeChildren();
    foreach(QTreeWidgetItem* eachItem, itemToRemove) {
        DELETEPTR(eachItem);
    }
    // the mpMediaSwitchItem is deleted by the previous loop but the pointer is not set to zero
    mpMediaSwitchItem = NULL;
}

void UBTeacherGuidePresentationWidget::onActiveSceneChanged()
{
    cleanData();
    mpPageNumberLabel->setText(tr("Page: %0").arg(UBApplication::boardController->currentPage()));
    UBDocumentProxy* documentProxy = UBApplication::boardController->selectedDocument();
    if (mpDocumentTitle)
        mpDocumentTitle->setText( documentProxy->metaData(UBSettings::sessionTitle).toString());
}

void UBTeacherGuidePresentationWidget::createMediaButtonItem()
{
    if (!mpMediaSwitchItem) {
        mpMediaSwitchItem = new QTreeWidgetItem(mpRootWidgetItem);
        mpMediaSwitchItem->setText(0, "+");
        mpMediaSwitchItem->setExpanded(false);
        mpMediaSwitchItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_EXPAND);
        mpMediaSwitchItem->setData(0, Qt::BackgroundRole, QVariant(QColor(200, 200, 200)));
        mpMediaSwitchItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 16)));
        mpMediaSwitchItem->setData(0, Qt::TextAlignmentRole, QVariant(Qt::AlignCenter));
        mpRootWidgetItem->addChild(mpMediaSwitchItem);
    }
}

void UBTeacherGuidePresentationWidget::showData( QVector<tUBGEElementNode*> data)
{
    cleanData();
 #ifdef Q_WS_MAC
    if(mpMediaSwitchItem && mpMediaSwitchItem->isDisabled()){
        mpRootWidgetItem->removeChild(mpMediaSwitchItem);
        DELETEPTR(mpMediaSwitchItem);
    }
#endif

    foreach(tUBGEElementNode* element, data) {
        if (element->name == "pageTitle")
            mpPageTitle->showText(element->attributes.value("value"));
        else if (element->name == "comment")
            mpComment->showText(element->attributes.value("value"));
        else if (element->name == "action") {
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpRootWidgetItem);
            newWidgetItem->setFlags(Qt::ItemIsEnabled);

            QString colorString = element->attributes.value("owner").toInt() == 0 ? "blue" : "green";
            UBTGAdaptableText* textWidget = new UBTGAdaptableText(newWidgetItem, 0);

            //N/C - NNE - 20140326 : Text are truncated
            textWidget->setStyleSheet( "QWidget {background: #EEEEEE; border:none; color:" + colorString + ";}");

            textWidget->showText(element->attributes.value("task"));
            textWidget->document()->setDefaultFont( QFont(QApplication::font().family(), 11));

            mpTreeWidget->setItemWidget(newWidgetItem, 0, textWidget);
        }else if (element->name == "link") {
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem( mpMediaSwitchItem);
            newWidgetItem->setIcon(0, QIcon(":images/teacherGuide/link_24x24.svg"));
            newWidgetItem->setText(0, element->attributes.value("title"));
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_URL);
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnUrl, QVariant(element->attributes.value("url")));
            newWidgetItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 11)));
            newWidgetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpRootWidgetItem->addChild(newWidgetItem);
        }else if (element->name == "file") { //Issue 1716 - ALTI/AOU - 20140128
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
            if (element->attributes.value("path").isEmpty()){
                newWidgetItem->setIcon(0, QIcon(":images/teacherGuide/document_large_warning.gif")); // different icon, if no file choosed.
            }
            else{
                newWidgetItem->setIcon(0, QIcon(":images/teacherGuide/document_large.gif"));
            }
            newWidgetItem->setText(0, element->attributes.value("title"));
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_FILE);
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnUrl, element->attributes.value("path"));
            newWidgetItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 11)));
            newWidgetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpRootWidgetItem->addChild(newWidgetItem);
        }
    }

#ifdef Q_WS_MAC
    //HACK: on mac only this hack allows to correctly diplay a teacher bar containing only actions if those
    // actions need more that the vertical available space.
    // In this particular case if mpMediaSwitchItem isn't added to the widget tree then the vertical scrollbar
    // doesn't send any signal when moved.

    if(!mpMediaSwitchItem){
        mpMediaSwitchItem = new QTreeWidgetItem(mpRootWidgetItem);
        mpMediaSwitchItem->setDisabled(true);
        mpRootWidgetItem->addChild(mpMediaSwitchItem);
    }
    else{
        onAddItemClicked(mpMediaSwitchItem,0);
        onAddItemClicked(mpMediaSwitchItem,0);
    }
#endif
}

void UBTeacherGuidePresentationWidget::onAddItemClicked(QTreeWidgetItem* widget, int column)
{
    int associateAction = widget->data(column, tUBTGTreeWidgetItemRole_HasAnAction).toInt();
    if (column == 0 && associateAction != tUBTGActionAssociateOnClickItem_NONE) {
        switch (associateAction) {
        case tUBTGActionAssociateOnClickItem_EXPAND:
            widget->setExpanded(!widget->isExpanded());
            if (widget->isExpanded()){
#ifdef Q_WS_MAC
                for(int i = 0 ; i < mpMediaSwitchItem->childCount(); i+=1 ){
                    QTreeWidgetItem* eachItem = mpMediaSwitchItem->child(i);
                    eachItem->setHidden(false);
                }
#endif
                mpMediaSwitchItem->setText(0, "-");
            }
            else{
#ifdef Q_WS_MAC
                for(int i = 0 ; i < mpMediaSwitchItem->childCount(); i+=1 ){
                    QTreeWidgetItem* eachItem = mpMediaSwitchItem->child(i);
                    eachItem->setHidden(true);
                }
#endif
                mpMediaSwitchItem->setText(0, "+");
            }
            break;
        case tUBTGActionAssociateOnClickItem_URL:
            widget->data(column, tUBTGTreeWidgetItemRole_HasAnUrl).toString();
            UBApplication::webController->loadUrl( QUrl( widget->data(column, tUBTGTreeWidgetItemRole_HasAnUrl).toString()));
            break;
        case tUBTGActionAssociateOnClickItem_FILE: { //Issue 1716 - ALTI/AOU - 20140128
            widget->data(column, tUBTGTreeWidgetItemRole_HasAnUrl).toString();

            QString pathFile = widget->data(column, tUBTGTreeWidgetItemRole_HasAnUrl).toString();
            QString pathDocument = UBApplication::boardController->selectedDocument()->persistencePath();
            QString fullPathFile = pathDocument + "/" + pathFile;

            if ( pathFile.isEmpty() || ! QFile::exists(fullPathFile) ) // Si le fichier n'existe pas, on prévient l'utilisateur
            {
                UBApplication::showMessage(tr("File not found"));
            }
            else
            {
                QCursor oldCursor = cursor();
                setCursor(Qt::WaitCursor);
                if ( ! QDesktopServices::openUrl(QUrl("file:///" + fullPathFile)) ){
                    UBApplication::showMessage(tr("No application was found to handle this file type"));
                }
                setCursor(oldCursor);
            }
            break;
        }
        default:
            qDebug() << "associateAction no action set " << associateAction;
        }
    }
}

/***************************************************************************
 *                  class   UBTeacherGuidePageZeroWidget                   *
 ***************************************************************************/
UBTeacherGuidePageZeroWidget::UBTeacherGuidePageZeroWidget(QWidget* parent, const char* name) :
    QWidget(parent)
  , mpLayout(NULL)
  , mpButtonTitleLayout(NULL)
  , mpContainerWidgetLayout(NULL)
  , mpModePushButton(NULL)
  , mpPageNumberLabel(NULL)
  , mpScrollArea(NULL)
  , mpContainerWidget(NULL)
  , mpSessionTitle(NULL)
  , mpSeparatorSessionTitle(NULL)
  , mpAuthorsLabel(NULL)
  , mpAuthors(NULL)
  , mpSeparatorAuthors(NULL)
  , mpCreationLabel(NULL)
  , mpLastModifiedLabel(NULL)
  , mpObjectivesLabel(NULL)
  , mpObjectives(NULL)
  , mpSeparatorObjectives(NULL)
  , mpIndexLabel(NULL)
  , mpKeywordsLabel(NULL)
  , mpKeywords(NULL)
  , mpSchoolLevelItemLabel(NULL)
  , mpSchoolLevelBox(NULL)
  , mpSchoolLevelValueLabel(NULL)
  , mpSchoolSubjectsItemLabel(NULL)
  , mpSchoolSubjectsBox(NULL)
  , mpSchoolSubjectsValueLabel(NULL)
  , mpSchoolTypeItemLabel(NULL)
  , mpSchoolTypeBox(NULL)
  , mpSchoolTypeValueLabel(NULL)
  , mpSeparatorIndex(NULL)
  , mpLicenceLabel(NULL)
  , mpLicenceBox( NULL)
  , mpLicenceIcon(NULL)
  , mpLicenceLayout(NULL)
  , mpSceneItemSessionTitle(NULL)
  , mCurrentDocument(NULL)
  // Issue 1683 (Evolution) - AOU - 20131206
  , mpSeparatorFiles(NULL)
  , mpTreeWidgetEdition(NULL)
  , mpTreeWidgetPresentation(NULL)
  , mMode(tUBTGZeroPageMode_EDITION)
  , mbFilesChanged(false)
  // Fin Issue 1683 (Evolution) - AOU - 20131206
  , mIsModified(false)
{
    setObjectName(name);
    QString chapterStyle("QLabel {font-size:16px; font-weight:bold;}");
    mpLayout = new QVBoxLayout(0);

    setLayout(mpLayout);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");
    mpPageNumberLabel->setText(tr("Title page"));
    mpLayout->addWidget(mpPageNumberLabel);

    mpScrollArea = new QScrollArea();
    mpScrollArea->setFocusPolicy(Qt::NoFocus);
    mpContainerWidget = new QWidget();
    mpContainerWidget->setFocusPolicy(Qt::NoFocus);
    mpContainerWidgetLayout = new QVBoxLayout();

    mpLayout->addWidget(mpScrollArea);
    mpScrollArea->setWidget(mpContainerWidget);
    mpScrollArea->setWidgetResizable(true);
    mpScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpContainerWidget->setLayout(mpContainerWidgetLayout);

    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/teacherGuide/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);
    mpModePushButton->installEventFilter(this);
    mpButtonTitleLayout->addWidget(mpModePushButton);
    connect(mpModePushButton, SIGNAL(clicked()), this, SLOT(switchToMode()));

    mpSessionTitle = new UBTGAdaptableText(0, this, "UBTGSessionTitle");
    mpSessionTitle->setPlaceHolderText(tr("Type session title here ..."));
    mpSessionTitle->setMaximumLength(1000);
    mpSessionTitle->setTabChangesFocus(true);
    mpButtonTitleLayout->addWidget(mpSessionTitle);
    connect(this, SIGNAL(resized()), mpSessionTitle, SLOT(onTextChanged()));

    mpContainerWidgetLayout->addLayout(mpButtonTitleLayout);

    mpSeparatorSessionTitle = new QFrame(this);
    mpSeparatorSessionTitle->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorSessionTitle->setObjectName("UBTGSeparator");
    mpContainerWidgetLayout->addWidget(mpSeparatorSessionTitle);

    mpAuthorsLabel = new QLabel(this);
    mpAuthorsLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpAuthorsLabel->setText(tr("Author(s)"));
    mpAuthorsLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpAuthorsLabel);

    mpAuthors = new UBTGAdaptableText(0, this);
    mpAuthors->setObjectName("UBTGZeroPageInputText");
    mpAuthors->setPlaceHolderText(tr("Type authors here ..."));
    mpAuthors->setTabChangesFocus(true);
    mpContainerWidgetLayout->addWidget(mpAuthors);
    connect(this, SIGNAL(resized()), mpAuthors, SLOT(onTextChanged()));

    mpCreationLabel = new QLabel(this);
    mpCreationLabel->setObjectName("UBTGZeroPageDateLabel");
    mpContainerWidgetLayout->addWidget(mpCreationLabel);

    mpLastModifiedLabel = new QLabel(this);
    mpLastModifiedLabel->setObjectName("UBTGZeroPageDateLabel");
    mpContainerWidgetLayout->addWidget(mpLastModifiedLabel);

    mpSeparatorAuthors = new QFrame(this);
    mpSeparatorAuthors->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorAuthors->setObjectName("UBTGSeparator");
    mpContainerWidgetLayout->addWidget(mpSeparatorAuthors);

    mpObjectivesLabel = new QLabel(this);
    mpObjectivesLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpObjectivesLabel->setText(tr("Objective(s)"));
    mpObjectivesLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpObjectivesLabel);

    mpObjectives = new UBTGAdaptableText(0, this);
    mpObjectives->setObjectName("UBTGZeroPageInputText");
    mpObjectives->setPlaceHolderText(tr("Type objectives here..."));
    mpObjectives->setTabChangesFocus(true);
    mpContainerWidgetLayout->addWidget(mpObjectives);
    connect(this, SIGNAL(resized()), mpObjectives, SLOT(onTextChanged()));

    mpSeparatorObjectives = new QFrame(this);
    mpSeparatorObjectives->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorObjectives->setObjectName("UBTGSeparator");
    mpContainerWidgetLayout->addWidget(mpSeparatorObjectives);

    mpIndexLabel = new QLabel(this);
    mpIndexLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpIndexLabel->setText(tr("Resource indexing"));
    mpIndexLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpIndexLabel);

    mpKeywordsLabel = new QLabel(this);
    mpKeywordsLabel->setObjectName("UBTGZeroPageItemLabel");
    mpKeywordsLabel->setText(tr("Keywords:"));
    mpKeywordsLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpKeywordsLabel);
    mpKeywords = new UBTGAdaptableText(0, this);
    mpKeywords->setPlaceHolderText(tr("Type keywords here ..."));
    mpKeywords->setTabChangesFocus(true);
    mpContainerWidgetLayout->addWidget(mpKeywords);
    connect(this, SIGNAL(resized()), mpKeywords, SLOT(onTextChanged()));

    mpSchoolLevelItemLabel = new QLabel(this);
    mpSchoolLevelItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolLevelItemLabel->setText(tr("Level:"));
    mpSchoolLevelItemLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpSchoolLevelItemLabel);
    mpSchoolLevelBox = new QComboBox(this);
    mpSchoolLevelBox->setMinimumHeight(22);
    mpSchoolLevelBox->setMinimumWidth(LOWER_RESIZE_WIDTH);
    mpSchoolLevelBox->setObjectName("DockPaletteWidgetComboBox");
    connect(mpSchoolLevelBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onSchoolLevelChanged(QString)));
    mpContainerWidgetLayout->addWidget(mpSchoolLevelBox);
    mpSchoolLevelValueLabel = new QLabel(this);
    mpContainerWidgetLayout->addWidget(mpSchoolLevelValueLabel);

    mpSchoolSubjectsItemLabel = new QLabel(this);
    mpSchoolSubjectsItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolSubjectsItemLabel->setText(tr("Subjects:"));
    mpSchoolSubjectsItemLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpSchoolSubjectsItemLabel);
    mpSchoolSubjectsBox = new QComboBox(this);
    mpSchoolSubjectsBox->setMinimumHeight(22);
    mpSchoolSubjectsBox->setMinimumWidth(LOWER_RESIZE_WIDTH);
    mpSchoolSubjectsBox->setObjectName("DockPaletteWidgetComboBox");
    mpContainerWidgetLayout->addWidget(mpSchoolSubjectsBox);
    mpSchoolSubjectsValueLabel = new QLabel(this);
    mpContainerWidgetLayout->addWidget(mpSchoolSubjectsValueLabel);

    mpSchoolTypeItemLabel = new QLabel(this);
    mpSchoolTypeItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolTypeItemLabel->setText(tr("Type:"));
    mpSchoolTypeItemLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpSchoolTypeItemLabel);
    mpSchoolTypeBox = new QComboBox(this);
    mpSchoolTypeBox->setMinimumHeight(22);
    mpSchoolTypeBox->setMinimumWidth(LOWER_RESIZE_WIDTH);
    mpSchoolTypeBox->setObjectName("DockPaletteWidgetComboBox");
    mpContainerWidgetLayout->addWidget(mpSchoolTypeBox);
    mpSchoolTypeValueLabel = new QLabel(this);
    mpContainerWidgetLayout->addWidget(mpSchoolTypeValueLabel);

    mpSeparatorIndex = new QFrame(this);
    mpSeparatorIndex->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorIndex->setObjectName("UBTGSeparator");
    mpContainerWidgetLayout->addWidget(mpSeparatorIndex);

    mpLicenceLabel = new QLabel(this);
    mpLicenceLabel->setObjectName("UBTGZeroPageItemLabel");
    mpLicenceLabel->setText(tr("Licence"));
    mpLicenceLabel->setStyleSheet(chapterStyle);
    mpContainerWidgetLayout->addWidget(mpLicenceLabel);
    mpLicenceBox = new QComboBox(this);
    mpLicenceBox->setMinimumHeight(22);
    mpLicenceBox->setMinimumWidth(LOWER_RESIZE_WIDTH);
    mpLicenceBox->setObjectName("DockPaletteWidgetComboBox");
    mpContainerWidgetLayout->addWidget(mpLicenceBox);
    mpLicenceLayout = new QVBoxLayout(0); // Issue 1517 - ALTI/AOU - 20131206 : change le layout Horizontal en Vertical.

    //issue 1517 - NNE - 20131206
    mpLicenceIcon = new UBClickableLabel(this);
    mpLicenceIcon->setMinimumWidth(LOWER_RESIZE_WIDTH/2);
    mpLicenceLayout->addWidget(mpLicenceIcon);

    //issue 1517 - NNE - 20131206
    connect(mpLicenceIcon, SIGNAL(clicked()), this, SLOT(onClickLicence()));

    mpLicenceValueLabel = new QLabel(this);
    mpLicenceValueLabel->setMinimumWidth(LOWER_RESIZE_WIDTH/2);
    mpLicenceLayout->addWidget(mpLicenceValueLabel);
    mpContainerWidgetLayout->addLayout(mpLicenceLayout);


    // Les QTreeWidget qui permettent d'embarquer des fichiers externes dans le Document : // Issue 1683 (Evolution) - AOU - 20131206

    mpSeparatorFiles = new QFrame(this);
    mpSeparatorFiles->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorFiles->setObjectName("UBTGSeparator");
    mpContainerWidgetLayout->addWidget(mpSeparatorFiles);

    // QTreeView visible en Mode Edition :
    mpTreeWidgetEdition = new QTreeWidget(this);
    mpTreeWidgetEdition->setStyleSheet("selection-background-color:transparent; padding-bottom:5px; padding-top:5px;");
    mpContainerWidgetLayout->addWidget(mpTreeWidgetEdition);

    QTreeWidgetItem* mpRootWidgetItem = mpTreeWidgetEdition->invisibleRootItem();
    mpTreeWidgetEdition->setRootIsDecorated(false);
    mpTreeWidgetEdition->setIndentation(0);
    mpTreeWidgetEdition->setDropIndicatorShown(false);
    mpTreeWidgetEdition->header()->close();
    mpTreeWidgetEdition->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpTreeWidgetEdition->setColumnCount(2);
    mpTreeWidgetEdition->header()->setStretchLastSection(false);
    mpTreeWidgetEdition->header()->setResizeMode(0, QHeaderView::Stretch);
    mpTreeWidgetEdition->header()->setResizeMode(1, QHeaderView::Fixed);
    mpTreeWidgetEdition->header()->setDefaultSectionSize(18);
    mpTreeWidgetEdition->setSelectionMode(QAbstractItemView::NoSelection);

	// Issue 1683 - AOU - 20131219 : amélioration présentation du Tree dans ScrollArea, pour gérer les petits écrans.
    mpTreeWidgetEdition->setExpandsOnDoubleClick(false);
    mpTreeWidgetEdition->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(mpScrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onScrollAreaRangeChanged(int, int)));
	// Fin Issue 1683 - AOU - 20131219

    connect(mpTreeWidgetEdition, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onAddItemClicked(QTreeWidgetItem*,int)));

    mpAddAFileItem = new UBAddItem(tr("Add a file"), eUBTGAddSubItemWidgetType_File, mpTreeWidgetEdition);
    mpRootWidgetItem->addChild(mpAddAFileItem);


    // QTreeView visible en Mode Presentation :
    mpTreeWidgetPresentation = new QTreeWidget(this);
    mpContainerWidgetLayout->addWidget(mpTreeWidgetPresentation);
    mpTreeWidgetPresentation->setDragEnabled(false);
    mpTreeWidgetPresentation->setRootIsDecorated(false);
    mpTreeWidgetPresentation->setIndentation(0);
    mpTreeWidgetPresentation->setDropIndicatorShown(false);
    mpTreeWidgetPresentation->header()->close();
    mpTreeWidgetPresentation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpTreeWidgetPresentation->setStyleSheet("selection-background-color:transparent; padding-bottom:5px; padding-top:5px; ");
    mpTreeWidgetPresentation->setIconSize(QSize(24,24));

	// Issue 1683 - AOU - 20131219 : amélioration présentation du Tree dans ScrollArea, pour gérer les petits écrans.
    mpTreeWidgetPresentation->setExpandsOnDoubleClick(false);
    mpTreeWidgetPresentation->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mpContainerWidgetLayout->addStretch();
	// Fin Issue 1683 - AOU - 20131219

    connect(mpTreeWidgetPresentation, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onAddItemClicked(QTreeWidgetItem*,int)));

    // Fin Issue 1683 (Evolution) - AOU - 20131206

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
    fillComboBoxes();

    if (UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()) {
        connect(UBApplication::boardController, SIGNAL(documentSet(UBDocumentProxy*)), this, SLOT(onActiveDocumentChanged()));
    }

    connect(mpSessionTitle, SIGNAL(cursorPositionChanged()), this, SLOT(setIsModified()));
    connect(mpAuthors, SIGNAL(cursorPositionChanged()), this, SLOT(setIsModified()));
    connect(mpObjectives, SIGNAL(cursorPositionChanged()), this, SLOT(setIsModified()));
    connect(mpKeywords, SIGNAL(cursorPositionChanged()), this, SLOT(setIsModified()));
    connect(mpSchoolLevelBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setIsModified()));
    connect(mpSchoolSubjectsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setIsModified()));
    connect(mpSchoolTypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setIsModified()));
    connect(mpLicenceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setIsModified()));

}

//issue 1517 - NNE - 20131206
void UBTeacherGuidePageZeroWidget::onClickLicence()
{
    UBApplication::webController->loadUrl(QUrl("http://creativecommons.org"));
}
//issue 1517 - NNE - 20131206

UBTeacherGuidePageZeroWidget::~UBTeacherGuidePageZeroWidget()
{
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpSessionTitle);
    DELETEPTR(mpSeparatorSessionTitle);
    DELETEPTR(mpAuthorsLabel);
    DELETEPTR(mpAuthors);
    DELETEPTR(mpSeparatorAuthors);
    DELETEPTR(mpCreationLabel);
    DELETEPTR(mpLastModifiedLabel);
    DELETEPTR(mpObjectivesLabel);
    DELETEPTR(mpObjectives);
    DELETEPTR(mpSeparatorObjectives);
    DELETEPTR(mpIndexLabel);
    DELETEPTR(mpKeywordsLabel);
    DELETEPTR(mpKeywords);
    DELETEPTR(mpSchoolLevelItemLabel);
    DELETEPTR(mpSchoolLevelBox);
    DELETEPTR(mpSchoolLevelValueLabel);
    DELETEPTR(mpSchoolSubjectsItemLabel);
    DELETEPTR(mpSchoolSubjectsBox);
    DELETEPTR(mpSchoolSubjectsValueLabel);
    DELETEPTR(mpSchoolTypeItemLabel);
    DELETEPTR(mpSchoolTypeBox);
    DELETEPTR(mpSchoolTypeValueLabel);
    DELETEPTR(mpSeparatorIndex);
    DELETEPTR(mpLicenceLabel);
    DELETEPTR(mpLicenceBox);
    DELETEPTR(mpLicenceValueLabel);
    DELETEPTR(mpLicenceIcon);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpLicenceLayout);
    DELETEPTR(mpButtonTitleLayout);
    // Issue 1683 (Evolution) - AOU - 20131206
    DELETEPTR(mpSeparatorFiles);
    DELETEPTR(mpMediaSwitchItem);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpAddAFileItem);
    DELETEPTR(mpTreeWidgetEdition);
    DELETEPTR(mpTreeWidgetPresentation);
    // Fin Issue 1683 (Evolution) - AOU - 20131206
    DELETEPTR(mpContainerWidgetLayout);
    DELETEPTR(mpContainerWidget);
    DELETEPTR(mpScrollArea);
    DELETEPTR(mpLayout);
}

bool UBTeacherGuidePageZeroWidget::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);
    if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
        return true;
    return false;
}

void UBTeacherGuidePageZeroWidget::fillComboBoxes()
{
    QString parametersConfigFilePath = UBSettings::settings()->applicationCustomizationDirectory() + "/teacherGuide/indexingParameters.xml";
    QFile parametersFile(parametersConfigFilePath);
    if (!parametersFile.exists()) {
        qCritical() << "UBTeacherGuidePageZeroEditionWidget fillComboBoxes file not found " << parametersConfigFilePath;
        return;
    }

    parametersFile.open(QFile::ReadOnly);

    QDomDocument doc;
    doc.setContent(parametersFile.readAll());
    QDomElement rootElement = doc.elementsByTagName("teacherGuide").at(0).toElement();

    QDomNodeList subjects = rootElement.elementsByTagName("subjects");
    for (int baseLevelCounter = 0; baseLevelCounter < subjects.count(); baseLevelCounter += 1) {
        QDomNode subjectsForBaseLevel = subjects.at(baseLevelCounter);
        QDomNodeList subjectsList = subjectsForBaseLevel.childNodes();
        QStringList subjectsRelatedToBaseLevel;
        for (int j = 0; j < subjectsList.count(); j += 1) {
            subjectsRelatedToBaseLevel.append(subjectsList.at(j).toElement().attribute("label"));
        }
        mSubjects.insert( subjectsForBaseLevel.toElement().attribute("baseLevel"), subjectsRelatedToBaseLevel);
    }

    QDomNodeList gradeLevels = rootElement.elementsByTagName("gradeLevels").at(0).childNodes();
    for (int i = 0; i < gradeLevels.count(); i += 1) {
        mGradeLevelsMap.insert(gradeLevels.at(i).toElement().attribute("label"), gradeLevels.at(i).toElement().attribute("baseLevel"));
        mpSchoolLevelBox->addItem( gradeLevels.at(i).toElement().attribute("label"));
    }

    QDomNodeList types = rootElement.elementsByTagName("types").at(0).childNodes();
    for (int i = 0; i < types.count(); i += 1)
        mpSchoolTypeBox->addItem(types.at(i).toElement().attribute("label"));

    parametersFile.close();

    QStringList licences;
    licences << tr("Attribution-ShareAlike CC BY-SA")
             << tr("Attribution CC BY")
             << tr("Attribution-NoDerivs CC BY-ND")
             << tr("Attribution-NonCommercial CC BY-NC")
             << tr("Attribution-NonCommercial-NoDerivs CC BY-NC-ND")
             << tr("Attribution-NonCommercial-ShareAlike CC BY-NC-SA")
             << tr("Public domain")
             << tr("Copyright");
    mpLicenceBox->addItems(licences);
    QStringList licenceIconList;
    licenceIconList << ":images/licenses/ccbysa.png"
                    << ":images/licenses/ccby.png"
                    << ":images/licenses/ccbynd.png"
                    << ":images/licenses/ccbync.png"
                    << ":images/licenses/ccbyncnd.png"
                    << ":images/licenses/ccbyncsa.png";
    for (int i = 0; i < licenceIconList.count(); i += 1)
        mpLicenceBox->setItemData(i, licenceIconList.at(i));
}

void UBTeacherGuidePageZeroWidget::onSchoolLevelChanged(QString schoolLevel)
{
    QStringList subjects = mSubjects.value(mGradeLevelsMap.value(schoolLevel));
    mpSchoolSubjectsBox->clear();
    if (subjects.count()) {
        mpSchoolSubjectsItemLabel->setEnabled(true);
        mpSchoolSubjectsBox->setEnabled(true);
        mpSchoolSubjectsBox->addItems(subjects);
    }
    else {
        mpSchoolSubjectsItemLabel->setDisabled(true);
        mpSchoolSubjectsBox->setDisabled(true);
    }
}

void UBTeacherGuidePageZeroWidget::onActiveSceneChanged()
{
    UBDocumentProxy* documentProxy = UBApplication::boardController->selectedDocument();
    if (documentProxy && UBApplication::boardController->currentPage() == 0) {
        QDateTime creationDate = documentProxy->documentDate();
        mpCreationLabel->setText( tr("Created the:\n") + creationDate.toString(Qt::DefaultLocaleShortDate));
        QDateTime updatedDate = documentProxy->lastUpdate();
        mpLastModifiedLabel->setText( tr("Updated the:\n") + updatedDate.toString(Qt::DefaultLocaleShortDate));
        loadData();
        updateSceneTitle();
    }
    load(UBSvgSubsetAdaptor::readTeacherGuideNode(0)); // Issue 1683 (Evolution) - AOU - 20131206

    //Issue 1454 - CFA - 20140306 : correction des raccourcis clavier fleches gauche et droite
    UBApplication::boardController->controlView()->setFocus();
}



void UBTeacherGuidePageZeroWidget::hideEvent(QHideEvent * event)
{
    persistData();
    QWidget::hideEvent(event);
}

void UBTeacherGuidePageZeroWidget::loadData()
{
    //UBDocumentProxy* documentProxy = UBApplication::boardController->selectedDocument();
    mCurrentDocument = UBApplication::boardController->selectedDocument();
    UBDocumentProxy* documentProxy = mCurrentDocument;
    mpSessionTitle->setText( documentProxy->metaData(UBSettings::sessionTitle).toString());
    mpAuthors->setText( documentProxy->metaData(UBSettings::sessionAuthors).toString());
    mpObjectives->setText( documentProxy->metaData(UBSettings::sessionObjectives).toString());
    mpKeywords->setText( documentProxy->metaData(UBSettings::sessionKeywords).toString());

    int currentIndex = mpSchoolLevelBox->findText(documentProxy->metaData(UBSettings::sessionGradeLevel).toString());
    mpSchoolLevelBox->setCurrentIndex((currentIndex != -1) ? currentIndex : 0);

    currentIndex = mpSchoolSubjectsBox->findText(documentProxy->metaData(UBSettings::sessionSubjects).toString());
    mpSchoolSubjectsBox->setCurrentIndex((currentIndex != -1) ? currentIndex : 0);

    currentIndex = mpSchoolTypeBox->findText(documentProxy->metaData(UBSettings::sessionType).toString());
    mpSchoolTypeBox->setCurrentIndex((currentIndex != -1) ? currentIndex : 0);

    currentIndex = documentProxy->metaData(UBSettings::sessionLicence).toInt();
    mpLicenceBox->setCurrentIndex((currentIndex != -1) ? currentIndex : 0);

    setIsModified(false);
}



void UBTeacherGuidePageZeroWidget::persistData()
{
    // check necessary because at document closing hide event is send after boardcontroller set
    // to NULL
    if (UBApplication::boardController) {
        UBDocumentProxy* documentProxy = UBApplication::boardController->selectedDocument();

        if(mCurrentDocument != documentProxy){
            qDebug() << "this should never happens";
            return;
        }

        documentProxy->setMetaData(UBSettings::sessionTitle, mpSessionTitle->text());
        documentProxy->setMetaData(UBSettings::sessionAuthors, mpAuthors->text());
        documentProxy->setMetaData(UBSettings::sessionObjectives, mpObjectives->text());
        documentProxy->setMetaData(UBSettings::sessionKeywords, mpKeywords->text());
        documentProxy->setMetaData(UBSettings::sessionGradeLevel, mpSchoolLevelBox->currentText());
        documentProxy->setMetaData(UBSettings::sessionSubjects, mpSchoolSubjectsBox->currentText());
        documentProxy->setMetaData(UBSettings::sessionType, mpSchoolTypeBox->currentText());
        documentProxy->setMetaData(UBSettings::sessionLicence, mpLicenceBox->currentIndex());
        //Issue 1683 (Evolution) - CFA - 20140124 : a la modifications des infos de la teacher guide apres reimport de l'ubz, les external files (DocumentProxy) n'etaient pas recharges.
        //A la persistence dans subset adaptor, on perdait les fichiers dans le svg a la reecriture
        if(mpAddAFileItem->childCount() > 0)
            setFilesChanged();
    }
}

void UBTeacherGuidePageZeroWidget::updateSceneTitle()
{
    QString sessionTitle = mpSessionTitle->text();
    if (!sessionTitle.isEmpty())
        UBApplication::boardController->activeScene()->textForObjectName(mpSessionTitle->text());
}

void UBTeacherGuidePageZeroWidget::switchToMode(tUBTGZeroPageMode mode)
{
    setMode(mode); // Issue 1683 (Evolution) - AOU - 20131206

    if (mode == tUBTGZeroPageMode_EDITION) {
        QString inputStyleSheet("QTextEdit { background: white; border-radius: 10px; border: 2px;}");
        mpModePushButton->hide();
        mpSessionTitle->setReadOnly(false);
        mpSessionTitle->managePlaceholder(true);
        mpSessionTitle->setStyleSheet(inputStyleSheet);
        QFont titleFont(QApplication::font().family(), 11, -1);
        mpSessionTitle->document()->setDefaultFont(titleFont);
        mpAuthors->setReadOnly(false);
        mpAuthors->managePlaceholder(false);
        mpAuthors->setStyleSheet(inputStyleSheet);
        mpObjectives->setReadOnly(false);
        mpObjectives->managePlaceholder(false);
        mpObjectives->setStyleSheet(inputStyleSheet);
        mpKeywords->setReadOnly(false);
        mpKeywords->managePlaceholder(false);
        mpKeywords->setStyleSheet(inputStyleSheet);
        mpSchoolLevelValueLabel->hide();
        mpSchoolLevelBox->show();
        mpSchoolSubjectsValueLabel->hide();
        mpSchoolSubjectsBox->show();
        mpSchoolTypeValueLabel->hide();
        mpSchoolTypeBox->show();
        mpLicenceIcon->hide();
        mpLicenceValueLabel->hide();
        mpLicenceBox->show();

        // Issue 1683 (Evolution) - AOU - 20131206
        mpTreeWidgetEdition->show();
        mpTreeWidgetPresentation->hide();

        // Redim tree :
        int treeTotalHeight = UBWidgetUtils::getTreeWidgetItemVisualHeight(mpTreeWidgetEdition, mpAddAFileItem);
        treeTotalHeight += 2 * mpTreeWidgetEdition->frameWidth(); // add thickness of outline (top and bottom borders)
        mpTreeWidgetEdition->setFixedHeight(treeTotalHeight);
        // Fin Issue 1683 (Evolution) - AOU - 20131206
    }
    else {
        QString inputStyleSheet( "QTextEdit { background: transparent; border: none;}");
        mpModePushButton->show();
        mpSessionTitle->showText(mpSessionTitle->text());
        mpSessionTitle->setStyleSheet(inputStyleSheet);
        updateSceneTitle();
        QFont titleFont(QApplication::font().family(), 14, 1);
        mpSessionTitle->document()->setDefaultFont(titleFont);
        mpAuthors->setStyleSheet(inputStyleSheet);
        mpAuthors->setTextColor(QColor(Qt::black));
        mpAuthors->showText(mpAuthors->text());
        mpObjectives->setStyleSheet(inputStyleSheet);
        mpObjectives->setTextColor(QColor(Qt::black));
        mpObjectives->showText(mpObjectives->text());
        mpKeywords->setStyleSheet(inputStyleSheet);
        mpKeywords->setTextColor(QColor(Qt::black));
        mpKeywords->showText(mpKeywords->text());
        mpSchoolLevelValueLabel->setText(mpSchoolLevelBox->currentText());
        mpSchoolLevelValueLabel->show();
        mpSchoolLevelBox->hide();
        mpSchoolSubjectsValueLabel->setText(mpSchoolSubjectsBox->currentText());
        mpSchoolSubjectsValueLabel->show();
        mpSchoolSubjectsBox->hide();
        mpSchoolTypeValueLabel->setText(mpSchoolTypeBox->currentText());
        mpSchoolTypeValueLabel->show();
        mpSchoolTypeBox->hide();
        mpLicenceValueLabel->setText(mpLicenceBox->currentText());
        QString licenceIconPath = mpLicenceBox->itemData(mpLicenceBox->currentIndex()).toString();
        if (!licenceIconPath.isEmpty()) {
            mpLicenceIcon->setPixmap(QPixmap(licenceIconPath));
            mpLicenceIcon->show();
        }
        mpLicenceValueLabel->show();
        mpLicenceBox->hide();
        persistData();

        // Issue 1683 (Evolution) - AOU - 20131206
        mpTreeWidgetEdition->hide();

        // Remove empty ExternalFiles from treeEdition :
        for(int i=0; i<mpAddAFileItem->childCount();)
        {
            QTreeWidgetItem* item = mpAddAFileItem->child(i);
            UBTGFileWidget* fileItem = dynamic_cast<UBTGFileWidget*>(mpTreeWidgetEdition->itemWidget(item, 0));
            if (fileItem && fileItem->titreFichier().trimmed().isEmpty() && fileItem->path().isEmpty()){ // if no title nor file choosen ...
                QTreeWidgetItem * itemtoBeDeleted = mpAddAFileItem->takeChild(i); // remove item from tree...
                DELETEPTR(itemtoBeDeleted); // and destroy item.
            }
            else{
                i++;
            }
        }

        // Rafraichir le QTreeWidget "Presentation" avec les items du QTreeWidget "Edition"
        cleanData(tUBTGZeroPageMode_PRESENTATION);
        for(int i=0; i<mpAddAFileItem->childCount(); ++i)
        {
            QTreeWidgetItem* item = mpAddAFileItem->child(i);
            UBTGFileWidget* fileItem = dynamic_cast<UBTGFileWidget*>(mpTreeWidgetEdition->itemWidget(item, 0));
            if (fileItem)
            {
                createMediaButtonItem();
                QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
                if (fileItem->path().isEmpty()){
                    newWidgetItem->setIcon(0, QIcon(":images/teacherGuide/document_large_warning.gif")); // different icon, if no file choosed.
                }
                else{
                    newWidgetItem->setIcon(0, QIcon(":images/teacherGuide/document_large.gif"));
                }
                newWidgetItem->setText(0, fileItem->titreFichier());
                newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_FILE);
                newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnUrl, QVariant(fileItem->path()));
                newWidgetItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 11)));
                newWidgetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                mpTreeWidgetPresentation->invisibleRootItem()->addChild(newWidgetItem);
            }
        }
        mpTreeWidgetPresentation->show();

        // Redim tree :
        int treeTotalHeight = UBWidgetUtils::getTreeWidgetItemVisualHeight(mpTreeWidgetPresentation, mpMediaSwitchItem);
        treeTotalHeight += 2 * mpTreeWidgetPresentation->frameWidth(); // add thickness of outline (top and bottom borders)
        mpTreeWidgetPresentation->setFixedHeight(treeTotalHeight);

        // Fin Issue 1683 (Evolution) - AOU - 20131206
    }
    update();
}

QVector<tUBGEElementNode*> UBTeacherGuidePageZeroWidget::getData()
{
    QVector<tUBGEElementNode*> result;
    tUBGEElementNode* elementNode = new tUBGEElementNode();
    elementNode->name = "sessionTitle";
    elementNode->attributes.insert("value", mpSessionTitle->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "authors";
    elementNode->attributes.insert("value", mpAuthors->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "creationDate";
    elementNode->attributes.insert("value", mpCreationLabel->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "lastModifiedDate";
    elementNode->attributes.insert("value", mpLastModifiedLabel->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "goals";
    elementNode->attributes.insert("value", mpObjectives->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "keywords";
    elementNode->attributes.insert("value", mpKeywords->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "schoolLevel";
    elementNode->attributes.insert("value", mpSchoolLevelBox->currentText());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "schoolBranch";
    elementNode->attributes.insert("value", mpSchoolSubjectsBox->currentText());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "schoolType";
    elementNode->attributes.insert("value", mpSchoolTypeBox->currentText());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->name = "licence";
    elementNode->attributes.insert("value", QString("%1").arg(mpLicenceBox->currentIndex()));
    result << elementNode;
    return result;
}

bool UBTeacherGuidePageZeroWidget::isModified()
{
    return mIsModified;
}

bool UBTeacherGuidePageZeroWidget::hasUserDataInTeacherGuide()
{
    bool result = false;
    UBDocumentProxy* documentProxy = UBApplication::boardController->selectedDocument();
    if(mCurrentDocument == documentProxy){
        result |= mpSessionTitle->text().length() > 0;
        result |= mpAuthors->text().length() > 0;
        result |= mpObjectives->text().length() > 0;
        result |= mpKeywords->text().length() > 0;
        result |= mpSchoolLevelBox->currentIndex() > 0;
        result |= mpSchoolSubjectsBox->currentIndex() > 0;
        result |= mpSchoolTypeBox->currentIndex() > 0;
        result |= mpLicenceBox->currentIndex() > 0;
        result |= filesChanged(); // Issue 1683 (Evolution) - AOU - 20131206
    }
    else
        qDebug() << "this should not happen";

    return result;
}

void UBTeacherGuidePageZeroWidget::resizeEvent(QResizeEvent* ev)
{
    emit resized();
    QWidget::resizeEvent(ev);
}

// Issue 1683 (Evolution) - AOU - 20131206
void UBTeacherGuidePageZeroWidget::load(QDomDocument doc)
{
    setMode(tUBTGZeroPageMode_EDITION);
    cleanData(tUBTGZeroPageMode_EDITION);
    for (QDomElement element = doc.documentElement().firstChildElement();
         !element.isNull(); element = element.nextSiblingElement()) {
        QString tagName = element.tagName();
        if (tagName == "file")
            onAddItemClicked(mpAddAFileItem, 0, &element);
    }
    mbFilesChanged = false;
    setIsModified(false);
}


void UBTeacherGuidePageZeroWidget::onAddItemClicked(QTreeWidgetItem* widget, int column, QDomElement *element)
{
    setIsModified();

    int addSubItemWidgetType = widget->data(column, Qt::UserRole).toInt();

    if (mode() == tUBTGZeroPageMode_EDITION)
    {
        if (addSubItemWidgetType != eUBTGAddSubItemWidgetType_None) {
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(widget);
            newWidgetItem->setData(column, Qt::UserRole, eUBTGAddSubItemWidgetType_None);
            newWidgetItem->setData(1, Qt::UserRole, eUBTGAddSubItemWidgetType_None);
            newWidgetItem->setIcon(1, QIcon(":images/close.svg"));

            switch (addSubItemWidgetType) {
            case eUBTGAddSubItemWidgetType_File: {
                UBTGFileWidget* fileWidget = new UBTGFileWidget(newWidgetItem);
                if (element)
                    fileWidget->initializeWithDom(*element);
                mpTreeWidgetEdition->setItemWidget(newWidgetItem, 0, fileWidget);
                connect(fileWidget, SIGNAL(changed()), this, SLOT(setFilesChanged()));

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
        else if (column == 1 && addSubItemWidgetType == eUBTGAddSubItemWidgetType_None)
        {
            UBTGFileWidget * fileWidget = dynamic_cast<UBTGFileWidget*>(mpTreeWidgetEdition->itemWidget(widget, 0));
            if (fileWidget)
            {
                // Supprimer le fichier embarqué (si il est bien dans un chemin relatif au dossier Document)
                QString pathFile = fileWidget->path();
                if (pathFile.left(pathFile.indexOf('/')) == UBPersistenceManager::teacherGuideDirectory)
                {
                    QFile file(UBApplication::boardController->selectedDocument()->persistencePath() + "/" + pathFile);
                    QFileInfo fi(file);
                    file.remove();  // supprimer le fichier
                    QDir parentDir = fi.dir();
                    parentDir.rmpath(parentDir.absolutePath()); // supprimer le repertoire
                }
            }

            delete widget;

            setFilesChanged();
        }

        // Redim TreeViewEdition :
        int treeTotalHeight = UBWidgetUtils::getTreeWidgetItemVisualHeight(mpTreeWidgetEdition, mpAddAFileItem);
        treeTotalHeight += (2 * mpTreeWidgetEdition->frameWidth()); // add thickness of outline (top and bottom borders)
        mpTreeWidgetEdition->setFixedHeight(treeTotalHeight);
    }
    else if (mode() == tUBTGZeroPageMode_PRESENTATION)
    {
        switch (addSubItemWidgetType) {
        case tUBTGActionAssociateOnClickItem_EXPAND: {
            widget->setExpanded(!widget->isExpanded());
            if (widget->isExpanded()){
#ifdef Q_WS_MAC
                for(int i = 0 ; i < mpMediaSwitchItem->childCount(); i+=1 ){
                    QTreeWidgetItem* eachItem = mpMediaSwitchItem->child(i);
                    eachItem->setHidden(false);
                }
#endif
                mpMediaSwitchItem->setText(0, "-");
            }
            else
            {
#ifdef Q_WS_MAC
                for(int i = 0 ; i < mpMediaSwitchItem->childCount(); i+=1 ){
                    QTreeWidgetItem* eachItem = mpMediaSwitchItem->child(i);
                    eachItem->setHidden(true);
                }
#endif
                mpMediaSwitchItem->setText(0, "+");
            }

            // Redim tree :
            int treeTotalHeight = UBWidgetUtils::getTreeWidgetItemVisualHeight(mpTreeWidgetPresentation, mpMediaSwitchItem);
            treeTotalHeight += 2 * mpTreeWidgetPresentation->frameWidth(); // add thickness of outline (top and bottom borders)
            mpTreeWidgetPresentation->setFixedHeight(treeTotalHeight);

            break;
        }
        case tUBTGActionAssociateOnClickItem_FILE: {
            widget->data(column, tUBTGTreeWidgetItemRole_HasAnUrl).toString();

            QString pathFile = widget->data(column, tUBTGTreeWidgetItemRole_HasAnUrl).toString();
            QString pathDocument = UBApplication::boardController->selectedDocument()->persistencePath();
            QString fullPathFile = pathDocument + "/" + pathFile;

            if ( pathFile.isEmpty() || ! QFile::exists(fullPathFile) ) // Si le fichier n'existe pas, on prévient l'utilisateur
            {
                UBApplication::showMessage(tr("File not found"));
            }
            else
            {
                QCursor oldCursor = cursor();
                setCursor(Qt::WaitCursor);
                if ( ! QDesktopServices::openUrl(QUrl("file:///" + fullPathFile)) ){
                    UBApplication::showMessage(tr("No application was found to handle this file type"));
                }
                setCursor(oldCursor);
            }
            break;
        }
        default:
            qCritical() << "onAddItemClicked no action set";
            return;
        }
    }
}

void UBTeacherGuidePageZeroWidget::setFilesChanged()
{
    // Issue 1683 - ALTI/AOU - 20131212
    // Dans cette fonction, on reconstruit les DocumentProxy.externalFiles, à partir des items du treeViewEdition
    UBDocumentProxy* pDoc = UBApplication::boardController->selectedDocument();
    pDoc->externalFilesClear();

    QList<QTreeWidgetItem*> children;
    for (int i = 0; i < mpAddAFileItem->childCount(); i += 1)
        children << mpAddAFileItem->child(i);

    foreach(QTreeWidgetItem* widgetItem, children) {

        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidgetEdition->itemWidget( widgetItem, 0))->saveData();
        if (node) {
            UBDocumentExternalFile* externalFile = new UBDocumentExternalFile();
            externalFile->setPath(node->attributes.value("path"));
            externalFile->setTitle(node->attributes.value("title"));
            pDoc->externalFilesAdd(externalFile);
        }
    }
    // Fin Issue 1683 - ALTI/AOU - 20131212

    mbFilesChanged = true;
}


void UBTeacherGuidePageZeroWidget::createMediaButtonItem()
{
    if (!mpMediaSwitchItem) {
        mpMediaSwitchItem = new QTreeWidgetItem(mpTreeWidgetPresentation->invisibleRootItem());
        mpMediaSwitchItem->setText(0, "+");
        mpMediaSwitchItem->setExpanded(false);
        mpMediaSwitchItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_EXPAND);
        mpMediaSwitchItem->setData(0, Qt::BackgroundRole, QVariant(QColor(200, 200, 200)));
        mpMediaSwitchItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 16)));
        mpMediaSwitchItem->setData(0, Qt::TextAlignmentRole, QVariant(Qt::AlignCenter));
        mpTreeWidgetPresentation->invisibleRootItem()->addChild(mpMediaSwitchItem);
    }
}

void UBTeacherGuidePageZeroWidget::cleanData(tUBTGZeroPageMode mode)
{
    if (mode == tUBTGZeroPageMode_EDITION)
    {
        QList<QTreeWidgetItem*> itemToRemove = mpAddAFileItem->takeChildren();
        foreach(QTreeWidgetItem* eachItem, itemToRemove) {
            DELETEPTR(eachItem);
        }
    }
    else if (mode == tUBTGZeroPageMode_PRESENTATION)
    {
        QList<QTreeWidgetItem*> itemToRemove = mpTreeWidgetPresentation->invisibleRootItem()->takeChildren();
        foreach(QTreeWidgetItem* eachItem, itemToRemove) {
            DELETEPTR(eachItem);
        }
        // the mpMediaSwitchItem is deleted by the previous loop but the pointer is not set to zero
        mpMediaSwitchItem = NULL;
    }
}
// Fin Issue 1683 (Evolution) - AOU - 20131206

/***************************************************************************
 *                    class    UBTeacherGuideWidget                        *
 ***************************************************************************/
UBTeacherGuideWidget::UBTeacherGuideWidget(QWidget* parent, const char* name) :
    UBAbstractTeacherGuide(parent)
  , mpPageZeroWidget(NULL)
  , mpEditionWidget(NULL)
  , mpPresentationWidget(NULL)
{
    setObjectName(name);
    if (UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()) {
        mpPageZeroWidget = new UBTeacherGuidePageZeroWidget(this);
        addWidget(mpPageZeroWidget);
    }
    if (UBSettings::settings()->teacherGuideLessonPagesActivated->get().toBool()) {
        mpEditionWidget = new UBTeacherGuideEditionWidget(this);
        addWidget(mpEditionWidget);
        mpPresentationWidget = new UBTeacherGuidePresentationWidget(this);
        addWidget(mpPresentationWidget);
    }

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
}

UBTeacherGuideWidget::~UBTeacherGuideWidget()
{
    DELETEPTR(mpPageZeroWidget);
    DELETEPTR(mpEditionWidget);
    DELETEPTR(mpPresentationWidget);
}

void UBTeacherGuideWidget::onActiveSceneChanged()
{
    if (UBApplication::boardController->currentPage() == 0) {
        if(mpPageZeroWidget->hasUserDataInTeacherGuide())
            mpPageZeroWidget->switchToMode(tUBTGZeroPageMode_PRESENTATION);
        else
            mpPageZeroWidget->switchToMode(tUBTGZeroPageMode_EDITION);

        setCurrentWidget(mpPageZeroWidget);
    }
    else{
        if(mpEditionWidget->hasUserDataInTeacherGuide()){
            mCurrentData = mpEditionWidget->getData();
            mpPresentationWidget->showData(mCurrentData);
            setCurrentWidget(mpPresentationWidget);
        }
        else
            setCurrentWidget(mpEditionWidget);
    }

    //Issue 1454 - CFA - 20140306 : correction des raccourcis clavier fleches gauche et droite
    UBApplication::boardController->controlView()->setFocus();

}

void UBTeacherGuideWidget::showPresentationMode()
{
    if (currentWidget() == mpPageZeroWidget) {
        mCurrentData = mpPageZeroWidget->getData();
        mpPageZeroWidget->switchToMode(tUBTGZeroPageMode_PRESENTATION);
    }
    else if (currentWidget() == mpEditionWidget) {
        mCurrentData = mpEditionWidget->getData();
        mpPresentationWidget->showData(mCurrentData);
        setCurrentWidget(mpPresentationWidget);
    }
}

void UBTeacherGuideWidget::changeMode()
{
    if (currentWidget() == mpEditionWidget)
        setCurrentWidget(mpPresentationWidget);
    else
        setCurrentWidget(mpEditionWidget);

}

bool UBTeacherGuideWidget::isModified()
{
    if (currentWidget() == mpPageZeroWidget)
        return mpPageZeroWidget->isModified();
    else
        return mpEditionWidget->isModified();
}

bool UBTeacherGuideWidget::hasUserDataInTeacherGuide()
{
    if (currentWidget() == mpPageZeroWidget)
        return mpPageZeroWidget->hasUserDataInTeacherGuide();
    else
        return mpEditionWidget->hasUserDataInTeacherGuide();
}

void UBTeacherGuidePageZeroWidget::onActiveDocumentChanged()
{
    int activeSceneIndex = UBApplication::boardController->activeSceneIndex();
    if (UBApplication::boardController->pageFromSceneIndex(activeSceneIndex) == 0)
        load(UBSvgSubsetAdaptor::readTeacherGuideNode(activeSceneIndex));
}

void UBTeacherGuidePageZeroWidget::onScrollAreaRangeChanged(int min, int max) // Issue 1683 - AOU - 20131219 : amélioration présentation du Tree dans ScrollArea, pour gérer les petits écrans.
{
    Q_UNUSED(min)

    static int nbExternalFiles = 0;

    if (nbExternalFiles != mpAddAFileItem->childCount())
    {
        mpScrollArea->verticalScrollBar()->setValue(max);
        nbExternalFiles = mpAddAFileItem->childCount();
    }
}

void UBTeacherGuidePageZeroWidget::setIsModified(bool isModified /* = true */)
{
    mIsModified = isModified;
}

//issue 1682 - NNE - 20140110
UBTeacherResources::UBTeacherResources(QWidget *parent) :
    UBAbstractTeacherGuide(parent)
{
    setObjectName("UBTeacherResources");
    mEditionWidget = new UBTeacherGuideResourceEditionWidget(this);

    this->addWidget(mEditionWidget);
    this->setCurrentWidget(mEditionWidget);

    mPresentationWidget =  new UBTeacherGuideResourcesPresentationWidget(this);
    this->addWidget(mPresentationWidget);

    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
}

void UBTeacherResources::changeMode()
{
    if(this->currentWidget() == mEditionWidget){
        this->setCurrentWidget(mPresentationWidget);
    }else{
        this->setCurrentWidget(mEditionWidget);
    }
}

void UBTeacherResources::onActiveSceneChanged()
{
    //This slot is called when the active scene changed.
    //At this time, the following methods have been called (in call order)
    //-             mEdition->onActiveSceneChanged()
    //-             mPresentationWidget->onActiveSceneChanged()
    //Refer to the contructor of UBTeacherResources to know why
    //the functions are called in this order.

    if (UBApplication::boardController->currentPage() > 0)
    {
        if(mEditionWidget->hasUserDataInTeacherGuide()){
            QVector<tUBGEElementNode*> data = mEditionWidget->getData();
            mPresentationWidget->showData(data);
            setCurrentWidget(mPresentationWidget);
        }
        else
        {
            setCurrentWidget(mEditionWidget);
        }
    }

}

void UBTeacherResources::showPresentationMode()
{

    QVector<tUBGEElementNode*> data = mEditionWidget->getData();
    mPresentationWidget->showData(data);

    setCurrentWidget(mPresentationWidget);
}

bool UBTeacherResources::isModified()
{
    return mEditionWidget->isModified();
}
//issue 1682 - NNE - 20140110 : END

bool UBTeacherResources::hasUserDataInTeacherGuide()
{
    return mEditionWidget->hasUserDataInTeacherGuide();
}
