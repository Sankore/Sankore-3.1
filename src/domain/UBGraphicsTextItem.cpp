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



#include <QtGui>
#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsTextItem.h"
#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "core/UBSettings.h"

#include "document/UBDocumentProxy.h"
#include "core/UBApplication.h"
#include "document/UBDocumentController.h"
#include "board/UBBoardController.h"
#include "document/UBDocumentProxy.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBPersistenceManager.h"
#include "core/UBTextTools.h"

#include "gui/UBCreateTablePalette.h"
#include "core/memcheck.h"

QColor UBGraphicsTextItem::lastUsedTextColor;

UBGraphicsTextItem::UBGraphicsTextItem(QGraphicsItem * parent) :
    QGraphicsTextItem(parent)
    , UBGraphicsItem()
    , mMultiClickState(0)
    , mLastMousePressTime(QTime::currentTime())
    , mBackgroundColor(QColor(Qt::transparent))
    , mHtmlIsInterpreted(false)
{    
    setDelegate(new UBGraphicsTextItemDelegate(this));
    Delegate()->init();

    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);
    Delegate()->setCanTrigAnAction(true);

    mTypeTextHereLabel = tr("<Type Text Here>");


    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly


    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setTextInteractionFlags(Qt::TextEditorInteraction);

    //issue 1554 - NNE - 20131008
    isActivatedTextEditor = true;

    setUuid(QUuid::createUuid());

    connect(document(), SIGNAL(contentsChanged()), Delegate(), SLOT(contentsChanged()));
    connect(document(), SIGNAL(undoCommandAdded()), this, SLOT(undoCommandAdded()));
    connect(this, SIGNAL(linkActivated(QString)), this, SLOT(loadUrl(QString)));


    connect(document()->documentLayout(), SIGNAL(documentSizeChanged(const QSizeF &)),
            this, SLOT(documentSizeChanged(const QSizeF &)));

}

UBGraphicsTextItem::~UBGraphicsTextItem()
{
}

void UBGraphicsTextItem::insertColumn(bool onRight)
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        if (onRight)
            t->insertColumns(t->cellAt(textCursor()).column()+1, 1);
        else
            t->insertColumns(t->cellAt(textCursor()).column(), 1);
    }
}

void UBGraphicsTextItem::insertRow(bool onRight)
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        if (onRight)
            t->insertRows(t->cellAt(textCursor()).row()+1, 1);
        else
            t->insertRows(t->cellAt(textCursor()).row(), 1);
    }
}

void UBGraphicsTextItem::deleteColumn()
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        t->removeColumns(t->cellAt(textCursor()).column(), 1);
    }
}

void UBGraphicsTextItem::deleteRow()
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        t->removeRows(t->cellAt(textCursor()).row(), 1);
    }
}


void UBGraphicsTextItem::setCellWidth(int percent)
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        QTextTableFormat f = t->format();
        f.clearColumnWidthConstraints();
        f.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        QVector<QTextLength> constraints = t->format().columnWidthConstraints();
        constraints.replace(t->cellAt(textCursor()).column(), QTextLength(QTextLength::PercentageLength, percent));
        f.setColumnWidthConstraints(constraints);
        t->setFormat(f);
    }
}

QVariant UBGraphicsTextItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsTextItem::itemChange(change, newValue);
}

void UBGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // It is a cludge...
    if (UBStylusTool::Play == UBDrawingController::drawingController()->stylusTool())
    {
        QGraphicsTextItem::mousePressEvent(event);
        event->accept();
        clearFocus();
        Delegate()->startUndoStep(); //Issue 1541 - AOU - 20131002
        return;
    }

    if (Delegate())
    {
        Delegate()->mousePressEvent(event);
        if (Delegate() && parentItem() && UBGraphicsGroupContainerItem::Type == parentItem()->type())
        {
            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(parentItem());
            if (group)
            {
                QGraphicsItem *curItem = group->getCurrentItem();
                if (curItem && this != curItem)
                {
                    group->deselectCurrentItem();
                }
                group->setCurrentItem(this);
                this->setSelected(true);
                Delegate()->positionHandles();
            }

        }
        else
        {

            Delegate()->getToolBarItem()->show();
        }

    }

    if (!data(UBGraphicsItemData::ItemEditable).toBool())
        return;

    int elapsed = mLastMousePressTime.msecsTo(QTime::currentTime());

    if (elapsed < UBApplication::app()->doubleClickInterval())
    {
        mMultiClickState++;
        if (mMultiClickState > 3)
            mMultiClickState = 1;
    }
    else
    {
        mMultiClickState = 1;
    }

    mLastMousePressTime = QTime::currentTime();

    if (mMultiClickState == 1)
    {
        //issue 1554 - NNE - 20131008
        activateTextEditor(true);

        QGraphicsTextItem::mousePressEvent(event);
        setFocus();
    }
    else if (mMultiClickState == 2)
    {
        QTextCursor tc= textCursor();
        tc.select(QTextCursor::WordUnderCursor);
        setTextCursor(tc);
    }
    else if (mMultiClickState == 3)
    {
        QTextCursor tc= textCursor();
        tc.select(QTextCursor::Document);
        setTextCursor(tc);
    }
    else
    {
        mMultiClickState = 0;
    }
}

void UBGraphicsTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!Delegate() || !Delegate()->mouseMoveEvent(event))
    {
        QGraphicsTextItem::mouseMoveEvent(event);
    }
}

void UBGraphicsTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // scene()->itemAt(pos) returns 0 if pos is not over text, but over text item, but mouse press comes.
    // It is a cludge...
    if (UBStylusTool::Play == UBDrawingController::drawingController()->stylusTool())
    {
        QPointF distance = event->pos() - event->lastPos();
        if( fabs(distance.x()) < 1 && fabs(distance.y()) < 1 )
            Delegate()->mouseReleaseEvent(event);
        else
            QGraphicsTextItem::mouseReleaseEvent(event);

        event->accept();
        clearFocus();
        return;
    }

    if (mMultiClickState == 1)
    {
        if (Delegate())
            Delegate()->mouseReleaseEvent(event);

        QGraphicsTextItem::mouseReleaseEvent(event);
    }
    else
    {
        event->accept();
    }
}

void UBGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (mBackgroundColor != Qt::transparent && !mHtmlIsInterpreted)
    {
        painter->setPen(Qt::transparent);
        painter->setBrush(mBackgroundColor);
        painter->drawRect(boundingRect());
    }

    QColor color = UBSettings::settings()->isDarkBackground() ? mColorOnDarkBackground : mColorOnLightBackground;
    setDefaultTextColor(color);

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    QGraphicsTextItem::paint(painter, &styleOption, widget);

    if (widget == UBApplication::boardController->controlView()->viewport() &&
            !isSelected() && toPlainText().isEmpty())
    {
        painter->setFont(font());
        painter->setPen(UBSettings::paletteColor);
        painter->drawText(boundingRect(), Qt::AlignCenter, mTypeTextHereLabel);
    }
}

bool UBGraphicsTextItem::htmlMode() const
{
    return mHtmlIsInterpreted;
}

void UBGraphicsTextItem::setHtmlMode(const bool mode)
{
    mHtmlIsInterpreted = mode;
}

void UBGraphicsTextItem::loadUrl(QString url)
{
    UBApplication::loadUrl(url);
}

UBItem* UBGraphicsTextItem::deepCopy() const
{
    UBGraphicsTextItem* copy = new UBGraphicsTextItem();

    copyItemParameters(copy);

   // TODO UB 4.7 ... complete all members ?

   return copy;
}

void UBGraphicsTextItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsTextItem *cp = dynamic_cast<UBGraphicsTextItem*>(copy);
    if (cp)
    {
        cp->setHtml(toHtml());
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setData(UBGraphicsItemData::ItemEditable, data(UBGraphicsItemData::ItemEditable).toBool());
        cp->setTextWidth(this->textWidth());
        cp->setTextHeight(this->textHeight());

        if(Delegate()->action()){
            if(Delegate()->action()->linkType() == eLinkToAudio){
                UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
                UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
                cp->Delegate()->setAction(action);
            }
            else
                cp->Delegate()->setAction(Delegate()->action());
        }
    }
}


QRectF UBGraphicsTextItem::boundingRect() const
{
    qreal width = textWidth();
    qreal height = textHeight();
    return QRectF(QPointF(), QSizeF(width, height));
}


QPainterPath UBGraphicsTextItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void UBGraphicsTextItem::setTextWidth(qreal width)
{
    QFontMetrics fm(font());
    qreal strictMin = fm.height();
    qreal newWidth = qMax(strictMin, width);

    if (toPlainText().isEmpty())
    {
        qreal minWidth = fm.width(mTypeTextHereLabel);
        newWidth = qMax(minWidth, newWidth);
    }

    QGraphicsTextItem::setTextWidth(newWidth);
}


void UBGraphicsTextItem::setTextHeight(qreal height)
{
    QFontMetrics fm(font());
    qreal minHeight = fm.height() + document()->documentMargin() * 2;
    mTextHeight = qMax(minHeight, height);
    update();
    setFocus();
}


qreal UBGraphicsTextItem::textHeight() const
{
    return mTextHeight;
}


void UBGraphicsTextItem::contentsChanged()
{
    if (scene())
    {
        scene()->setModified(true);
    }

    if (toPlainText().isEmpty())
    {
        setTextWidth(textWidth());
    }
}

void UBGraphicsTextItem::insertImage(QString src)
{
    //retour chariot avant de placer l'image
    textCursor().insertText("\n");

    QImage img = QImage(src);
    QString fileExtension = UBFileSystemUtils::extension(src);
    QString filename = UBPersistenceManager::imageDirectory + "/" + QUuid::createUuid() + "." + fileExtension;
    QString dest = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + filename;

    if (!UBFileSystemUtils::copy(src, dest, true))
        qDebug() << "UBFileSystemUtils::copy error";

    document()->addResource(QTextDocument::ImageResource, QUrl(filename), img);
    textCursor().insertImage(filename);
}

