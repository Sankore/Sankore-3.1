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



#include <QTreeWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QUrl>
#include <QWebSettings>
#include <QApplication>
#include <QDomElement>
#include <QWebFrame>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>

#include "UBTeacherGuideWidgetsTools.h"

#include "board/UBBoardPaletteManager.h"
#include "gui/UBDockTeacherGuideWidget.h"

#include "gui/UBMainWindow.h"

#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"
#include "core/UBTextTools.h"

#include "board/UBBoardController.h"

#include "domain/UBGraphicsWidgetItem.h"

#include "globals/UBGlobals.h"

#include "frameworks/UBFileSystemUtils.h"

#include "customWidgets/UBMediaWidget.h"

#include "core/memcheck.h"


/***************************************************************************
 *                             class    UBAddItem                          *
 ***************************************************************************/
UBAddItem::UBAddItem(const QString &string, int addSubItemWidgetType, QTreeWidget* parent): QTreeWidgetItem(parent)
{
    setIcon(0,QIcon(":images/increase.svg"));
    setText(0,string);
    setData(0,Qt::UserRole,QVariant(addSubItemWidgetType));
    setData(1,Qt::UserRole,QVariant(addSubItemWidgetType));
    setData(0,Qt::BackgroundRole,QVariant(QColor(200,200,200)));
    setData(1,Qt::BackgroundRole,QVariant(QColor(200,200,200)));
    setData(0,Qt::FontRole,QVariant(QFont(QApplication::font().family(),12)));
}

UBAddItem::~UBAddItem()
{
    //NOOP
}

/***************************************************************************
 *                      class    UBTGActionWidget                          *
 ***************************************************************************/
UBTGActionWidget::UBTGActionWidget(QTreeWidgetItem* widget, QWidget* parent, const char* name) : QWidget(parent)
  , mpLayout(NULL)
  , mpOwner(NULL)
  , mpTask(NULL)
{
    setObjectName(name);
    SET_STYLE_SHEET();
    mpLayout = new QVBoxLayout(this);
    mpOwner = new QComboBox(this);
    mpOwner->setObjectName("DockPaletteWidgetComboBox");
    mpOwner->setMinimumHeight(22);
    QStringList qslOwner;
    qslOwner << tr("Teacher") << tr("Student");
    mpOwner->insertItems(0,qslOwner);
    mpOwner->setCurrentIndex(0);
    mpTask = new UBTGAdaptableText(widget,this);
    mpTask->setPlaceHolderText(tr("Type task here ..."));
    mpTask->setAcceptRichText(true);
    mpTask->setObjectName("ActionWidgetTaskTextEdit");

    //N/C - NNE - 20140328 : Sort the actions
    mpTreeWidgetItem = widget;

    mpLayout->addWidget(mpOwner);
    mpLayout->addWidget(mpTask);

    //N/C - NNE - 20141020
    connect(mpOwner, SIGNAL(currentIndexChanged(int)), this, SLOT(onChange()));
    connect(mpTask, SIGNAL(textChanged()), this, SLOT(onChange()));
    //N/C - NNE - 20141020 : END
}

//N/C - NNE - 20141020
void UBTGActionWidget::onChange()
{
    emit hasChanged();
}

//N/C - NNE - 20141020 : END

//N/C - NNE - 20140328 : Sort the actions
void UBTGActionWidget::onUpButton()
{
    QTreeWidgetItem *parent = mpTreeWidgetItem->parent();

    int index = parent->indexOfChild(mpTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGActionWidget *copy = new UBTGActionWidget(mpTreeWidgetItem);

    copy->mpOwner->setCurrentIndex(mpOwner->currentIndex());
    copy->mpTask->setText(mpTask->text());

    parent->takeChild(index);

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mpTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));
    connect(actionColumn, SIGNAL(clickOnClose()), copy, SLOT(onClose()));


    //from now 'this' is invalid...
    index = qMax(0, index - 1);

    parent->insertChild(index, mpTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 1, actionColumn);

    //update the layout correctly
    mpTreeWidgetItem->setExpanded(true);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

void UBTGActionWidget::onDownButton()
{
    QTreeWidgetItem *parent = mpTreeWidgetItem->parent();

    int index = parent->indexOfChild(mpTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGActionWidget *copy = new UBTGActionWidget(mpTreeWidgetItem);

    copy->mpOwner->setCurrentIndex(mpOwner->currentIndex());
    copy->mpTask->setText(mpTask->text());

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mpTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));
    connect(actionColumn, SIGNAL(clickOnClose()), copy, SLOT(onClose()));

    parent->takeChild(index);

    //from now 'this' is invalid...
    index = qMin(parent->childCount(), index + 1);

    parent->insertChild(index, mpTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 1, actionColumn);

    //layout correctly
    mpTreeWidgetItem->setExpanded(true);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

