/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "core/UBApplication.h"
#include "globals/UBGlobals.h"
#include "UBMediaWidget.h"

/**
  * \brief Constructor
  * @param type as the media type
  * @param parent as the parent widget
  * @param name as the object name
  */
UBMediaWidget::UBMediaWidget(eMediaType type, QWidget *parent, const char *name):UBActionableWidget(parent, name)
  , mpMediaObject(NULL)
  , mpVideoWidget(NULL)
  , mpAudioOutput(NULL)
  , mpPlayStopButton(NULL)
  , mpPauseButton(NULL)
  , mpSlider(NULL)
  , mAutoUpdate(false)
  , mGeneratingThumbnail(false)
  , mBorder(5)
  , mpMediaContainer(NULL)
  , mpCover(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
  , mpPicture(NULL)
  , mpPreviewTitle(NULL)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());
    mVizMode = eVizualisationMode_Edit;
    mDragStarted = false;

    addAction(eAction_Close);
    mType = type;
    setLayout(&mLayout);

    mpPreviewTitle = new UBMediaTitle(type, this);
    mLayout.addWidget(mpPreviewTitle);
    mpPreviewTitle->setVisible(false);

    if(eMediaType_Video == type || eMediaType_Audio == type){
        mpPlayStopButton = new UBMediaButton(this);
        mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
        mpPauseButton = new UBMediaButton(this);
        mpPauseButton->setPixmap(QPixmap(":images/pause.svg"));
        mpPauseButton->setEnabled(false);
        mpSlider = new QSlider(this);
        mpSlider->setOrientation(Qt::Horizontal);
        mpSlider->setMinimum(0);
        mpSlider->setMaximum(0);

        mSeekerLayout.addWidget(mpPlayStopButton, 0);
        mSeekerLayout.addWidget(mpPauseButton, 0);
        mSeekerLayout.addWidget(mpSlider, 1);
        mSeekerLayout.setContentsMargins(0, 0, 0, 0);

        connect(mpPlayStopButton, SIGNAL(clicked()), this, SLOT(onPlayStopClicked()));
        connect(mpPauseButton, SIGNAL(clicked()), this, SLOT(onPauseClicked()));
        connect(mpSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
    }
    connect(mpPreviewTitle, SIGNAL(clicked()), this, SLOT(onTitleClicked()));
}

/**
  * \brief Destructor
  */
UBMediaWidget::~UBMediaWidget()
{
    unsetActionsParent();
    DELETEPTR(mpPreviewTitle);
    DELETEPTR(mpPicture);
    DELETEPTR(mpTitle);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpCover);
    DELETEPTR(mpSlider);
    DELETEPTR(mpPauseButton);
    DELETEPTR(mpPlayStopButton);
    DELETEPTR(mpAudioOutput);
    DELETEPTR(mpVideoWidget);
    DELETEPTR(mpMediaObject);
}

/**
  * \brief Set the media file
  * @param filePath as the media file path
  */
void UBMediaWidget::setFile(const QString &filePath)
{
    Q_ASSERT("" != filePath);
    mFilePath = filePath;
    mpMediaObject = new Phonon::MediaObject(this);
    mpMediaObject->setTickInterval(TICK_INTERVAL);
    connect(mpMediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(onStateChanged(Phonon::State,Phonon::State)));
    connect(mpMediaObject, SIGNAL(totalTimeChanged(qint64)), this, SLOT(onTotalTimeChanged(qint64)));
    connect(mpMediaObject, SIGNAL(tick(qint64)), this, SLOT(onTick(qint64)));
    mpMediaObject->setCurrentSource(Phonon::MediaSource(filePath));
}

/**
  * \brief Get the media type
  * @returns the media type
  */
eMediaType UBMediaWidget::mediaType()
{
    return mType;
}

