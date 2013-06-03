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



#ifndef UBTHUMBNAILWIDGET_H_
#define UBTHUMBNAILWIDGET_H_

#include <QtGui>
#include <QtSvg>
#include <QTime>
#include <QGraphicsSceneHoverEvent>

#include "frameworks/UBCoreGraphicsScene.h"
#include "core/UBSettings.h"
#include "domain/UBItem.h"

#define STARTDRAGTIME   1000000
#define BUTTONSIZE      48
#define BUTTONSPACING   5

class UBDocumentProxy;
class UBThumbnailTextItem;
class UBThumbnail;

class UBThumbnailWidget : public QGraphicsView
{
    Q_OBJECT

    public:
        UBThumbnailWidget(QWidget* parent);
        virtual ~UBThumbnailWidget();

        QList<QGraphicsItem*> selectedItems();
        void selectItemAt(int pIndex, bool extend = false);
        void unselectItemAt(int pIndex);

        qreal thumbnailWidth()
        {
            return mThumbnailWidth;
        }

        void setBackgroundBrush(const QBrush& brush)
        {
            mThumbnailsScene.setBackgroundBrush(brush);
        }

    public slots:
        void setThumbnailWidth(qreal pThumbnailWidth);
        void setSpacing(qreal pSpacing);
        virtual void setGraphicsItems(const QList<QGraphicsItem*>& pGraphicsItems, const QList<QUrl>& pItemPaths, const QStringList pLabels = QStringList(), const QString& pMimeType = QString(""));
        void refreshScene();
        void sceneSelectionChanged();

    signals:
        void resized();
        void selectionChanged();
        void mouseDoubleClick(QGraphicsItem* item, int index);
        void mouseClick(QGraphicsItem* item, int index);


    protected:
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void resizeEvent(QResizeEvent * event);
        void mouseDoubleClickEvent(QMouseEvent * event);

        virtual void keyPressEvent(QKeyEvent *event);
        virtual void focusInEvent(QFocusEvent *event);

        QList<QGraphicsItem*> mGraphicItems;
        QList<UBThumbnailTextItem*> mLabelsItems;
        QPointF mMousePressScenePos;
        QPoint mMousePressPos;

    protected:
        qreal spacing() { return mSpacing; }
        QList<QUrl> mItemsPaths;
        QStringList mLabels;
        bool bSelectionInProgress;
        bool bCanDrag;       

    private:
        void selectAll();
        void selectItems(int startIndex, int count);
        int rowCount() const;
        int columnCount() const;

        static bool thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2);

        void deleteLasso();

        UBCoreGraphicsScene mThumbnailsScene;

        QString mMimeType;

        QPointF prevMoveMousePos;

        qreal mThumbnailWidth;
        qreal mThumbnailHeight;
        qreal mSpacing;

        UBThumbnail *mLastSelectedThumbnail;
        int mSelectionSpan;
        QRectF mPrevLassoRect;
        QGraphicsRectItem *mLassoRectItem;
        QSet<QGraphicsItem*> mSelectedThumbnailItems;
        QSet<QGraphicsItem*> mPreviouslyIncrementalSelectedItemsX;
        QSet<QGraphicsItem*> mPreviouslyIncrementalSelectedItemsY;
        QTime mClickTime;
};


class UBThumbnail
{
    public:
        UBThumbnail();

        virtual ~UBThumbnail();

        QStyleOptionGraphicsItem muteStyleOption(const QStyleOptionGraphicsItem *option)
        {
            // Never draw the rubber band, we draw our custom selection with the DelegateFrame
            QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
            styleOption.state &= ~QStyle::State_Selected;

            return styleOption;
        }

         virtual void itemChange(QGraphicsItem *item, QGraphicsItem::GraphicsItemChange change, const QVariant &value);

        UBThumbnailTextItem *label(){return mLabel;}
        void setLabel(UBThumbnailTextItem *label){mLabel = label;}
        int column() { return mColumn; }
        void setColumn(int column) { mColumn = column; }
        int row() { return mRow; }
        void setRow(int row) { mRow = row; }

    protected:
        QGraphicsRectItem *mSelectionItem;
        private:
        bool mAddedToScene;

        int mColumn;
        int mRow;

        UBThumbnailTextItem *mLabel;
};