void UBTGActionWidget::onClose()
{
    QTreeWidgetItem *parent = mpTreeWidgetItem->parent();

    int index = parent->indexOfChild(mpTreeWidgetItem);

    delete parent->takeChild(index);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}
//N/C - NNE - 20140328 : END

UBTGActionWidget::~UBTGActionWidget()
{
    DELETEPTR(mpOwner);
    DELETEPTR(mpTask);
    DELETEPTR(mpLayout);
}

void UBTGActionWidget::initializeWithDom(QDomElement element)
{
    mpOwner->setCurrentIndex(element.attribute("owner").toInt());
    mpTask->setInitialText(element.attribute("task"));
}

tUBGEElementNode* UBTGActionWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->name = "action";
    result->attributes.insert("owner",QString("%0").arg(mpOwner->currentIndex()));
    result->attributes.insert("task",mpTask->text());
    return result;
}

UBTGActionColumn::UBTGActionColumn(QTreeWidgetItem *widgetItem, QWidget *parent):
    QWidget(parent)
{
    QVBoxLayout *vl = new QVBoxLayout();

    mCloseButton = new QPushButton(QIcon(":images/close.svg"), "");
    mUpButton = new QPushButton(QIcon(":images/up_arrow.png"), "");
    mDownButton = new QPushButton(QIcon(":images/down_arrow.png"), "");
    mWidgetItem = widgetItem;

    vl->addStretch();
    vl->addWidget(mCloseButton);
    vl->addWidget(mUpButton);
    vl->addWidget(mDownButton);
    vl->addStretch();

    connect(mCloseButton, SIGNAL(clicked()), this, SLOT(onClickClose()));
    connect(mUpButton, SIGNAL(clicked()), this, SLOT(onClickUp()));
    connect(mDownButton, SIGNAL(clicked()), this, SLOT(onClickDown()));

    connect(mCloseButton, SIGNAL(clicked()), this, SIGNAL(clickOnClose()));
    connect(mUpButton, SIGNAL(clicked()), this, SIGNAL(clickOnUp()));
    connect(mDownButton, SIGNAL(clicked()), this, SIGNAL(clickOnDown()));

    mCloseButton->setFixedHeight(16);
    mDownButton->setFixedHeight(16);
    mUpButton->setFixedHeight(16);

    setLayout(vl);
}


void UBTGActionColumn::onClickClose()
{
    emit clickOnClose(mWidgetItem);
}

void UBTGActionColumn::onClickUp()
{
    emit clickOnUp(mWidgetItem);
}

void UBTGActionColumn::onClickDown()
{
    emit clickOnDown(mWidgetItem);
}

/***************************************************************************
 *                      class    UBTGAdaptableText                         *
 ***************************************************************************/
UBTGAdaptableText::UBTGAdaptableText(QTreeWidgetItem* widget, QWidget* parent, const char* name):QTextEdit(parent)
  , mBottomMargin(5)
  , mpTreeWidgetItem(widget)
  , mMinimumHeight(0)
  , mHasPlaceHolder(false)
  , mIsUpdatingSize(false)
  , mMaximumLength(0)
{
    setObjectName(name);

    //N/C - NNE - 20140326 : Text are truncated
    connect(this->document()->documentLayout(), SIGNAL(documentSizeChanged(QSizeF)), this, SLOT(onTextChanged()));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void UBTGAdaptableText::setMaximumLength(int length)
{
    mMaximumLength = length;
}

void UBTGAdaptableText::setPlaceHolderText(QString text)
{
    mHasPlaceHolder = true;

    // the space addition is to make this string unique and check against it to know
    // if we are talking about a typed string or the placeholder string
    mPlaceHolderText = text + "     ";
    setPlainText(mPlaceHolderText);
}

void UBTGAdaptableText::keyReleaseEvent(QKeyEvent* e)
{
    QTextEdit::keyReleaseEvent(e);

    if(mMaximumLength && toPlainText().length()>mMaximumLength){
        setPlainText(toPlainText().left(mMaximumLength));
        QTextCursor tc(document());
        tc.setPosition(mMaximumLength);
        setTextCursor(tc);
    }
}

void UBTGAdaptableText::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);

    if(!mIsUpdatingSize && !hasFocus() && mHasPlaceHolder && toPlainText().isEmpty() && !isReadOnly()){
        setTextColor(QColor(Qt::lightGray));
        setPlainText(mPlaceHolderText);
    }
    else
        // If the teacherguide is collapsed, don't updated the size. Or set the size as the expanded size
        onTextChanged();
}

QString UBTGAdaptableText::text()
{
    QString result = toPlainText();
    if(mHasPlaceHolder && result == mPlaceHolderText)
        return "";

    return result;
}