void UBMediaWidget::showEvent(QShowEvent* event)
{
    if(eMediaType_Video == mType){
        if(!mpVideoWidget){
            mpVideoWidget = new Phonon::VideoWidget(this);
            mMediaLayout.addStretch(1);
            mMediaLayout.addWidget(mpVideoWidget, 0);
            mMediaLayout.addStretch(1);
            Phonon::createPath(mpMediaObject, mpVideoWidget);
            adaptSizeToVideo();
            mpMediaObject->play();
            mpMediaObject->stop();
        }
    }
    QWidget::showEvent(event);
}

/**
  * \brief Create the media player
  */
void UBMediaWidget::createMediaPlayer()
{
    if(eMediaType_Audio == mType || eMediaType_Video == mType){
        mpMediaContainer = new QWidget(this);
        mpMediaContainer->setObjectName("UBMediaVideoContainer");
        mpMediaContainer->setLayout(&mMediaLayout);

        if(eMediaType_Video == mType){
            if(eVizualisationMode_Half == mVizMode){
                mMediaLayout.setContentsMargins(0, 0, 0, 0);
            }else{
                mMediaLayout.setContentsMargins(10, 10, 25, 10);
            }

            if(isVisible()){
                mpVideoWidget = new Phonon::VideoWidget(this);
                mMediaLayout.addStretch(1);
                mMediaLayout.addWidget(mpVideoWidget, 0);
                mMediaLayout.addStretch(1);
                Phonon::createPath(mpMediaObject, mpVideoWidget);
                adaptSizeToVideo();
            }
            mpAudioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
            Phonon::createPath(mpMediaObject, mpAudioOutput);
        }else if(eMediaType_Audio == mType){
            if(eVizualisationMode_Half == mVizMode){
                mMediaLayout.setContentsMargins(0, 0, 0, 0);
            }else{
                mMediaLayout.setContentsMargins(10, 10, 10, 10);
            }
            mpCover = new QLabel(mpMediaContainer);
            mpMediaContainer->setStyleSheet(QString("background: none;"));
            setAudioCover(":images/libpalette/soundIcon.svg");
            mpCover->setScaledContents(true);
            mMediaLayout.addStretch(1);
            mMediaLayout.addWidget(mpCover, 0);
            mMediaLayout.addStretch(1);
            mpAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
            Phonon::createPath(mpMediaObject, mpAudioOutput);
        }

        mLayout.addWidget(mpMediaContainer, 1);
        mLayout.addLayout(&mSeekerLayout, 0);
        setActionsParent(mpMediaContainer);
    }else if(eMediaType_Picture == mType){
        mpPicture = new QLabel(this);
        QPixmap pix = QPixmap(mUrl);
        pix.scaledToWidth(mpPicture->width());
        mpPicture->resize(pix.width(), pix.height());
        mpPicture->setPixmap(pix);
        mpPicture->setScaledContents(true);
        mLayout.addWidget(mpPicture);
        setActionsParent(mpPicture);
    }
    mpTitleLabel = new QLabel(tr("Title"), this);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mLayout.addWidget(mpTitleLabel, 0);
    mLayout.addWidget(mpTitle, 0);
}

/**
  * \brief Adapt the widget size to the video in order to keep the good aspect ratio
  */
void UBMediaWidget::adaptSizeToVideo()
{
    if(NULL != mpMediaContainer){
        Phonon::VideoWidget::AspectRatio ar = mpVideoWidget->aspectRatio();
        int newW = width();
        float ratio = 1.0;
        switch(ar){
            case Phonon::VideoWidget::AspectRatio16_9:
                ratio = 16.0/9.0;
            break;
        case Phonon::VideoWidget::AspectRatio4_3:
                ratio = 4.0/3.0;
            break;
        default:
                ratio = mpVideoWidget->width() / mpVideoWidget->height();
            break;
        }

        int newH = newW / ratio;
        mpVideoWidget->resize(newW, newH);
    }
}

/**
  * \brief Handle the media state change notification
  * @param newState as the new state
  * @param oldState as the old state
  */