class UBThumbnailSvg : public QGraphicsSvgItem, public UBThumbnail
{
    public:
        UBThumbnailSvg(const QString& path)
            : QGraphicsSvgItem(path)
        {
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        virtual ~UBThumbnailSvg()
        {
            // NOOP
        }

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
        {
            QStyleOptionGraphicsItem styleOption = UBThumbnail::muteStyleOption(option);
            QGraphicsSvgItem::paint(painter, &styleOption, widget);
        }

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
        {
            UBThumbnail::itemChange(this, change, value);
            return QGraphicsSvgItem::itemChange(change, value);
        }

};


class UBThumbnailPixmap : public QGraphicsPixmapItem, public UBThumbnail
{
    public:
        UBThumbnailPixmap(const QPixmap& pix)
            : QGraphicsPixmapItem(pix)
        {
            setTransformationMode(Qt::SmoothTransformation); // UB 4.3 may be expensive -- make configurable
            setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        virtual ~UBThumbnailPixmap()
        {
            // NOOP
        }

        virtual QPainterPath shape () const
        {
            QPainterPath path;
            path.addRect(boundingRect());
            return path;
        }


        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
        {
            QStyleOptionGraphicsItem styleOption = UBThumbnail::muteStyleOption(option);
            QGraphicsPixmapItem::paint(painter, &styleOption, widget);
        }

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
        {
            UBThumbnail::itemChange(this, change, value);
            return QGraphicsPixmapItem::itemChange(change, value);
        }
};


class UBSceneThumbnailPixmap : public UBThumbnailPixmap
{
    public:
        UBSceneThumbnailPixmap(const QPixmap& pix, UBDocumentProxy* proxy, int pSceneIndex)
            : UBThumbnailPixmap(pix)
            , mProxy(proxy)
            , mSceneIndex(pSceneIndex)
        {
            // NOOP
        }

        virtual ~UBSceneThumbnailPixmap();

        UBDocumentProxy* proxy()
        {
            return mProxy;
        }

        int sceneIndex()
        {
            return mSceneIndex;
        }

    private:
        UBDocumentProxy* mProxy;
        int mSceneIndex;
};

class UBSceneThumbnailNavigPixmap : public UBSceneThumbnailPixmap
{
    public:
        UBSceneThumbnailNavigPixmap(const QPixmap& pix, UBDocumentProxy* proxy, int pSceneIndex);
        ~UBSceneThumbnailNavigPixmap();

    protected:
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);

    private:
        void updateButtonsState();
        void deletePage();
        void duplicatePage();
        void moveUpPage();
        void moveDownPage();
        void stickPageOnPreviousViews();

        bool bButtonsVisible;
        bool bCanDelete;
        bool bCanMoveUp;
        bool bCanMoveDown;
        bool bCanDuplicate;
        bool bCanStickOnPreviousViews;
};

class UBThumbnailVideo : public UBThumbnailPixmap
{
    public:
        UBThumbnailVideo(const QUrl &path)
            : UBThumbnailPixmap(QPixmap(":/images/movie.svg"))
            , mPath(path)
        {
            // NOOP
        }

        virtual ~UBThumbnailVideo()
        {
            // NOOP
        }

        QUrl path()
        {
            return mPath;
        }

    private:

        QUrl mPath;
};

class UBThumbnailTextItem : public QGraphicsTextItem
{
    public:
        UBThumbnailTextItem(const QString& text)
            : QGraphicsTextItem(text)
            , mUnelidedText(text)
            , mIsHighlighted(true)
        {
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        QRectF boundingRect() const { return QRectF(QPointF(0.0, 0.0), QSize(mWidth, QFontMetricsF(font()).height() + 5));}

        void setWidth(qreal pWidth)
        {
                if (mWidth != pWidth)
                {
                        prepareGeometryChange();
                        mWidth = pWidth;
                        computeText();
                }
        };

        qreal width() {return mWidth;}

        void highlight(bool enable = true)
        {
            mIsHighlighted = enable;
            computeText();
        }

        void computeText()
        {
            QFontMetricsF fm(font());
            QString elidedText = fm.elidedText(mUnelidedText, Qt::ElideRight, mWidth);

            if (mIsHighlighted)
            {
                setHtml("<span style=\"color: #6682b5\">" + elidedText + "</span>");
            }
            else
            {
                setPlainText(elidedText);
            }
        }

    private:
        qreal mWidth;
        QString mUnelidedText;
        bool mIsHighlighted;
};

class UBImgTextThumbnailElement
{
private:
	UBSceneThumbnailNavigPixmap* thumbnail;
	UBThumbnailTextItem* caption;
	int border;

public:
	UBImgTextThumbnailElement(UBSceneThumbnailNavigPixmap* thumb, UBThumbnailTextItem* text): border(0)
	{
		this->thumbnail = thumb;
		this->caption = text;
	}

	UBSceneThumbnailNavigPixmap* getThumbnail() const { return this->thumbnail; }
	void setThumbnail(UBSceneThumbnailNavigPixmap* newGItem) { this->thumbnail = newGItem; }

	UBThumbnailTextItem* getCaption() const { return this->caption; }
	void setCaption(UBThumbnailTextItem* newcaption) { this->caption = newcaption; }

	void Place(int row, int col, qreal width, qreal height);

	int getBorder() const { return this->border; }
	void setBorder(int newBorder) { this->border = newBorder; }
};


#endif /* UBTHUMBNAILWIDGET_H_ */