void UBTGAdaptableText::onTextChanged()
{
    //N/C - NNE - 20140326 : Text are truncated
    qreal documentSize = document()->size().height();

    mIsUpdatingSize = true;

    setFixedHeight(documentSize + contentsMargins().bottom() + contentsMargins().top());

    updateGeometry();

    //to trig a resize on the tree widget item
    if(mpTreeWidgetItem){
        mpTreeWidgetItem->setDisabled(true);
        mpTreeWidgetItem->setExpanded(true);
        mpTreeWidgetItem->setDisabled(false);
        setFocus();
    }

    mIsUpdatingSize = false;
    //N/C - NNE - 20140326 : END
}

void UBTGAdaptableText::setInitialText(const QString& text)
{
    setText(text);
    setReadOnly(false);
    onTextChanged();
}

void UBTGAdaptableText::resetText()
{
    if(mHasPlaceHolder && !mPlaceHolderText.isEmpty()){
        setTextColor(QColor(Qt::lightGray));
        setText(mPlaceHolderText);
    }
    else{
        setText("");
        setTextColor(QColor(Qt::black));
    }
    onTextChanged();
}

void UBTGAdaptableText::showText(const QString & text)
{
    setText(text);
    setReadOnly(true);
    onTextChanged();
}

void UBTGAdaptableText::bottomMargin(int newValue)
{
    mBottomMargin = newValue;
    onTextChanged();
}

void UBTGAdaptableText::focusInEvent(QFocusEvent* e)
{
    if(isReadOnly()){
        e->ignore();
    }
    managePlaceholder(true);
    QTextEdit::focusInEvent(e);
}

void UBTGAdaptableText::focusOutEvent(QFocusEvent* e)
{
    managePlaceholder(false);
    QTextEdit::focusOutEvent(e);
}

void UBTGAdaptableText::insertFromMimeData(const QMimeData *source)
{
    QMimeData editedMimeData;
    QTextDocument textDoc;
    QString plainText;

    if (source->hasHtml())
    {
        textDoc.setHtml(UBTextTools::cleanHtml(source->html()));
        plainText = textDoc.toPlainText();
    }
    if (source->hasText())
        if (textDoc.toPlainText() != source->text())
            plainText = source->text();
    if (source->hasUrls())
    {
        foreach(QUrl url, source->urls())
        {
            plainText = url.toString();
        }
    }

    editedMimeData.setText(plainText);
    QTextEdit::insertFromMimeData(&editedMimeData);
}

void UBTGAdaptableText::managePlaceholder(bool focus)
{
    disconnect(this,SIGNAL(textChanged()),this,SLOT(onTextChanged())); // ALTI/AOU - 20140207 : avoid infinite loop, because setPlainText raises a textChanged signal, and then onTextChanged calls setFocus, and focusOutEvent calls managePlaceHolder.
    if(focus){
        if(toPlainText() == mPlaceHolderText){
            setTextColor(QColor(Qt::black));
            setPlainText("");
            setCursorToTheEnd();
        }
    }
    else{
        if(toPlainText().isEmpty()){
            setTextColor(QColor(Qt::lightGray));
            setPlainText(mPlaceHolderText);
        }
    }
    connect(this,SIGNAL(textChanged()),this,SLOT(onTextChanged())); // ALTI/AOU - 20140207 : re-connect.
}

void UBTGAdaptableText::setCursorToTheEnd()
{
    QTextDocument* doc = document();
    if(NULL != doc){
        QTextBlock block = doc->lastBlock();
        QTextCursor cursor(doc);
        cursor.setPosition(block.position() + block.length() - 1);
        setTextCursor(cursor);
    }
}

/***************************************************************************
 *                      class   UBTGDraggableWeb                           *
 ***************************************************************************/
UBDraggableWeb::UBDraggableWeb(QString& relativePath, QWidget* parent): QWebView(parent)
  , mDragStartPosition(QPoint(-1,-1))
  , mDragStarted(false)

{
    if(!relativePath.startsWith("file://"))
        mRelativePath = QUrl::fromLocalFile(relativePath).toString();
    else
        mRelativePath = relativePath;
    //NOOP
}

void UBDraggableWeb::mousePressEvent(QMouseEvent* event)
{
    mDragStartPosition = event->pos();
    mDragStarted = true;
    QWebView::mousePressEvent(event);
}

void UBDraggableWeb::mouseReleaseEvent(QMouseEvent* event)
{
    mDragStarted = false;
    QWebView::mouseReleaseEvent(event);
}

void UBDraggableWeb::mouseMoveEvent(QMouseEvent* event)
{
    if(mDragStarted && (event->pos() - mDragStartPosition).manhattanLength() > QApplication::startDragDistance()){
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urlList;
        urlList << QUrl(mRelativePath);
        mimeData->setUrls(urlList);
        drag->setMimeData(mimeData);

        drag->exec();
        event->accept();
        mDragStarted = false;
    }
    else
        QWebView::mouseMoveEvent(event);

}

/***************************************************************************
 *                      class    UBTGMediaWidget                           *
 ***************************************************************************/
