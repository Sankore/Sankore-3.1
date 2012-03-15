#include <QDate>

#include "UBTeacherBarDataMgr.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "board/UBBoardController.h"

#include "globals/UBGlobals.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"


UBTeacherBarDataMgr::UBTeacherBarDataMgr()
{

}

UBTeacherBarDataMgr::~UBTeacherBarDataMgr()
{

}

void UBTeacherBarDataMgr::clearLists()
{
    mActionList.clear();
    mUrlList.clear();
    mMediaList.clear();
}



void UBTeacherBarDataMgr::saveContent()
{
    // Store the page information in the UBZ
    sTeacherBarInfos infos;

    // Page Title
    infos.title = mPageTitle;
    // Actions
    foreach(sAction action, mActionList){
        infos.actions << QString("%0%1%2").arg(action.type).arg(UBPersistenceManager::gSeparator).arg(action.content);
    }
    // Media
    foreach(sMedia media, mMediaList){
        infos.medias << QString("%0%1%2").arg(media.title).arg(UBPersistenceManager::gSeparator).arg(media.url.remove(UBApplication::boardController->activeDocument()->persistencePath() + "/"));
    }
    // Links
    foreach(sLink link, mUrlList){
        if("" != link.title && "" != link.link){
            infos.urls << QString("%0%1%2").arg(link.title).arg(UBPersistenceManager::gSeparator).arg(link.link);
        }
    }
    // Comments
    infos.comments = mComments;

    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    if(documentProxy){
        UBPersistenceManager::persistenceManager()->persistTeacherBar(documentProxy, UBApplication::boardController->activeSceneIndex(), infos);

        documentProxy->setSessionTitle(mSessionTitle);
        documentProxy->setSessionTarget(mSessionTarget);
        documentProxy->setSessionLicence(QString("%0").arg(mSessionLicence));
        documentProxy->setSessionKeywords(mKeywords);
        documentProxy->setSessionLevel(mLevel);
        documentProxy->setSessionTopic(mTopic);
        documentProxy->setSessionAuthor(mAuthors);

        UBMetadataDcSubsetAdaptor::persist(documentProxy);
    }
}



void UBTeacherBarDataMgr::loadContent(bool docChanged)
{
    Q_UNUSED(docChanged);
    clearLists();
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();

    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(documentProxy, UBApplication::boardController->activeSceneIndex());
    if(true/*docChanged*/){
        mSessionTitle = documentProxy->sessionTitle();
        mSessionTarget = documentProxy->sessionTarget();
        mSessionLicence = (eLicense)documentProxy->sessionLicence().toInt();
        mKeywords = documentProxy->sessionKeywords();
        mLevel = documentProxy->sessionLevel();
        mTopic = documentProxy->sessionTopic();
        mAuthors = documentProxy->sessionAuthors();
        if("" != documentProxy->documentDate()){
            mCreationDate = documentProxy->documentDate();
        }else{
            mCreationDate = QDate::currentDate().toString("yyyy-MM-dd");
        }

    }

    // Page Title
    mPageTitle = nextInfos.title;
    // Actions
    foreach(QString eachAction, nextInfos.actions){
        QStringList qslAction = eachAction.split(UBPersistenceManager::gSeparator);
        if(2 <= qslAction.size()){
            sAction action;
            action.type = qslAction.at(0).toInt();
            action.content = qslAction.at(1);
            mActionList << action;
        }
    }
    // Media URL
    if((nextInfos.medias.size() == 1) && (nextInfos.medias.at(0) == "")){
        // Do not retrieve it
    }
    else{
        // title;url@title;url;...
        foreach(QString media, nextInfos.medias){
            QStringList qslMedia = media.split(UBPersistenceManager::gSeparator);
            if(qslMedia.size() >= 2){
                sMedia med;
                med.title = qslMedia.at(0);
                med.url = UBApplication::boardController->activeDocument()->persistencePath() + "/" + qslMedia.at(1);
                mMediaList << med;
            }
        }
    }

    // Links
    foreach(QString eachUrl, nextInfos.urls){
        QStringList qslUrl = eachUrl.split(UBPersistenceManager::gSeparator);
        if(2 <= qslUrl.size()){
            sLink link;
            link.title = qslUrl.at(0);
            link.link = qslUrl.at(1);
            mUrlList << link;
        }
    }

    // Comments
    mComments = nextInfos.comments;
}

// ------------------------------------------------------------------------------------
UBTBSeparator::UBTBSeparator(QWidget *parent, const char *name):QFrame(parent)
{
    Q_UNUSED(name);
    setObjectName("UBTBSeparator");
    setMinimumHeight(5);
    setMaximumHeight(5);
}

UBTBSeparator::~UBTBSeparator()
{

}

// ------------------------------------------------------------------------------------
UBTBLicenseWidget::UBTBLicenseWidget(QWidget *parent, const char *name):QWidget(parent)
{
    setObjectName(name);
    setLayout(&mLayout);

    mpIcon = new QLabel(this);
    mpText = new QLabel(this);
    mpText->setWordWrap(true);
    mLayout.addWidget(mpIcon);
    mLayout.addWidget(mpText);
}

UBTBLicenseWidget::~UBTBLicenseWidget()
{
    DELETEPTR(mpIcon);
    DELETEPTR(mpText);
}

void UBTBLicenseWidget::setLicense(eLicense lic)
{
    switch(lic){
        case eLicense_CCBY:
            mpIcon->setPixmap(QPixmap(":images/licenses/ccby.png"));
            mpText->setText(tr("Creative Common License %0").arg("CC BY"));
            break;
        case eLicense_CCBYND:
            mpIcon->setPixmap(QPixmap(":images/licenses/ccbynd.png"));
            mpText->setText(tr("Creative Common License %0").arg("CC BY-ND"));
            break;
        case eLicense_CCBYNCSA:
            mpIcon->setPixmap(QPixmap(":images/licenses/ccbyncsa.png"));
            mpText->setText(tr("Creative Common License %0").arg("CC BY-NC-SA"));
            break;
        case eLicense_CCBYSA:
            mpIcon->setPixmap(QPixmap(":images/licenses/ccbysa.png"));
            mpText->setText(tr("Creative Common License %0").arg("CC BY-SA"));
            break;
        case eLicense_CCBYNC:
            mpIcon->setPixmap(QPixmap(":images/licenses/ccbync.png"));
            mpText->setText(tr("Creative Common License %0").arg("CC BY-NC"));
            break;
        case eLicense_CCBYNCND:
            mpIcon->setPixmap(QPixmap(":images/licenses/ccbyncnd.png"));
            mpText->setText(tr("Creative Common License %0").arg("CC BY-NC-ND"));
            break;
    }
}
