#include "domain/UBGraphicsDelegateControls.h"

#include "core/UBSettings.h"

DelegateButton::DelegateButton(const QString & fileName, QGraphicsItem* pDelegated, QGraphicsItem * parent, Qt::WindowFrameSection section)
: QGraphicsSvgItem(fileName, parent)
, mDelegated(pDelegated)
, mIsTransparentToMouseEvent(false)
, mButtonAlignmentSection(section)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));
    setCacheMode(QGraphicsItem::NoCache); /* because of SANKORE-1017: this allows pixmap to be refreshed when grabbing window, thus teacher screen is synchronized with main screen. */
}

DelegateButton::~DelegateButton()
{
    // NOOP
}

void DelegateButton::setFileName(const QString & fileName)
{
    QGraphicsSvgItem::setSharedRenderer(new QSvgRenderer (fileName, this));
}


void DelegateButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{   
    if (!shouldPaint())
        return;

    QGraphicsSvgItem::paint(painter, option, widget);
}

void DelegateButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // make sure delegate is selected, to avoid control being hidden
    mPressedTime = QTime::currentTime();

    event->setAccepted(!mIsTransparentToMouseEvent);
}

void DelegateButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    int timeto = qAbs(QTime::currentTime().msecsTo(mPressedTime));

    if (timeto < UBSettings::longClickInterval) {
        emit clicked();
    } else {
        emit longClicked();
    }

    event->setAccepted(!mIsTransparentToMouseEvent);
}