UBTGMediaWidget::UBTGMediaWidget(QTreeWidgetItem* widget, QWidget* parent,const char* name): QStackedWidget(parent)
  , mpTreeWidgetItem(widget)
  , mpDropMeWidget(NULL)
  , mpWorkWidget(NULL)
  , mpLayout(NULL)
  , mpMediaLayout(NULL)
  , mpTitle(NULL)
  , mpMediaLabelWidget(NULL)
  , mpMediaWidget(NULL)
  , mpWebView(NULL)
  , mMediaPath(QString(""))
  , mIsPresentationMode(false)
  , mIsInitializationMode(false)
  , mMediaWidgetHeight(150)
{
    setObjectName(name);
    mpDropMeWidget = new QLabel();
    mpDropMeWidget->setObjectName("UBTGMediaDropMeLabel");
    mpDropMeWidget->setText(tr("drop media here ..."));
    mpDropMeWidget->setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    addWidget(mpDropMeWidget);

    setMinimumHeight(250);
}

UBTGMediaWidget::UBTGMediaWidget(QString mediaPath, QTreeWidgetItem* widget, QWidget* parent,bool forceFlashMediaType,const char* name): QStackedWidget(parent)
  , mpTreeWidgetItem(widget)
  , mpDropMeWidget(NULL)
  , mpWorkWidget(NULL)
  , mpLayout(NULL)
  , mpMediaLayout(NULL)
  , mpTitle(NULL)
  , mpMediaLabelWidget(NULL)
  , mpMediaWidget(NULL)
  , mpWebView(NULL)
  , mIsPresentationMode(true)
  , mMediaType("")
  , mIsInitializationMode(false)
  , mMediaWidgetHeight(150)
{
    setObjectName(name);
    mMediaPath = UBApplication::boardController->selectedDocument()->persistencePath()+ "/" + mediaPath;
    setAcceptDrops(false);
    createWorkWidget(forceFlashMediaType);
    setFixedHeight(200);
}

//N/C - NNE - 20140328 : Sort the actions
void UBTGMediaWidget::onUpButton()
{
    QTreeWidgetItem *parent = mpTreeWidgetItem->parent();

    int index = parent->indexOfChild(mpTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGMediaWidget *copy = clone();

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mpTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));

    //from now 'this' is invalid...
    parent->takeChild(index);

    index = qMax(0, index - 1);

    parent->insertChild(index, mpTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 1, actionColumn);

    //update the layout correctly
    mpTreeWidgetItem->setExpanded(true);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

//N/C - NNE - 20140401
UBTGMediaWidget *UBTGMediaWidget::clone() const
{
    bool forceFlash = mMediaType.toLower() == "flash";

    UBTGMediaWidget *copy = new UBTGMediaWidget(mpTreeWidgetItem);

    copy->mMediaPath = mMediaPath;
    copy->createWorkWidget(forceFlash);
    copy->mpTitle->setText(mpTitle->text());

    return copy;
}
//N/C - NNE - 20140401 : END

void UBTGMediaWidget::onDownButton()
{
    QTreeWidgetItem *parent = mpTreeWidgetItem->parent();

    int index = parent->indexOfChild(mpTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGMediaWidget *copy = clone();

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mpTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));

    //from now 'this' is invalid...
    parent->takeChild(index);

    index = qMin(parent->childCount(), index + 1);

    parent->insertChild(index, mpTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mpTreeWidgetItem, 1, actionColumn);

    //update the layout correctly
    mpTreeWidgetItem->setExpanded(true);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

void UBTGMediaWidget::onClose()
{
    removeSource();
    QTreeWidgetItem *parent = mpTreeWidgetItem->parent();

    int index = parent->indexOfChild(mpTreeWidgetItem);

    delete parent->takeChild(index);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}
//N/C - NNE - 20140328 : END

void UBTGMediaWidget::onTitleChanged()
{
    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

UBTGMediaWidget::~UBTGMediaWidget()
{
    DELETEPTR(mpTitle);
    DELETEPTR(mpMediaLabelWidget);
    DELETEPTR(mpMediaWidget);
    DELETEPTR(mpWebView);
    DELETEPTR(mpMediaLayout);
    DELETEPTR(mpLayout);

    removeWidget(mpDropMeWidget);
    DELETEPTR(mpDropMeWidget);
    removeWidget(mpWorkWidget);
    DELETEPTR(mpWorkWidget);
}

void UBTGMediaWidget::initializeWithDom(QDomElement element)
{
    mIsInitializationMode = true;
    setAcceptDrops(false);
    mMediaPath = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + element.attribute("relativePath");
    createWorkWidget(element.attribute("mediaType").contains("flash"));
    setFixedHeight(200);
    mpTitle->setInitialText(element.attribute("title"));
    mIsInitializationMode = false;
}

void UBTGMediaWidget::removeSource()
{
    QFileInfo fileInfo(mMediaPath);
    if(fileInfo.isFile())
        QFile(mMediaPath).remove();
    else
        UBFileSystemUtils::deleteDir(mMediaPath);
}

void UBTGMediaWidget::hideEvent(QHideEvent* event)
{
    if(mpWebView)
        mpWebView->page()->mainFrame()->setContent(UBGraphicsW3CWidgetItem::freezedWidgetPage().toAscii());
    QWidget::hideEvent(event);
}

void UBTGMediaWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if(mpWebView){
        QString indexPath = mMediaPath+"/index.htm";
        if(!QFile::exists(indexPath))
            indexPath += "l";
        mpWebView->load(QUrl::fromLocalFile(indexPath));
    }
}