void UBMediaWidget::onStateChanged(Phonon::State newState, Phonon::State oldState)
{
    if(!mGeneratingThumbnail){
        if(Phonon::LoadingState == oldState && Phonon::StoppedState == newState){
            if(eMediaType_Video == mType){
                // We do that here to generate the thumbnail of the video
                mGeneratingThumbnail = true;
                mpMediaObject->play();
                mpMediaObject->pause();
                mGeneratingThumbnail = false;
            }
        }else if(Phonon::PlayingState == oldState && Phonon::PausedState == newState){
            mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
            mpPauseButton->setEnabled(false);
        }else if((Phonon::PausedState == oldState && Phonon::PlayingState == newState) ||
                 (Phonon::StoppedState == oldState && Phonon::PlayingState == newState)){
            mpPlayStopButton->setPixmap(QPixmap(":images/stop.svg"));
            mpPauseButton->setEnabled(true);
        }else if(Phonon::PlayingState == oldState && Phonon::StoppedState == newState){
            mpPlayStopButton->setPixmap(QPixmap(":images/play.svg"));
            mpPauseButton->setEnabled(false);
            mpSlider->setValue(0);
        }
    }
}

/**
  * \brief Handles the total time change notification
  * @param total as the new total time
  */
void UBMediaWidget::onTotalTimeChanged(qint64 total)
{
    mpSlider->setMaximum(total);
}

/**
  * \brief Handles the tick notification
  * @param currentTime as the current time
  */
void UBMediaWidget::onTick(qint64 currentTime)
{
    mAutoUpdate = true;
    mpSlider->setValue((int)currentTime);
    mAutoUpdate = false;
}

/**
  * \brief Handles the seeker value change notification
  * @param value as the new seeker value
  */
void UBMediaWidget::onSliderChanged(int value)
{
    if(!mAutoUpdate){
        mpMediaObject->seek(value);
    }
}

/**
  * \brief Toggle Play-Stop
  */
void UBMediaWidget::onPlayStopClicked()
{
    switch(mpMediaObject->state()){
        case Phonon::PlayingState:
            mpMediaObject->stop();
            break;

        case Phonon::StoppedState:
        case Phonon::PausedState:
            mpMediaObject->play();
            break;
        default:
            break;
    }
}

/**
  * \brief Pause the media
  */
void UBMediaWidget::onPauseClicked()
{
    mpMediaObject->pause();
}

/**
  * Get the border
  * @returns the actual border
  */
int UBMediaWidget::border()
{
    return mBorder;
}

/**
  * \brief Handles the resize event
  * @param ev as the resize event
  */
void UBMediaWidget::resizeEvent(QResizeEvent* ev)
{
    Q_UNUSED(ev);
}

/**
  * \brief Set the audio cover
  * @param coverPath as the cover image file path
  */
void UBMediaWidget::setAudioCover(const QString &coverPath)
{
    if(NULL != mpCover){
        mpCover->setPixmap(QPixmap(coverPath));
    }
}