void UBGraphicsTextItem::insertTable(const int lines, const int columns)
{
    QTextCursor cursor = textCursor();

    QTextTableFormat format;
    format.clearColumnWidthConstraints();
    format.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    QTextLength t = QTextLength(QTextLength::PercentageLength, 100/columns);
    QVector<QTextLength> v;
    for (int i=0; i< columns; i++)
        v.push_back(t);
    format.setColumnWidthConstraints(v);
    format.merge(cursor.charFormat());
    cursor.insertTable(lines,columns,format);    
}

void UBGraphicsTextItem::setBackgroundColor(const QColor& color)
{
    QTextBlockFormat format;
    format.setBackground(QBrush(color));

    QTextCursor cursor = textCursor();   

    QTextTable* t = cursor.currentTable();
    int firstRow,numRows,firstCol,numCols;
    cursor.selectedTableCells(&firstRow,&numRows,&firstCol,&numCols);
    if (firstRow < 0)
    {
        if (t)
        {
            QTextTableCell c = t->cellAt(cursor);
            QTextCharFormat format;
            format.setBackground(QBrush(color));
            c.setFormat(format);
        }
        else
            mBackgroundColor = color;
    }
    else
    {
        for (int i = 0; i < numRows; i++)
        {
            for (int j = 0; j < numCols; j++)
            {
                QTextTableCell c = t->cellAt(firstRow+i,firstCol+j);
                QTextCharFormat format;
                format.setBackground(QBrush(color));
                c.setFormat(format);
            }
        }
    }

}

void UBGraphicsTextItem::setForegroundColor(const QColor& color)
{
    QTextBlockFormat format;
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
}

void UBGraphicsTextItem::setAlignmentToLeft()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignLeft);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
}

void UBGraphicsTextItem::setAlignmentToCenter()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
}

void UBGraphicsTextItem::setAlignmentToRight()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignRight);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);    
    setFocus();
}

UBGraphicsScene* UBGraphicsTextItem::scene()
{
    return static_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsTextItem::resize(qreal w, qreal h)
{
    setTextWidth(w);
    setTextHeight(h);

    if (Delegate())
    {
        Delegate()->positionHandles();
        UBGraphicsTextItemDelegate* textDelegate = dynamic_cast<UBGraphicsTextItemDelegate*>(Delegate());
        if (textDelegate)
        {
            QSize tablePaletteSize = textDelegate->linkPalette()->size();
            textDelegate->tablePalette()->setPos(QPoint((w-tablePaletteSize.width())/2, (h-tablePaletteSize.height())/2));
            textDelegate->linkPalette()->setPos(QPoint((w-tablePaletteSize.width())/2, (h-tablePaletteSize.height())/2));
            textDelegate->cellPropertiesPalette()->setPos(QPoint((w-tablePaletteSize.width())/2, (h-tablePaletteSize.height())/2));
        }
    }
}

/*
void UBGraphicsTextItem::setSelected(bool selected)
{
    UBGraphicsTextItemDelegate* textDelegate = dynamic_cast<UBGraphicsTextItemDelegate*>(Delegate());
    if (textDelegate)
    {
        if (selected)
            textDelegate->tablePalette()->show();
        else
            textDelegate->tablePalette()->hide();
    }

    QGraphicsTextItem::setSelected(selected);
}
*/

QSizeF UBGraphicsTextItem::size() const
{
    return QSizeF(textWidth(), textHeight());
}

void UBGraphicsTextItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}


void UBGraphicsTextItem::undoCommandAdded()
{
    emit textUndoCommandAdded(this);
}


void UBGraphicsTextItem::documentSizeChanged(const QSizeF & newSize)
{
    resize(newSize.width(), newSize.height());
}

//issue 1554 - NNE - 20131009
void UBGraphicsTextItem::activateTextEditor(bool activateTextEditor)
{
    qDebug() << textInteractionFlags();

    this->isActivatedTextEditor = activateTextEditor;

    if(!activateTextEditor){
        setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextBrowserInteraction);
    }else{
        setTextInteractionFlags(Qt::TextEditorInteraction | Qt::TextBrowserInteraction);
    }

    qDebug() <<  textInteractionFlags();
}
//issue 1554 - NNE - 20131009 : END

//issue 1539 - NNE - 20131018
void UBGraphicsTextItem::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Paste)){
        UBTextTools::cleanHtmlClipboard();
    }

    QGraphicsTextItem::keyPressEvent(event);
}
//issue 1539 - NNE - 20131018 : END

//issue 1539 - NNE - 20131211
void UBGraphicsTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    UBTextTools::cleanHtmlClipboard();
    QGraphicsTextItem::contextMenuEvent(event);
}
