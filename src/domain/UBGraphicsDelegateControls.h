#ifndef UBGRAPHICS_DELEGATE_CONTROLS_H
#define UBGRAPHICS_DELEGATE_CONTROLS_H

#include <QtGui>
#include <QtSvg>
#include <QMimeData>

#include "domain/UBItem.h"
#include "core/UB.h"

class UBGraphicsMediaItem;

class DelegateButton: public QGraphicsSvgItem, public UBGraphicsItem
{
    Q_OBJECT

public:
    DelegateButton(const QString & fileName, QGraphicsItem* pDelegated, QGraphicsItem * parent = 0, Qt::WindowFrameSection section = Qt::TopLeftSection);

    virtual ~DelegateButton();

    enum { Type = UBGraphicsItemType::DelegateButtonType };
    virtual int type() const { return Type; }

    void setTransparentToMouseEvent(bool tr)
    {
        mIsTransparentToMouseEvent = tr;
    }

    void setFileName(const QString & fileName);

    void setSection(Qt::WindowFrameSection section) {mButtonAlignmentSection =  section;}
    Qt::WindowFrameSection getSection() const {return mButtonAlignmentSection;}
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void modified();

private:

    QGraphicsItem* mDelegated;

    QTime mPressedTime;
    bool mIsTransparentToMouseEvent;
    Qt::WindowFrameSection mButtonAlignmentSection;

signals:
    void clicked (bool checked = false);
    void longClicked();

};

/*
    Code of this class is copied from QT QLCDNumber class sources
    See src\gui\widgets\qlcdnumber.cpp for original code
*/
class MediaTimer: public QGraphicsRectItem
{
public:
    MediaTimer(QGraphicsItem * parent = 0);
    ~MediaTimer();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

    void display(const QString &str);

private:

    static const char* getSegments(char);
    void drawString(const QString& s, QPainter &, QBitArray * = 0, bool = true);
    void drawDigit(const QPoint &, QPainter &, int, char, char = ' ');
    void drawSegment(const QPoint &, char, QPainter &, int, bool = false);
    void addPoint(QPolygon&, const QPoint&);
    void internalSetString(const QString& s);
    void setNumDigits(int nDigits);

    static char segments [][8];

    int ndigits;
    QString digitStr;
    QBitArray points;
    double val;

    uint shadow : 1;
    uint smallPoint : 1;
};

class DelegateMediaControl: public QObject, public QGraphicsRectItem
{
    Q_OBJECT

    public:

        DelegateMediaControl(UBGraphicsMediaItem* pDelegated, QGraphicsItem * parent = 0);

        virtual ~DelegateMediaControl()
        {
            // NOOP
        }

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

        QPainterPath shape() const;

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void update();

        void positionHandles();
        void updateTicker(qint64 time);
        void totalTimeChanged(qint64 newTotalTime);
        QSizeF lcdAreaSize(){return mLCDTimerArea.size();}

    signals:
        void used();

    protected:
        void seekToMousePos(QPointF mousePos);

        UBGraphicsMediaItem* mDelegate;
        bool mDisplayCurrentTime;

        qint64 mCurrentTimeInMs;
        qint64 mTotalTimeInMs;

    private:
        int mStartWidth;
        int mSeecAreaBorderHeight;

        QRectF mSeecArea;
        QRectF mLCDTimerArea;

        MediaTimer *lcdTimer;
};

class UBGraphicsToolBarItem : public QGraphicsRectItem, public UBGraphicsItem, public QObject
{
    public:
        UBGraphicsToolBarItem(QGraphicsItem * parent = 0);
        virtual ~UBGraphicsToolBarItem() {;}

        bool isVisibleOnBoard() const { return mVisible; }
        void setVisibleOnBoard(bool visible) { mVisible = visible; }
        bool isShifting() const { return mShifting; }
        void setShifting(bool shifting) { mShifting = shifting; } 
        QList<QGraphicsItem*> itemsOnToolBar() const { return mItemsOnToolBar; }
        void setItemsOnToolBar(QList<QGraphicsItem*> itemsOnToolBar) { mItemsOnToolBar = itemsOnToolBar;}
        int minWidth() { return mMinWidth; }
        void positionHandles();
        void update();
        int getElementsPadding(){return mElementsPadding;}

    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

    private:
        bool mShifting;
        bool mVisible;
        int mMinWidth;
        int mInitialHeight;
        int mElementsPadding;
        QList<QGraphicsItem*> mItemsOnToolBar;
};



#endif