void UBMediaWidget::setVizualisationMode(eVizualisationMode mode)
{
    switch(mode){
    case eVizualisationMode_Edit:
        qDebug() << "--> Setting mode to Edit";
        mpPreviewTitle->setVisible(false);
        mpTitleLabel->setVisible(true);
        mpTitle->setVisible(true);
        if(eMediaType_Audio == mType || eMediaType_Video == mType){
            mpMediaContainer->setVisible(true);
            mpPlayStopButton->setVisible(true);
            mpPauseButton->setVisible(true);
            mpSlider->setVisible(true);
        }else if(eMediaType_Picture == mType){
            mpPicture->setVisible(true);
        }
        break;
    case eVizualisationMode_Half:
        qDebug() << "--> Setting mode to Half";
        mpPreviewTitle->setVisible(true);
        mpPreviewTitle->setStyleSheet("background:red;");
        mpTitleLabel->setVisible(false);
        mpTitle->setVisible(false);
        mLayout.removeWidget(mpTitleLabel);
        mLayout.removeWidget(mpTitle);
        if(eMediaType_Audio == mType || eMediaType_Video == mType){
            mpMediaContainer->setVisible(false);
            mpPlayStopButton->setVisible(false);
            mpPauseButton->setVisible(false);
            mpSlider->setVisible(false);
            mLayout.removeWidget(mpMediaContainer);
            mLayout.removeItem(&mSeekerLayout);
        }else if(eMediaType_Picture == mType){
            mpPicture->setVisible(false);
            mLayout.removeWidget(mpPicture);
        }
        //setMaximumHeight(80);
        break;
    case eVizualisationMode_Full:
        qDebug() << "--> Setting mode to Full";
        // TODO: Update the height here
        int h = maximumHeight();
        mpPreviewTitle->setVisible(true);
        mpTitleLabel->setVisible(false);
        mpTitle->setVisible(false);
        if(eMediaType_Audio == mType || eMediaType_Video == mType){
            mpMediaContainer->setVisible(true);
            h += mpMediaContainer->height();
            mpPlayStopButton->setVisible(true);
            mpPauseButton->setVisible(true);
            mpSlider->setVisible(true);
            h += mpPlayStopButton->height();
        }else if(eMediaType_Picture == mType){
            mpPicture->setVisible(true);
            h += mpPicture->height();
        }
        setMinimumHeight(300);
        break;
    }
    mVizMode = mode;
    qDebug() << "Number of widgets: " << mLayout.count();
    qDebug() << "Size: " << size();
}

void UBMediaWidget::onTitleClicked()
{
    if(eVizualisationMode_Half == mVizMode){
        setVizualisationMode(eVizualisationMode_Full);
    }else if(eVizualisationMode_Full == mVizMode){
        setVizualisationMode(eVizualisationMode_Half);
    }
}

// -----------------------------------------------------------------------------------------------------------
/**
  * \brief Constructor
  * @param parent as the parent widget
  * @param name as the object name
  */
UBMediaButton::UBMediaButton(QWidget *parent, const char *name):QLabel(parent)
  , mPressed(false)
{
    setObjectName(name);
    resize(UBMEDIABUTTON_SIZE, UBMEDIABUTTON_SIZE);
    setStyleSheet(QString("padding:0px 0px 0px 0px; margin:0px 0px 0px 0px;"));
}

/**
  * \brief Destructor
  */
UBMediaButton::~UBMediaButton()
{

}

/**
  * \brief Handles the mouse press notification
  * @param ev as the mouse press event
  */
void UBMediaButton::mousePressEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    mPressed = true;
}

/**
  * \brief Handles the mouse release notification
  * @param ev as the mouse release event
  */
void UBMediaButton::mouseReleaseEvent(QMouseEvent* ev)
{
    Q_UNUSED(ev);
    if(mPressed){
        mPressed = false;
        emit clicked();
    }
}

// --------------------------------------------------------------------------------
UBMediaTitle::UBMediaTitle(eMediaType type, QWidget *parent, const char *name):QWidget(parent)
  , mpLayout(NULL)
  , mpIcon(NULL)
  , mpTitle(NULL)
  , mpAddToPageButton(NULL)
  , mpPlayFullscreenButton(NULL)
{
    setObjectName(name);
    mpLayout = new QHBoxLayout();
    mpLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mpLayout);
    setMinimumHeight(30);
    setMaximumHeight(42);

    // Icon
    mpIcon = new QLabel(this);
    mpIcon->setStyleSheet("padding-top:-19px; padding-left:-4px;");
    mpIcon->setMinimumSize(16,16);
    //mpIcon->setMaximumSize(16, 16);
    QImage icon;
    if(eMediaType_Video == type){
        icon = QImage(":/images/IconMovie.svg");
     }else if(eMediaType_Audio == type){
        icon = QImage(":/images/IconAudio.svg");
    }else if(eMediaType_Picture == type){
        icon = QImage(":/images/IconPicture.svg");
    }else if(eMediaType_App == type){
        icon = QImage(":/images/IconApp.svg");
    }else if(eMediaType_Flash == type){
        icon = QImage(":/images/IconFlash.svg");
    }

    mpIcon->setPixmap(QPixmap::fromImage(icon.scaledToHeight(24, Qt::SmoothTransformation)));
    //mpIcon->setScaledContents(true);

    mpLayout->addWidget(mpIcon, 0);

    // Title
    mpTitle = new QLabel(this);
    mpTitle->setAlignment(Qt::AlignLeft);
    mpLayout->addWidget(mpTitle, 1);

    // "Add To Page" button
    mpAddToPageButton = new QPushButton(this);
    // TODO: create icon for this button and set it

    mpAddToPageButton->setVisible(false);
    mpLayout->addWidget(mpAddToPageButton, 0);
    connect(mpAddToPageButton, SIGNAL(clicked()), this, SLOT(onAddToPage()));

    // "Play Fullscreen" button
    mpPlayFullscreenButton = new QPushButton(this);
    // TODO: create icon for this button and set it

    mpPlayFullscreenButton->setVisible(false);
    mpLayout->addWidget(mpPlayFullscreenButton, 0);
    connect(mpPlayFullscreenButton, SIGNAL(clicked()), this, SLOT(onPlayFullscreen()));
}

