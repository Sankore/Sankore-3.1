#include "UBTeacherGuideResourcesPresentationWidget.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "UBTeacherGuideConstantes.h"
#include "board/UBBoardController.h"
#include "web/UBWebController.h"
#include "globals/UBGlobals.h"

UBTeacherGuideResourcesPresentationWidget::UBTeacherGuideResourcesPresentationWidget(QWidget *parent):
    QWidget(parent)
{
    mpLayout = new QVBoxLayout(this);

    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");
    mpLayout->addWidget(mpPageNumberLabel);

    mpButtonLayout = new QHBoxLayout();

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/teacherGuide/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);
    mpModePushButton->installEventFilter(this);

    connect(mpModePushButton, SIGNAL(clicked()), parentWidget(), SLOT(changeMode()));
    mpButtonLayout->addWidget(mpModePushButton);
    mpButtonLayout->addStretch();

    mpLayout->addLayout(mpButtonLayout);

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

UBTeacherGuideResourcesPresentationWidget::~UBTeacherGuideResourcesPresentationWidget()
{
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpSeparator);
    DELETEPTR(mpMediaSwitchItem);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpButtonLayout);
    DELETEPTR(mpTreeWidget);
    DELETEPTR(mpLayout);
}

void UBTeacherGuideResourcesPresentationWidget::showData( QVector<tUBGEElementNode*> data)
{
    cleanData();
 #ifdef Q_WS_MAC
    if(mpMediaSwitchItem && mpMediaSwitchItem->isDisabled()){
        mpRootWidgetItem->removeChild(mpMediaSwitchItem);
        DELETEPTR(mpMediaSwitchItem);
    }
#endif

    foreach(tUBGEElementNode* element, data) {
        if (element->name == "media") {
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
            newWidgetItem->setIcon(0, QIcon( ":images/teacherGuide/" + element->attributes.value("mediaType") + "_24x24.svg"));
            newWidgetItem->setText(0, element->attributes.value("title"));
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_MEDIA);
            newWidgetItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 11)));
            QString mimeTypeString;
#ifdef Q_WS_WIN
            mimeTypeString = QUrl::fromLocalFile(UBApplication::boardController->selectedDocument()->persistencePath()+ "/" + element->attributes.value("relativePath")).toString();
#else
            mimeTypeString = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + element->attributes.value("relativePath");
#endif
            newWidgetItem->setData(0, TG_USER_ROLE_MIME_TYPE, mimeTypeString);
            newWidgetItem->setFlags( Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpRootWidgetItem->addChild(newWidgetItem);

            QTreeWidgetItem* mediaItem = new QTreeWidgetItem(newWidgetItem);
            mediaItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_NONE);
            qDebug() << element->attributes.value("mediaType");
            UBTGMediaWidget* mediaWidget = new UBTGMediaWidget(element->attributes.value("relativePath"), newWidgetItem,0,element->attributes.value("mediaType").contains("flash"));
            newWidgetItem->setExpanded(mpMediaSwitchItem->isExpanded());
            mpTreeWidget->setItemWidget(mediaItem, 0, mediaWidget);
        }
        else if (element->name == "link") {
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem( mpMediaSwitchItem);
            newWidgetItem->setIcon(0, QIcon(":images/teacherGuide/link_24x24.svg"));
            newWidgetItem->setText(0, element->attributes.value("title"));
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnAction, tUBTGActionAssociateOnClickItem_URL);
            newWidgetItem->setData(0, tUBTGTreeWidgetItemRole_HasAnUrl, QVariant(element->attributes.value("url")));
            newWidgetItem->setData(0, Qt::FontRole, QVariant(QFont(QApplication::font().family(), 11)));
            newWidgetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpRootWidgetItem->addChild(newWidgetItem);
        }
        else if (element->name == "file") { //Issue 1716 - ALTI/AOU - 20140128
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
}

void UBTeacherGuideResourcesPresentationWidget::createMediaButtonItem()
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

void UBTeacherGuideResourcesPresentationWidget::onActiveSceneChanged()
{    
    cleanData();
    mpPageNumberLabel->setText(tr("Page: %0").arg(UBApplication::boardController->currentPage()));
}

void UBTeacherGuideResourcesPresentationWidget::cleanData()
{
    //tree clean
    QList<QTreeWidgetItem*> itemToRemove = mpRootWidgetItem->takeChildren();
    foreach(QTreeWidgetItem* eachItem, itemToRemove) {
        delete eachItem;
    }
    // the mpMediaSwitchItem is deleted by the previous loop but the pointer is not set to zero
    mpMediaSwitchItem = NULL;
}

void UBTeacherGuideResourcesPresentationWidget::onAddItemClicked(QTreeWidgetItem* widget,int column)
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
        case tUBTGActionAssociateOnClickItem_MEDIA:
            widget->setExpanded(!widget->isExpanded());
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

bool UBTeacherGuideResourcesPresentationWidget::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
        return true;
    return false;
}

#ifdef Q_WS_MAC
void UBTeacherGuideResourcesPresentationWidget::onSliderMoved(int size)
{
    Q_UNUSED(size);
    if(mpMediaSwitchItem)
        mpMediaSwitchItem->setExpanded(true);
}
#endif
