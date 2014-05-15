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
#include <QtSvg>

#include "core/UBApplication.h"
#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "gui/UBResources.h"
#include "gui/UBMainWindow.h"

#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsProxyWidget.h"

#include "core/UBSettings.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"

const int UBGraphicsTextItemDelegate::sMinPixelSize = 8;
const int UBGraphicsTextItemDelegate::sMinPointSize = 8;

UBGraphicsTextItemDelegate::UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated,0, parent, true)
    , mLastFontPixelSize(-1)
    , delta(5)
    , mTablePalette(new UBCreateTablePalette())
    , mLinkPalette(new UBCreateHyperLinkPalette())
{
    UBGraphicsProxyWidget* w = UBApplication::boardController->activeScene()->addWidget(mTablePalette);
    w->setParentItem(delegated());
    w->hide();

    UBGraphicsProxyWidget* w2 = UBApplication::boardController->activeScene()->addWidget(mLinkPalette);
    w2->setParentItem(delegated());
    w2->hide();

    delegated()->setData(UBGraphicsItemData::ItemEditable, QVariant(true));
    delegated()->setPlainText("");

    QTextCursor curCursor = delegated()->textCursor();
    QTextCharFormat format;
    QFont font(createDefaultFont());
    font.setPointSize(UBSettings::settings()->fontPointSize());

    format.setFont(font);
    curCursor.mergeCharFormat(format);
    delegated()->setTextCursor(curCursor);
    delegated()->setFont(font);

    delegated()->adjustSize();
    delegated()->contentsChanged();

    mTablePalette->move(delegated()->boundingRect().width()/2.0, 0 );
    mLinkPalette->move(delegated()->boundingRect().width()/2.0, 0 );

    connect(mTablePalette, SIGNAL(validationRequired()), this, SLOT(insertTable()));
    connect(mLinkPalette, SIGNAL(validationRequired()), this, SLOT(insertLink()));
}

UBGraphicsTextItemDelegate::~UBGraphicsTextItemDelegate()
{

}

QFont UBGraphicsTextItemDelegate::createDefaultFont()
{
    QTextCharFormat textFormat;

    QString fFamily = UBSettings::settings()->fontFamily();
    if (!fFamily.isEmpty())
        textFormat.setFontFamily(fFamily);

    bool bold = UBSettings::settings()->isBoldFont();
    if (bold)
        textFormat.setFontWeight(QFont::Bold);

    bool italic = UBSettings::settings()->isItalicFont();
    if (italic)
        textFormat.setFontItalic(true);

    QFont font(fFamily, -1, bold ? QFont::Bold : -1, italic);
    int pointSize = UBSettings::settings()->fontPointSize();
    if (pointSize > 0) {
        font.setPointSize(pointSize);
    }

    return font;
}