tUBGEElementNode* UBTGMediaWidget::saveData()
{
    if(!mpTitle)
        return 0;
    tUBGEElementNode* result = new tUBGEElementNode();
    QString relativePath = mMediaPath;
    relativePath = relativePath.replace(UBApplication::boardController->selectedDocument()->persistencePath()+"/","");
    result->name = "media";
    result->attributes.insert("title",mpTitle->text());
    result->attributes.insert("relativePath",relativePath);
    result->attributes.insert("mediaType",mMediaType);
    return result;
}

void UBTGMediaWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void UBTGMediaWidget::createWorkWidget(bool forceFlashMediaType)
{
    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(mMediaPath);
    bool setMedia = true;
    UBDocumentProxy* proxyDocument = UBApplication::boardController->selectedDocument();
    if(mimeType.contains("audio") || mimeType.contains("video")){
        mMediaType = mimeType.contains("audio")? "audio":"movie";
        mpMediaWidget = new UBMediaWidget(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
        if(mIsPresentationMode || mIsInitializationMode){
            mpMediaWidget->setFile(mMediaPath);
        }
        else{
            mMediaPath = UBPersistenceManager::persistenceManager()->addObjectToTeacherGuideDirectory(proxyDocument, mMediaPath);
            mpMediaWidget->setFile(mMediaPath);
        }
    }
    else if(mimeType.contains("image")){
        mMediaType = "image";
        if(!(mIsPresentationMode || mIsInitializationMode))
            mMediaPath = UBPersistenceManager::persistenceManager()->addObjectToTeacherGuideDirectory(proxyDocument, mMediaPath);

        mpMediaLabelWidget = new QLabel();
        QPixmap pixmap = QPixmap(mMediaPath);
        pixmap = pixmap.scaledToHeight(mMediaWidgetHeight);
        mpMediaLabelWidget->setPixmap(pixmap);
    }
    else if(mimeType.contains("widget") && !forceFlashMediaType){
        mMediaType = "w3c";
        if(!(mIsPresentationMode || mIsInitializationMode)){
            mMediaPath = UBPersistenceManager::persistenceManager()->addWidgetToTeacherGuideDirectory(proxyDocument, mMediaPath);
        }
        mpWebView = new UBDraggableWeb(mMediaPath);
        mpWebView->setAcceptDrops(false);
        mpWebView->settings()->setAttribute(QWebSettings::JavaEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        mpWebView->settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
        QString indexPath = mMediaPath+"/index.htm";
        if(!QFile::exists(indexPath))
            indexPath += "l";
        mpWebView->load(QUrl::fromLocalFile(indexPath));
    }
    else if(mimeType.contains("x-shockwave-flash") || forceFlashMediaType){
        mMediaType = "flash";
        if(!(mIsPresentationMode || mIsInitializationMode)){
            QDir baseW3CDirectory(UBPersistenceManager::persistenceManager()->teacherGuideAbsoluteObjectPath(proxyDocument));
            mMediaPath = UBGraphicsW3CWidgetItem::createNPAPIWrapperInDir(mMediaPath,baseW3CDirectory,mimeType,QSize(100,100),QUuid::createUuid());
        }
        qDebug() << mMediaPath;
        mpWebView = new UBDraggableWeb(mMediaPath);
        mpWebView->setAcceptDrops(false);
        mpWebView->settings()->setAttribute(QWebSettings::JavaEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        mpWebView->settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
        QString indexPath = mMediaPath+"/index.htm";
        if(!QFile::exists(indexPath))
            indexPath += "l";
        mpWebView->load(QUrl::fromLocalFile(indexPath));
    }
    else{
        qDebug() << "createWorkWidget mime type not handled" << mimeType;
        setMedia=false;
    }

    if(setMedia){
        setAcceptDrops(false);
        mpWorkWidget = new QWidget(this);
        if(!mIsPresentationMode){
            mpLayout = new QVBoxLayout(mpWorkWidget);
            mpTitle = new UBTGAdaptableText(mpTreeWidgetItem,mpWorkWidget);
            mpTitle->setPlaceHolderText(tr("Type title here..."));
            mpLayout->addWidget(mpTitle);
            mpMediaLayout = new QHBoxLayout;
            mpLayout->addLayout(mpMediaLayout);
            mpWorkWidget->setLayout(mpLayout);
            connect(mpTitle, SIGNAL(textChanged()), this, SLOT(onTitleChanged()));
        }
        else{
            mpMediaLayout = new QHBoxLayout(mpWorkWidget);
            mpWorkWidget->setLayout(mpMediaLayout);
        }

        mpMediaLayout->addStretch(1);

        if(mpMediaLabelWidget){
            mpMediaLabelWidget->setFixedHeight(mMediaWidgetHeight);
            mpMediaLabelWidget->setParent(mpWorkWidget);
            mpMediaLayout->addWidget(mpMediaLabelWidget);
        }
        else if (mpMediaWidget){
            mpMediaWidget->setFixedHeight(mMediaWidgetHeight);
            mpMediaWidget->setParent(mpWorkWidget);
            mpMediaLayout->addWidget(mpMediaWidget);
        }
        else if (mpWebView){
            mpWebView->setFixedHeight(mMediaWidgetHeight);
            mpWebView->setParent(mpWorkWidget);
            mpMediaLayout->addWidget(mpWebView);
            mpWebView->show();
        }
        mpMediaLayout->addStretch(1);

        addWidget(mpWorkWidget);
        setCurrentWidget(mpWorkWidget);
        mpWorkWidget->show();

    }
}

void UBTGMediaWidget::parseMimeData(const QMimeData* pMimeData)
{
    if(pMimeData){
        if(pMimeData->hasText()){
            mMediaPath = QUrl::fromLocalFile(pMimeData->text()).toString();
        }
        else if(pMimeData->hasUrls() && pMimeData->urls().count()){ // on windows found hasUrls = true and count = 0
            mMediaPath = pMimeData->urls().at(0).toString();
        }
        else if(pMimeData->hasImage()){
            qDebug() << "Not yet implemented";
        }
    }
    else
        qDebug() << "No mime data present";

    if(mMediaPath.startsWith("file:smb://") || //linux
        !mMediaPath.startsWith("file:///") ) //windows local file -> file:///%A_DRIVER%:/ vs windows smb share file -> file://%SAMBA_SHARE_NAME%
        UBApplication::mainWindow->information(tr("Drag and drop"),tr("The currect action is not supported. The teacher bar is design to work only with media stored locally."));
    else
        createWorkWidget();
}

void UBTGMediaWidget::dropEvent(QDropEvent* event)
{
    parseMimeData(event->mimeData());
    event->accept();
}

void UBTGMediaWidget::mousePressEvent(QMouseEvent *event)
{
    if (!mIsPresentationMode)
        event->ignore();
    else{
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData();
        QList<QUrl> urlList;
#ifdef Q_WS_WIN
        urlList << QUrl::fromLocalFile(mMediaPath);
#else
        urlList << QUrl(mMediaPath);
#endif
        mimeData->setUrls(urlList);
        drag->setMimeData(mimeData);

        drag->exec();
        event->accept();
    }
}



/***************************************************************************
 *                      class    UBTGUrlWidget                             *
 ***************************************************************************/
UBTGUrlWidget::UBTGUrlWidget(QTreeWidgetItem *treeWidgetItem, QWidget *parent, const char *name ):
    QWidget(parent)
  , mpLayout(NULL)
  , mpTitle(NULL)
  , mpUrl(NULL)
{
    setObjectName(name);
    SET_STYLE_SHEET();
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("UBTGLineEdit");
    mpTitle->setPlaceholderText(tr("Insert link title here..."));
    mpUrl = new QLineEdit(this);
    connect(mpUrl,SIGNAL(editingFinished()),this,SLOT(onUrlEditionFinished()));
    mpUrl->setObjectName("UBTGLineEdit");
    mpUrl->setPlaceholderText("http://");
    mpLayout->addWidget(mpTitle);
    mpLayout->addWidget(mpUrl);

    mTreeWidgetItem = treeWidgetItem;
}

UBTGUrlWidget::~UBTGUrlWidget()
{
    DELETEPTR(mpTitle);
    DELETEPTR(mpUrl);
    DELETEPTR(mpLayout);
}

void UBTGUrlWidget::onUrlEditionFinished()
{
    QString url = mpUrl->text();
    if(url.length() && !url.startsWith("http://") && !url.startsWith("https://") && !url.startsWith("ftp://") && !url.startsWith("sftp://") && !url.startsWith("http://")){
        mpUrl->setText("http://" + mpUrl->text());
        setFocus();
    }
}

void UBTGUrlWidget::initializeWithDom(QDomElement element)
{
    mpTitle->setText(element.attribute("title"));
    mpUrl->setText(element.attribute("url"));
}

//N/C - NNE - 20140328 : Sort the url
void UBTGUrlWidget::onUpButton()
{
    QTreeWidgetItem *parent = mTreeWidgetItem->parent();

    int index = parent->indexOfChild(mTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGUrlWidget *copy = clone();

    parent->takeChild(index);

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));
    connect(actionColumn, SIGNAL(clickOnClose()), copy, SLOT(onClose()));

    //from now 'this' is invalid...
    index = qMax(0, index - 1);

    parent->insertChild(index, mTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 1, actionColumn);

    //update the layout correctly
    mTreeWidgetItem->setExpanded(true);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

void UBTGUrlWidget::onDownButton()
{
    QTreeWidgetItem *parent = mTreeWidgetItem->parent();

    int index = parent->indexOfChild(mTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGUrlWidget *copy = clone();

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));
    connect(actionColumn, SIGNAL(clickOnClose()), copy, SLOT(onClose()));

    parent->takeChild(index);

    //from now 'this' is invalid...
    index = qMin(parent->childCount(), index + 1);

    parent->insertChild(index, mTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 1, actionColumn);

    //update the layout correctly
    mTreeWidgetItem->setExpanded(true);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

UBTGUrlWidget * UBTGUrlWidget::clone() const
{
    UBTGUrlWidget *copy = new UBTGUrlWidget(mTreeWidgetItem);

    copy->mpTitle->setText(mpTitle->text());
    copy->mpUrl->setText(mpUrl->text());

    return copy;
}

void UBTGUrlWidget::onClose()
{
    QTreeWidgetItem *parent = mTreeWidgetItem->parent();

    int index = parent->indexOfChild(mTreeWidgetItem);

    delete parent->takeChild(index);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

tUBGEElementNode* UBTGUrlWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->name = "link";
    result->attributes.insert("title",mpTitle->text());
    result->attributes.insert("url",mpUrl->text());
    return result;
}


/***************************************************************************
 *              class    UBTGDraggableTreeItem                             *
 ***************************************************************************/
UBTGDraggableTreeItem::UBTGDraggableTreeItem(QWidget* parent, const char* name) : QTreeWidget(parent)
{
    setObjectName(name);
}

QMimeData* UBTGDraggableTreeItem::mimeData(const QList<QTreeWidgetItem *> items) const
{
    QMimeData* result = new QMimeData();
    QList<QUrl> urls;
    urls << QUrl(items.at(0)->data(0,TG_USER_ROLE_MIME_TYPE).toString());
    result->setUrls(urls);
    return result;
}

// Issue 1683 (Evolution) - AOU - 20131206
/***************************************************************************
 *              class    UBTGFileWidget                                    *
 ***************************************************************************/
UBTGFileWidget::UBTGFileWidget(QTreeWidgetItem *treeWidgetItem, QWidget *parent, const char *name)
    : QWidget(parent)
    , mpLayout(NULL)
    , mpHLayout(NULL)
    , mpTitreFichier(NULL)
    , mpNomFichier(NULL)
    , mpBtnSelectFile(NULL)
    , mPath("")
{
    setObjectName(name);
    SET_STYLE_SHEET();
    mpLayout = new QVBoxLayout();

    mpTitreFichier = new QLineEdit;
    mpTitreFichier->setObjectName("UBTGLineEdit");
    mpTitreFichier->setPlaceholderText(tr("Insert file title here..."));
    mpLayout->addWidget(mpTitreFichier);

    mpHLayout = new QHBoxLayout;

    mpNomFichier = new QLineEdit;
    mpNomFichier->setEnabled(false);
    mpNomFichier->setObjectName("UBTGLineEdit");

    mpBtnSelectFile = new QPushButton("...");
    mpBtnSelectFile->setObjectName("UBTGPushButton");

    connect(mpBtnSelectFile, SIGNAL(clicked()), this, SLOT(OnClickBtnSelectFile()));
    mpHLayout->addWidget(mpNomFichier);
    mpHLayout->addWidget(mpBtnSelectFile);

    mpLayout->addLayout(mpHLayout);

    setLayout(mpLayout);

    mTreeWidgetItem = treeWidgetItem;

    connect(mpTitreFichier, SIGNAL(textEdited(QString)), this, SIGNAL(changed()));
}

UBTGFileWidget::~UBTGFileWidget()
{
    DELETEPTR(mpTitreFichier);
    DELETEPTR(mpNomFichier);
    DELETEPTR(mpBtnSelectFile);
    DELETEPTR(mpHLayout);
    DELETEPTR(mpLayout);
}

tUBGEElementNode* UBTGFileWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->name = "file";
    result->attributes.insert("title",mpTitreFichier->text());
    result->attributes.insert("path",mPath);
    return result;
}

void UBTGFileWidget::initializeWithDom(QDomElement element)
{
    mPath = element.attribute("path");
    mpTitreFichier->setText(element.attribute("title"));
    mpNomFichier->setText(mPath.mid(mPath.lastIndexOf('/') + 1));
}

void UBTGFileWidget::OnClickBtnSelectFile()
{

    // Ouvrir une dialog de selection de fichier :
    QString filename = QFileDialog::getOpenFileName(UBApplication::mainWindow, tr("Select File"), QString(), "*.*", NULL);

#ifdef Q_OS_MACX
    filename = filename.normalized(QString::NormalizationForm_C); // Issue - ALTI/AOU - 20140526 : on MacOSX, file names are in a special UTF-8 "NFD" encoding. We must convert the file name to a standard UTF-8.
#endif

    if (filename.length() > 0)
    {
        QString documentPath = UBApplication::boardController->selectedDocument()->persistencePath();

        // On a selectionné un fichier :
        // Si un fichier avait déjà été embarqué dans le dossier Document, on le supprime :
        if ( mPath.left(mPath.indexOf('/') ) == UBPersistenceManager::teacherGuideDirectory)
        {
            QFile file(documentPath + "/" + mPath);
            QFileInfo fi(file);
            file.remove();  // supprimer le fichier
            QDir parentDir = fi.dir();
            parentDir.rmpath(parentDir.absolutePath()); // supprimer le repertoire
        }

        QFileInfo fileInfo(filename);
        if (fileInfo.exists())
        {
            mpNomFichier->setText(fileInfo.fileName());

            // Dupliquer le fichier vers le dossier Document/TeacherGuide/Files/UUID/nomFichier.
            // On crée un nouveau dossier pour y copier le fichier à embarquer :
            QString dirPath = UBPersistenceManager::teacherGuideDirectory + "/" + UBPersistenceManager::fileDirectory + "/" + QUuid::createUuid().toString();
            QDir docDir(documentPath);
            docDir.mkpath(dirPath);

            // On copie le fichier dans ce nouveau repertoire :
            QString path = dirPath + "/" + fileInfo.fileName();
            QFile::copy(fileInfo.absoluteFilePath(), documentPath + "/" + path);

            setPath(path);
            if (mpTitreFichier->text().trimmed().isEmpty()){
                mpTitreFichier->setText(fileInfo.fileName()); // default Title is file name.
            }
        }

        emit changed();
    }
}

//N/C - NNE - 20140328 : Sort the url
void UBTGFileWidget::onUpButton()
{
    QTreeWidgetItem *parent = mTreeWidgetItem->parent();

    int index = parent->indexOfChild(mTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGFileWidget *copy = clone();

    parent->takeChild(index);

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));
    connect(actionColumn, SIGNAL(clickOnClose()), copy, SLOT(onClose()));

    //from now 'this' is invalid...
    index = qMax(0, index - 1);

    parent->insertChild(index, mTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 1, actionColumn);

    //update the layout correctly
    mTreeWidgetItem->setExpanded(true);

    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

void UBTGFileWidget::onDownButton()
{
    QTreeWidgetItem *parent = mTreeWidgetItem->parent();

    int index = parent->indexOfChild(mTreeWidgetItem);

    //make a copy, because Qt delete the widget (according to
    //the documentation, the widget is hold by the treeWidget)
    UBTGFileWidget *copy = clone();

    //recreate the actionColumn, because Qt delete it when we remove the node...
    UBTGActionColumn *actionColumn = new UBTGActionColumn(mTreeWidgetItem);

    connect(actionColumn, SIGNAL(clickOnUp()), copy, SLOT(onUpButton()));
    connect(actionColumn, SIGNAL(clickOnDown()), copy, SLOT(onDownButton()));
    connect(actionColumn, SIGNAL(clickOnClose()), copy, SLOT(onClose()));

    parent->takeChild(index);

    //from now 'this' is invalid...
    index = qMin(parent->childCount(), index + 1);

    parent->insertChild(index, mTreeWidgetItem);

    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 0, copy);
    parent->treeWidget()->setItemWidget(mTreeWidgetItem, 1, actionColumn);

    //layout correctly
    mTreeWidgetItem->setExpanded(true);

    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

UBTGFileWidget * UBTGFileWidget::clone() const
{
    UBTGFileWidget *copy = new UBTGFileWidget(mTreeWidgetItem);

    copy->mpTitreFichier->setText(mpTitreFichier->text());
    copy->mpNomFichier->setText(mpNomFichier->text());
    copy->setPath(path());

    return copy;
}

void UBTGFileWidget::onClose()
{
    QString documentPath = UBApplication::boardController->selectedDocument()->persistencePath();

    if ( mPath.left(mPath.indexOf('/') ) == UBPersistenceManager::teacherGuideDirectory)
    {
        QFile file(documentPath + "/" + mPath);
        QFileInfo fi(file);
        file.remove();  // supprimer le fichier
        QDir parentDir = fi.dir();
        parentDir.rmpath(parentDir.absolutePath()); // supprimer le repertoire
    }

    QTreeWidgetItem *parent = mTreeWidgetItem->parent();

    int index = parent->indexOfChild(mTreeWidgetItem);

    delete parent->takeChild(index);

    //notify that the teacher guide edition has been changed
    UBApplication::boardController->paletteManager()->teacherGuideDockWidget()->teacherGuideWidget()->teacherGuideEditionWidget()->teacherGuideChanged();
}

// Fin Issue 1683 (Evolution) - AOU - 20131206