UBMediaTitle::~UBMediaTitle()
{
    DELETEPTR(mpPlayFullscreenButton);
    DELETEPTR(mpAddToPageButton);
    DELETEPTR(mpTitle);
    DELETEPTR(mpIcon);
    DELETEPTR(mpLayout);
}

void UBMediaTitle::setTitle(const QString &title)
{
    mpTitle->setText(title);
}

void UBMediaTitle::onAddToPage()
{
    // TODO: Implement me v(^_^ )v
}

void UBMediaTitle::onPlayFullscreen()
{
    // TODO: Implement me v(^_^ )v
}

void UBMediaTitle::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->pos().x() >= mpTitle->x() &&
        ev->pos().x() <= mpTitle->x() + mpTitle->width() &&
        ev->pos().y() >= mpTitle->y() &&
        ev->pos().y() <= mpTitle->y() + mpTitle->height()){
        UBMediaWidget* pMediaWidget = dynamic_cast<UBMediaWidget*>(parentWidget());
        if(NULL != pMediaWidget && !pMediaWidget->dragStarted()){
            emit clicked();
        }
    }
}

// ----------------------------------------------------------------------------------
UBMediaExpander::UBMediaExpander(QWidget *parent, const char *name):QWidget(parent)
  , mpLayout(NULL)
  , mpButton(NULL)
{
    setObjectName(name);
    mpLayout = new QHBoxLayout();
    setLayout(mpLayout);

    setStyleSheet("background:lightblue; padding: 0 0 0 0;");
    mExpanded = false;

    mpButton = new QLabel(this);
    QImage icon = QImage(":/images/increase.svg");
    mpButton->setPixmap(QPixmap::fromImage(icon.scaledToHeight(UBTOGGLER_SIZE, Qt::SmoothTransformation)));
    mpLayout->addStretch(1);
    mpLayout->addWidget(mpButton, 0);
    mpLayout->addStretch(1);

    connect(this, SIGNAL(toggleMedias()), this, SLOT(onToggleMedias()));
}

UBMediaExpander::~UBMediaExpander()
{
    DELETEPTR(mpButton);
    DELETEPTR(mpLayout);
}

void UBMediaExpander::mousePressEvent(QMouseEvent* ev)
{
    if( ev->pos().x() >= mpButton->pos().x()
        && ev->pos().x() <= mpButton->pos().x() + mpButton->width()
        && ev->pos().y() >= mpButton->pos().y()
        && ev->pos().y() <= mpButton->pos().y() + mpButton->height()){
        emit toggleMedias();
    }
}

void UBMediaExpander::onToggleMedias()
{
    mExpanded = !mExpanded;
    QImage icon;
    if(mExpanded){
        icon = QImage(":/images/decrease.svg");
    }else{
        icon = QImage(":/images/increase.svg");
    }

    mpButton->setPixmap(QPixmap::fromImage(icon.scaledToHeight(UBTOGGLER_SIZE, Qt::SmoothTransformation)));
}