void UBGraphicsTextItemDelegate::buildButtons()
{
    UBGraphicsItemDelegate::buildButtons();

    mFontButton = new DelegateButton(":/images/font.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontBoldButton = new DelegateButton(":/images/bold.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontItalicButton = new DelegateButton(":/images/italic.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontUnderlineButton = new DelegateButton(":/images/underline.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mColorButton = new DelegateButton(":/images/color.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mDecreaseSizeButton = new DelegateButton(":/images/minus.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mIncreaseSizeButton = new DelegateButton(":/images/plus.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mBackgroundColorButton = new DelegateButton(":/images/color.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mTableButton = new DelegateButton(":/images/roundeRrectangle.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mLeftAlignmentButton = new DelegateButton(":/images/resizeLeft.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mCenterAlignmentButton = new DelegateButton(":/images/resizeTop.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mRightAlignmentButton = new DelegateButton(":/images/resizeRight.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mCodeButton = new DelegateButton(":/images/code.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mListButton = new DelegateButton(":/images/code.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mAddIndentButton = new DelegateButton(":/images/code.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mRemoveIndentButton = new DelegateButton(":/images/code.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mHyperLinkButton = new DelegateButton(":/images/plus.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    connect(mFontButton, SIGNAL(clicked(bool)), this, SLOT(pickFont()));
    connect(mFontBoldButton, SIGNAL(clicked()), this, SLOT(setFontBold()));
    connect(mFontItalicButton, SIGNAL(clicked()), this, SLOT(setFontItalic()));
    connect(mFontUnderlineButton, SIGNAL(clicked()), this, SLOT(setFontUnderline()));
    connect(mColorButton, SIGNAL(clicked(bool)), this, SLOT(pickColor()));    
    connect(mDecreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(decreaseSize()));
    connect(mIncreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(increaseSize()));
    connect(mBackgroundColorButton, SIGNAL(clicked(bool)), this, SLOT(pickBackgroundColor()));    
    connect(mTableButton, SIGNAL(clicked(bool)), this, SLOT(setTableSize()));
    connect(mLeftAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToLeft()));
    connect(mCenterAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToCenter()));
    connect(mRightAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToRight()));
    connect(mCodeButton, SIGNAL(clicked(bool)), this, SLOT(alternHtmlMode()));
    connect(mListButton, SIGNAL(clicked(bool)), this, SLOT(insertList()));
    connect(mAddIndentButton, SIGNAL(clicked(bool)), this, SLOT(addIndent()));
    connect(mRemoveIndentButton, SIGNAL(clicked(bool)), this, SLOT(removeIndent()));
    connect(mHyperLinkButton, SIGNAL(clicked(bool)), this, SLOT(addLink()));

    QList<QGraphicsItem*> itemsOnToolBar;
    itemsOnToolBar << mFontButton << mFontBoldButton << mFontItalicButton << mFontUnderlineButton
                   << mColorButton << mDecreaseSizeButton << mIncreaseSizeButton
                   << mBackgroundColorButton << mTableButton << mLeftAlignmentButton
                   << mCenterAlignmentButton << mRightAlignmentButton << mCodeButton
                   << mListButton << mAddIndentButton << mRemoveIndentButton << mHyperLinkButton;
    mToolBarItem->setItemsOnToolBar(itemsOnToolBar);
    mToolBarItem->setShifting(true);
    mToolBarItem->setVisibleOnBoard(true);
}

void UBGraphicsTextItemDelegate::contentsChanged()
{
    positionHandles();
    delegated()->contentsChanged();
}

// This method is used to filter the available fonts. Only the web-compliant fonts
// will remain in the font list.
void UBGraphicsTextItemDelegate::customize(QFontDialog &fontDialog)
{
    fontDialog.setOption(QFontDialog::DontUseNativeDialog);

    if (UBSettings::settings()->isDarkBackground()) {
        fontDialog.setStyleSheet("background-color: white;");
    }

    QListView *fontNameListView;
    QList<QListView*> listViews = fontDialog.findChildren<QListView*>();
    if (listViews.count() > 0)
    {
        fontNameListView = listViews.at(0);
        foreach (QListView* listView, listViews)
        {
            if (listView->pos().x() < fontNameListView->pos().x())
                fontNameListView = listView;
        }
    }
    if (fontNameListView)
    {
        QStringListModel *stringListModel = dynamic_cast<QStringListModel*>(fontNameListView->model());
        if (stringListModel)
        {
            QStringList dialogFontNames = stringListModel->stringList();
            QStringList safeWebFontNames;
            safeWebFontNames.append("Arial");
            safeWebFontNames.append("Arial Black");
            safeWebFontNames.append("Comic Sans MS");
            safeWebFontNames.append("Courier New");
            safeWebFontNames.append("Georgia");
            safeWebFontNames.append("Impact");
            safeWebFontNames.append("Times New Roman");
            safeWebFontNames.append("Trebuchet MS");
            safeWebFontNames.append("Verdana");

            QStringList customFontList =  UBResources::resources()->customFontList();
            int index = 0;
            foreach (QString dialogFontName, dialogFontNames){
                if (safeWebFontNames.contains(dialogFontName, Qt::CaseInsensitive) || customFontList.contains(dialogFontName, Qt::CaseSensitive))
                    index++;
                else
                    stringListModel->removeRow(index);
            }
        }
    }
    QList<QComboBox*> comboBoxes = fontDialog.findChildren<QComboBox*>();
    if (comboBoxes.count() > 0)
        comboBoxes.at(0)->setEnabled(false);
}


void UBGraphicsTextItemDelegate::pickFont()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QFontDialog fontDialog(delegated()->textCursor().charFormat().font(), mDelegated->scene()->views().at(0));
        customize(fontDialog);

        if (fontDialog.exec())
        {
            QFont selectedFont = fontDialog.selectedFont();
            UBSettings::settings()->setFontFamily(selectedFont.family());
            UBSettings::settings()->setBoldFont(selectedFont.bold());
            UBSettings::settings()->setItalicFont(selectedFont.italic());
            UBSettings::settings()->setFontPointSize(selectedFont.pointSize());

            //setting format for selected item
            QTextCursor curCursor = delegated()->textCursor();
            QTextCharFormat format;
            format.setFont(selectedFont);
            curCursor.mergeCharFormat(format);

            delegated()->setTextCursor(curCursor);
            delegated()->setFont(selectedFont);
            delegated()->setSelected(true);
//          disabled and replaced by the next line because of not optimum result (text splits to two lines when that is not necessary)
    //          delegated()->adjustSize();
                delegated()->resize(delegated()->document()->idealWidth(), delegated()->size().height());
                delegated()->contentsChanged();
            }
        }
    }

void UBGraphicsTextItemDelegate::setFontBold()
{
    QTextCharFormat format;
    QTextCursor cursor = delegated()->textCursor();
    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();

    if (anchorPos >= cursorPos)
        std::swap(cursorPos, anchorPos);

    format.setFontWeight(cursor.charFormat().fontWeight() == QFont::Normal ? QFont::Bold : QFont::Normal);

    if (cursor.selectedText().length() == 0)
    {
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(format);
        cursor.clearSelection();
    }
    else
        cursor.mergeCharFormat(format);

    if (anchorPos >= cursorPos)
    {
        cursor.setPosition(cursorPos, QTextCursor::MoveAnchor);
        cursor.setPosition(anchorPos, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(anchorPos, QTextCursor::MoveAnchor);
        cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    }

    delegated()->setFocus();
    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::setFontItalic()
{
    QTextCharFormat format;
    QTextCursor cursor = delegated()->textCursor();

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();
    if (anchorPos >= cursorPos)
        std::swap(cursorPos, anchorPos);

    format.setFontItalic(!cursor.charFormat().fontItalic());
    if (cursor.selectedText().length() == 0)
    {
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(format);
        cursor.clearSelection();
    }
    else
        cursor.mergeCharFormat(format);

    cursor.setPosition(anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    delegated()->setFocus();
    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::setFontUnderline()
{
    QTextCharFormat format;
    QTextCursor cursor = delegated()->textCursor();

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();
    if (anchorPos >= cursorPos)
        std::swap(cursorPos, anchorPos);

    format.setFontUnderline(!cursor.charFormat().fontUnderline());
    if (cursor.selectedText().length() == 0)
    {
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(format);
        cursor.clearSelection();
    }
    else
        cursor.mergeCharFormat(format);

    cursor.setPosition(anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    delegated()->setFocus();
    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::pickColor()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QColorDialog colorDialog(delegated()->defaultTextColor(), mDelegated->scene()->views().at(0));
        colorDialog.setWindowTitle(tr("Text Color"));
        if (UBSettings::settings()->isDarkBackground())
        {
            colorDialog.setStyleSheet("background-color: white;");
        }

        if (colorDialog.exec())
        {
            QColor selectedColor = colorDialog.selectedColor();
            delegated()->setDefaultTextColor(selectedColor);
            QTextCursor curCursor = delegated()->textCursor();
            QTextCharFormat format;
            format.setForeground(QBrush(selectedColor));
            curCursor.mergeCharFormat(format);
            delegated()->setTextCursor(curCursor);

            UBGraphicsTextItem::lastUsedTextColor = selectedColor;

            delegated()->setSelected(true);            
            delegated()->contentsChanged();
            delegated()->setFocus();
        }
    }
}

void UBGraphicsTextItemDelegate::pickBackgroundColor()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QColorDialog colorDialog(delegated()->defaultTextColor(), mDelegated->scene()->views().at(0));
        colorDialog.setWindowTitle(tr("Background Color"));
        if (UBSettings::settings()->isDarkBackground())
        {
            colorDialog.setStyleSheet("background-color: white;");
        }

        if (colorDialog.exec())
        {
            QColor selectedColor = colorDialog.selectedColor();
            delegated()->setBackgroundColor(selectedColor);
            delegated()->setSelected(true);
            delegated()->contentsChanged();
            delegated()->setFocus();
        }
    }
}

void UBGraphicsTextItemDelegate::insertTable()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {                
        delegated()->insertTable(mTablePalette->lines(), mTablePalette->columns());
        mTablePalette->hide();
    }
}

void UBGraphicsTextItemDelegate::addIndent()
{
    QTextCursor cursor = delegated()->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;
    QTextList *list = cursor.currentList();

    if (list)
    {
        QList<QTextBlock> newList;
        int firstSelectedBlock = list->itemNumber(cursor.block());
        int count = list->count();

        //get list's items which have to be indented
        for (int i=firstSelectedBlock; i < count; i++)
            newList.push_back(list->item(i));


        //create new format
        listFmt = list->format();
        QTextListFormat::Style style = static_cast<QTextListFormat::Style>((listFmt.style() - 1) % 4);
        if (style == QTextListFormat::ListStyleUndefined)
            style = QTextListFormat::ListDisc;
        listFmt.setStyle(style);
        listFmt.setIndent(listFmt.indent()+1);

        //create new sub-list with new format
        QTextList* theList = cursor.createList(listFmt);
        for (int i = 0; i < count-firstSelectedBlock; i++)
            theList->add(newList.at(i));
    }
    else
    {
        blockFmt.setIndent(blockFmt.indent()+1);
        cursor.setBlockFormat(blockFmt);
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::removeIndent()
{
    QTextCursor cursor = delegated()->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;

    QTextList *list = cursor.currentList();

    if (list)
    {
        //create new format
        listFmt = list->format();
        listFmt.setStyle(previousStyle(listFmt.style()));
        listFmt.setIndent(listFmt.indent()-1);

        //create new sub-list with new format
        QTextList* theList = cursor.createList(listFmt);
        theList->add(cursor.block());
    }
    else
    {
        blockFmt.setIndent(blockFmt.indent()-1);
        cursor.setBlockFormat(blockFmt);
    }

    delegated()->setFocus();
}


void UBGraphicsTextItemDelegate::insertList()
{
    QTextCursor cursor = delegated()->textCursor();

    QTextListFormat::Style style = QTextListFormat::ListDisc;

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;

    QTextList *list = cursor.currentList();

    if (list)
    {
        QTextListFormat listFormat;
        listFormat.setIndent(0);
        listFormat.setStyle(style);
        list->setFormat(listFormat);

        for( int i = list->count() - 1; i>=0 ; --i)
            list->removeItem(i);
    }
    else
    {
        listFmt.setIndent(blockFmt.indent()+1);
        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setTableSize()
{
    mTablePalette->show();
    mLinkPalette->hide();
}

void UBGraphicsTextItemDelegate::setAlignmentToLeft()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToLeft();        
    }
}

void UBGraphicsTextItemDelegate::setAlignmentToCenter()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToCenter();
    }
}

void UBGraphicsTextItemDelegate::setAlignmentToRight()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToRight();
    }
}

void UBGraphicsTextItemDelegate::addLink()
{
    mLinkPalette->show();
}

void UBGraphicsTextItemDelegate::insertLink()
{
    QString link = mLinkPalette->link();
    if (!link.startsWith("http://"))
        link = "http://" + link;

    if (!(mLinkPalette->text().isEmpty() || mLinkPalette->link().isEmpty()))
        delegated()->textCursor().insertHtml(QString("<a href=\"%1\">%2</a>").arg(link, mLinkPalette->text()));

    mLinkPalette->hide();
}

void UBGraphicsTextItemDelegate::alternHtmlMode()
{    
    if (!delegated()->htmlMode())
    {
        delegated()->setPlainText(delegated()->toHtml());

        QTextCursor cursor = delegated()->textCursor();
        QTextCharFormat format;
        QFont font;

        font.setFamily("Arial");
        font.setPointSize(12);

        format.setFont(font);

        cursor.select(QTextCursor::Document);
        cursor.setCharFormat(format);
        cursor.clearSelection();

        delegated()->setTextCursor(cursor);
        delegated()->setFont(font);
    }
    else
    {
        delegated()->setHtml(delegated()->toPlainText());
    }

    delegated()->setHtmlMode(!delegated()->htmlMode());
    changeDelegateButtonsMode(delegated()->htmlMode());
}

void UBGraphicsTextItemDelegate::decreaseSize()
{
    ChangeTextSize(-delta, changeSize);
}

void UBGraphicsTextItemDelegate::increaseSize()
{
   ChangeTextSize(delta, changeSize);
}

UBGraphicsTextItem* UBGraphicsTextItemDelegate::delegated()
{
    return static_cast<UBGraphicsTextItem*>(mDelegated);
}

void UBGraphicsTextItemDelegate::setEditable(bool editable)
{
    if (editable) {
        delegated()->setTextInteractionFlags(Qt::TextEditorInteraction);
        mDelegated->setData(UBGraphicsItemData::ItemEditable, QVariant(true));
    } else {
        QTextCursor cursor(delegated()->document());
        cursor.clearSelection();
        delegated()->setTextCursor(cursor);

        delegated()->setTextInteractionFlags(Qt::NoTextInteraction);
        mDelegated->setData(UBGraphicsItemData::ItemEditable, QVariant(false));
    }
}

void UBGraphicsTextItemDelegate::remove(bool canUndo)
{
    mTablePalette->hide();
    mLinkPalette->hide();
    UBGraphicsItemDelegate::remove(canUndo);
}

bool UBGraphicsTextItemDelegate::isEditable()
{
    return mDelegated->data(UBGraphicsItemData::ItemEditable).toBool();
}
void UBGraphicsTextItemDelegate::decorateMenu(QMenu *menu)
{
    UBGraphicsItemDelegate::decorateMenu(menu);

    mEditableAction = menu->addAction(tr("Editable"), this, SLOT(setEditable(bool)));
    mEditableAction->setCheckable(true);
    mEditableAction->setChecked(isEditable());

}
void UBGraphicsTextItemDelegate::updateMenuActionState()
{
    UBGraphicsItemDelegate::updateMenuActionState();
}

void UBGraphicsTextItemDelegate::positionHandles()
{
    UBGraphicsItemDelegate::positionHandles();

    if (mDelegated->isSelected() || (mDelegated->parentItem() && UBGraphicsGroupContainerItem::Type == mDelegated->parentItem()->type()))
    {
        if (mToolBarItem->isVisibleOnBoard())
        {
            qreal AntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
            mToolBarItem->setScale(AntiScaleRatio);
            QRectF toolBarRect = mToolBarItem->rect();
            toolBarRect.setWidth(delegated()->boundingRect().width()/AntiScaleRatio);
            mToolBarItem->setRect(toolBarRect);
            mToolBarItem->positionHandles();
            mToolBarItem->update();
            if (mToolBarItem->isShifting())
                mToolBarItem->setPos(0,-mToolBarItem->boundingRect().height()*AntiScaleRatio);
            else
                mToolBarItem->setPos(0, 0);

            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(mDelegated->parentItem());

            mToolBarItem->hide();
            if (mToolBarItem->parentItem())
            {
                if (group && group->getCurrentItem() == mDelegated && group->isSelected())
                    mToolBarItem->show();

                if (!group)
                     mToolBarItem->show();
            }

        }
    }
    else
    {
        mToolBarItem->hide();
    }
}

void UBGraphicsTextItemDelegate::changeDelegateButtonsMode(bool htmlMode)
{
    if (htmlMode)
    {
        mFontButton->setEnabled(false);
        mFontBoldButton->setEnabled(false);
        mFontItalicButton->setEnabled(false);
        mFontUnderlineButton->setEnabled(false);
        mColorButton->setEnabled(false);
        //mDecreaseSizeButton->setEnabled(false);
        //mIncreaseSizeButton->setEnabled(false);
        mBackgroundColorButton->setEnabled(false);
        mTableButton->setEnabled(false);
        mLeftAlignmentButton->setEnabled(false);
        mCenterAlignmentButton->setEnabled(false);
        mRightAlignmentButton->setEnabled(false);
        mListButton->setEnabled(false);
    }
    else
    {
        mFontButton->setEnabled(true);
        mFontBoldButton->setEnabled(true);
        mFontItalicButton->setEnabled(true);
        mFontUnderlineButton->setEnabled(true);
        mColorButton->setEnabled(true);
        //mDecreaseSizeButton->setEnabled(true);
        //mIncreaseSizeButton->setEnabled(true);
        mBackgroundColorButton->setEnabled(true);
        mTableButton->setEnabled(true);
        mLeftAlignmentButton->setEnabled(true);
        mCenterAlignmentButton->setEnabled(true);
        mRightAlignmentButton->setEnabled(true);
        mListButton->setEnabled(true);
    }
}

void UBGraphicsTextItemDelegate::ChangeTextSize(qreal factor, textChangeMode changeMode)
{    
    if (scaleSize == changeMode)
    {
        if (1 == factor)
            return;
    }
    else
    if (0 == factor)
        return;

    UBGraphicsTextItem *item = dynamic_cast<UBGraphicsTextItem*>(delegated());

    if (item && (QString() == item->toPlainText()))
        return;

    QTextCursor cursor = delegated()->textCursor();

    QTextCharFormat textFormat;

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();

    if (0 == anchorPos-cursorPos)
    {
        cursor.setPosition (0, QTextCursor::MoveAnchor);
        cursor.setPosition (cursor.document()->characterCount()-1, QTextCursor::KeepAnchor);
    }

    int startPos = qMin(cursor.anchor(), cursor.position());
    int endPos = qMax(cursor.anchor(), cursor.position());

    QFont curFont;
    bool bEndofTheSameBlock;
    int iBlockLen;
    int iPointSize;
    int iNextPointSize;
    int iCursorPos = startPos;
    bool isUnderline = false;
    bool isBold = false;
    bool isItalic = false;
    QFont nextFont;
    bool nextIsUnderline = false;
    bool nextIsItalic = false;
    bool nextIsBold = false;

    // we search continuous blocks of the text with the same PointSize and allpy new settings for them.
    cursor.setPosition (startPos, QTextCursor::MoveAnchor);
    while(iCursorPos < endPos)
    {
        bEndofTheSameBlock = false;
        iBlockLen = 0;

        cursor.setPosition (iCursorPos+1, QTextCursor::KeepAnchor);
        iPointSize = cursor.charFormat().font().pointSize();
        curFont = cursor.charFormat().font();        
        isUnderline = cursor.charFormat().fontUnderline();
        isItalic = cursor.charFormat().fontItalic();
        isBold = cursor.charFormat().fontWeight() == QFont::Normal;
        cursor.setPosition (iCursorPos, QTextCursor::KeepAnchor);

        do
        {
            iBlockLen++;

            cursor.setPosition (iCursorPos+iBlockLen+1, QTextCursor::KeepAnchor);
            iNextPointSize = cursor.charFormat().font().pointSize();
            nextFont = cursor.charFormat().font();
            nextIsUnderline = cursor.charFormat().fontUnderline();
            nextIsItalic = cursor.charFormat().fontItalic();
            nextIsBold = cursor.charFormat().fontWeight() == QFont::Normal;

            cursor.setPosition (iCursorPos+iBlockLen, QTextCursor::KeepAnchor);

            if ((isBold != nextIsBold)
                || (isItalic != nextIsItalic)
                || (isUnderline != nextIsUnderline)
                || (curFont != nextFont)
                || (iCursorPos+iBlockLen >= endPos))
                bEndofTheSameBlock = true;

        }while(!bEndofTheSameBlock);

        //setting new parameters
        int iNewPointSize = (changeSize == changeMode) ? (iPointSize + factor) : (iPointSize * factor);

        curFont.setPointSize( (iNewPointSize > 0)?iNewPointSize:1);
        textFormat.setFont(curFont);
        cursor.mergeCharFormat(textFormat);

        iCursorPos += iBlockLen;
        cursor.setPosition (iCursorPos, QTextCursor::MoveAnchor);
    }

    UBSettings::settings()->setFontPointSize(iPointSize);
    //returning initial selection
    cursor.setPosition (anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition (cursorPos, QTextCursor::KeepAnchor);

    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::scaleTextSize(qreal multiplyer)
{
    ChangeTextSize(multiplyer, scaleSize);
}

QVariant UBGraphicsTextItemDelegate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        if (delegated()->isSelected())
        {
            QTextCursor c = delegated()->textCursor();
            if (c.hasSelection())
            {
                c.clearSelection();
                delegated()->setTextCursor(c);
            }
        }
    }
    return UBGraphicsItemDelegate::itemChange(change, value);
}

UBCreateHyperLinkPalette *UBGraphicsTextItemDelegate::linkPalette()
{
    return mLinkPalette;
}

UBCreateTablePalette* UBGraphicsTextItemDelegate::tablePalette()
{
    return mTablePalette;
}
