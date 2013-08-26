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



#include <QGraphicsItem>
#include <QPointF>
#include <QtGui>

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "UBFeaturesController.h"
#include "core/UBSettings.h"
#include "tools/UBToolsManager.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBDownloadManager.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"
#include "domain/UBGraphicsVideoItem.h"
#include "domain/UBGraphicsWidgetItem.h"

#include "gui/UBFeaturesWidget.h"

void UBFeaturesComputingThread::scanFS(const QUrl & currentPath
                                       , const QString & currVirtualPath
                                       , const QPair<CategoryData, QSet<QUrl> > &pfavoriteInfo
                                       , UBFeature::Permissions pPermissions
                                       , const QList<CategoryData> &extData)
{
    Q_ASSERT(QFileInfo(currentPath.toLocalFile()).exists());

    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(currentPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        if (abort) {
            return;
        }

        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = UBFeaturesController::fileTypeFromUrl(fullFileName);
        QString fileName = fileInfo->fileName();

        QImage icon = UBFeaturesController::getIcon(fullFileName, featureType);

        if ( fullFileName.contains(".thumbnail."))
            continue;

        //Testing exception permissions
        QString testVirtualPath = currVirtualPath + "/" + fileName;
        UBFeature::Permissions testPermissions = pPermissions;

        foreach (CategoryData curException, extData) {
            QString exceptionVirtualPath = curException.categoryFeature().getFullVirtualPath();
            if (testVirtualPath.startsWith(exceptionVirtualPath)) {
                qDebug() << "catched";
                qDebug() << "ext" << exceptionVirtualPath << "test" << testVirtualPath;
                //folder itself
                if (exceptionVirtualPath.count() == testVirtualPath.count()) {
                    testPermissions = curException.categoryFeature().getPermissions();
                //subfolder
                } else {
                    testPermissions = curException.UserSubPermissions();
                }
            }
        }

        UBFeature testFeature(testVirtualPath, icon, fileName, QUrl::fromLocalFile(fullFileName), featureType, testPermissions);

        emit sendFeature(testFeature);
        emit featureSent();
        emit scanPath(fullFileName);

        if ( pfavoriteInfo.second.find(QUrl::fromLocalFile(fullFileName)) != pfavoriteInfo.second.end()) {
            emit sendFeature(UBFeature(pfavoriteInfo.first.categoryFeature().getFullVirtualPath() + "/" + fileName
                                       , icon
                                       , fileName
                                       , QUrl::fromLocalFile(fullFileName)
                                       , featureType
                                       , pfavoriteInfo.first.librarySubPermissions())); //permissions for favorites subfolders
        }

        if (featureType == FEATURE_FOLDER) {
            scanFS(QUrl::fromLocalFile(fullFileName), currVirtualPath + "/" + fileName, pfavoriteInfo, pPermissions, extData);
        }
    }
}

void UBFeaturesComputingThread::scanAll(QList<CategoryData> pScanningData
                                        , const QPair<CategoryData, QSet<QUrl> >  &pFavoriteInfo
                                        , const QList<CategoryData> &extData)
{
    for (int n = static_cast<int>(CategoryData::Library); n <= static_cast<int>(CategoryData::UserDefined); n++) {
        for (int i = 0; i < pScanningData.count(); i++) {
            if (abort) {
                return;
            }
            CategoryData curData = pScanningData.at(i);

            emit scanCategory(curData.categoryFeature().getDisplayName());

            CategoryData::pathType pType = static_cast<CategoryData::pathType>(n);
            QList<QUrl> curScanPaths = curData.pathData().values(pType);
            foreach (QUrl curUrl, curScanPaths) {
//                qDebug() << "location" << i << "is" << curUrl.toLocalFile();
                scanFS(curUrl
                       , curData.categoryFeature().getFullVirtualPath()
                       , pFavoriteInfo
                       , curData.subFolderPermissionsForType(pType)
                       , extData);
            }
        }
    }
}

int UBFeaturesComputingThread::featuresCount(const QUrl &pPath)
{
    int noItems = 0;

    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(pPath.toLocalFile());

    QFileInfoList::iterator fileInfo;
    for ( fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo +=  1) {
        QString fullFileName = fileInfo->absoluteFilePath();
        UBFeatureElementType featureType = UBFeaturesController::fileTypeFromUrl(fullFileName);

        if (featureType != FEATURE_INVALID && !fullFileName.contains(".thumbnail.")) {
            noItems++;
        }

        if (featureType == FEATURE_FOLDER) {
            noItems += featuresCount(QUrl::fromLocalFile(fullFileName));
        }
    }

    return noItems;
}

int UBFeaturesComputingThread::featuresCountAll(QList<CategoryData> pScanningData)
{
    int noItems = 0;
    for (int i = 0; i < pScanningData.count(); i++) {
        CategoryData curCategoryData = pScanningData.at(i);
        QMapIterator<CategoryData::pathType, QUrl> pathIterator(curCategoryData.pathData());
        while (pathIterator.hasNext()) {
            pathIterator.next();
            noItems += featuresCount(pathIterator.value());
        }
    }

    return noItems;
}

UBFeaturesComputingThread::UBFeaturesComputingThread(QObject *parent) :
QThread(parent)
{
    restart = false;
    abort = false;
}

void UBFeaturesComputingThread::compute(const QList<CategoryData> &pcategoryList, const QPair<CategoryData, QSet<QUrl> > &pFavoriteInfo, const QList<CategoryData> &extData)
{
    QMutexLocker curLocker(&mMutex);

    mScanningData = pcategoryList;
    mExceptionData = extData;
    mFavoriteInfo = pFavoriteInfo;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        mWaitCondition.wakeOne();
    }
}

void UBFeaturesComputingThread::run()
{
    forever {
//        qDebug() << "Custom thread started execution";

        mMutex.lock();
        QList<CategoryData> searchData = mScanningData;
        QList<CategoryData> extData = mExceptionData;
        QPair<CategoryData, QSet<QUrl> > favoriteInfo = mFavoriteInfo;
        mMutex.unlock();

        if (abort) {
            return;
        }
        if (restart) {
            break;
        }

//        QTime curTime = Q.Time::currentTime();
        int fsCnt = featuresCountAll(searchData);
//        int msecsto = curTime.msecsTo(QTime::currentTime());
//        qDebug() << "time on evaluation" << msecsto;

        emit maxFilesCountEvaluated(fsCnt);
        emit scanStarted();

//        curTime = QTime::currentTime();
        scanAll(searchData, favoriteInfo, extData);
//        qDebug() << "Time on finishing" << curTime.msecsTo(QTime::currentTime());
        emit scanFinished();

        mMutex.lock();
        if (!abort) {
            mWaitCondition.wait(&mMutex);
        }
        restart = false;
        mMutex.unlock();

    }
}

UBFeaturesComputingThread::~UBFeaturesComputingThread()
{
//    qDebug() <<  "thread destructor catched";

    mMutex.lock();
    abort = true;
    mWaitCondition.wakeOne();
    mMutex.unlock();

    wait();
}

UBFeature::UBFeature(const QString &url
                     , const QImage &icon
                     , const QString &name
                     , const QUrl &realPath
                     , UBFeatureElementType type
                     , Permissions pOwnPermissions
                     , QString pSortKey)
    : mThumbnail(icon)
    , mDisplayName(name)
    , mPath(realPath)
    , elementType(type)
    , mOwnPermissions(pOwnPermissions)
    , mSortKey(pSortKey)
{
    mName = getNameFromVirtualPath(url);
    virtualDir = getVirtualDirFromVirtualPath(url);
    if (mSortKey.isNull()) {
        mSortKey = name;
    }
}

UBFeature::~UBFeature()
{
}

QString UBFeature::getNameFromVirtualPath(const QString &pVirtPath)
{
    QString result;
    int slashPos = pVirtPath.lastIndexOf("/");
    if (slashPos != -1) {
        result = pVirtPath.right(pVirtPath.count() - slashPos - 1);
    } else {
        qDebug() << "UBFeature: incorrect virtual path parameter specified";
    }

    return result;
}

QString UBFeature::getVirtualDirFromVirtualPath(const QString &pVirtPath)
{
    QString result;
    int slashPos = pVirtPath.lastIndexOf("/");
    if (slashPos != -1) {
        result = pVirtPath.left(slashPos);
    } else {
        qDebug() << "UBFeature: incorrect virtual path parameter specified";
    }

    return result;
}

QString UBFeature::getUrl() const
{
    if ( elementType == FEATURE_INTERNAL )
        return getFullPath().toString();

    return getFullPath().toLocalFile();
}

UBFeature &UBFeature::markedWithSortKey(const QString &str)
{
    mSortKey = str;
    return *this;
}

bool UBFeature::operator ==( const UBFeature &f )const
{
    return virtualDir == f.getVirtualPath() && mName == f.getName() && mPath == f.getFullPath() && elementType == f.getType();
}

bool UBFeature::operator !=( const UBFeature &f )const
{
    return !(*this == f);
}

bool UBFeature::isFolder() const
{
    return elementType == FEATURE_CATEGORY || elementType == FEATURE_TRASH || elementType == FEATURE_FAVORITE
        || elementType == FEATURE_FOLDER || elementType == FEATURE_SEARCH;
}

bool UBFeature::allowedCopy() const
{
    return isFolder()
            && elementType != FEATURE_CATEGORY
            && elementType != FEATURE_SEARCH;
}

bool UBFeature::isDeletable() const
{
    return elementType == FEATURE_ITEM
            || elementType == FEATURE_AUDIO
            || elementType == FEATURE_VIDEO
            || elementType == FEATURE_IMAGE
            || elementType == FEATURE_FLASH
            || elementType == FEATURE_FOLDER
            || elementType == FEATURE_BOOKMARK
            || elementType == FEATURE_LINK
    //Ilia. Just a hotfix. Permission mechanism for UBFeatures should be reworked
            || getVirtualPath().startsWith("/root/Applications/Web");
}

bool UBFeature::inTrash() const
{
    return getFullPath().toLocalFile().startsWith(QUrl::fromLocalFile(UBSettings::userTrashDirPath()).toLocalFile() );
}

UBFeaturesController::UBFeaturesController(QWidget *pParentWidget) :
    QObject(pParentWidget)
    ,featuresList(0)
    ,mLastItemOffsetIndex(0)
{
    initHardcodedData();

    featuresList = new QList <UBFeature>();

    loadHardcodedItemsToModel();

    featuresModel = new UBFeaturesModel(featuresList, this);
    featuresModel->setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);

    featuresProxyModel = new UBFeaturesProxyModel(this);
    featuresProxyModel->setFilterFixedString(rootData.categoryFeature().getFullVirtualPath());
    featuresProxyModel->setSourceModel(featuresModel);
    featuresProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    featuresSearchModel = new UBFeaturesSearchProxyModel(this);
    featuresSearchModel->setSourceModel(featuresModel);
    featuresSearchModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    featuresPathModel = new UBFeaturesPathProxyModel(this);
    featuresPathModel->setPath(rootData.categoryFeature().getFullVirtualPath());
    featuresPathModel->setSourceModel(featuresModel);

    connect(featuresModel, SIGNAL(dataRestructured()), featuresProxyModel, SLOT(invalidate()));
    connect(&mCThread, SIGNAL(sendFeature(UBFeature)), featuresModel, SLOT(addItem(UBFeature)));
    connect(&mCThread, SIGNAL(featureSent()), this, SIGNAL(featureAddedFromThread()));
    connect(&mCThread, SIGNAL(scanStarted()), this, SIGNAL(scanStarted()));
    connect(&mCThread, SIGNAL(scanFinished()), this, SIGNAL(scanFinished()));
    connect(&mCThread, SIGNAL(maxFilesCountEvaluated(int)), this, SIGNAL(maxFilesCountEvaluated(int)));
    connect(&mCThread, SIGNAL(scanCategory(QString)), this, SIGNAL(scanCategory(QString)));
    connect(&mCThread, SIGNAL(scanPath(QString)), this, SIGNAL(scanPath(QString)));
    connect(UBApplication::boardController, SIGNAL(npapiWidgetCreated(QString)), this, SLOT(createNpApiFeature(QString)));

    QTimer::singleShot(0, this, SLOT(scanFS()));
}

void UBFeaturesController::scanFS()
{
    QList<CategoryData> computingData;

    computingData << audiosData
                  << moviesData
                  << flashData
                  << picturesData
                  << appData
                  << shapesData
                  << interactivityData
                  << webSearchData
                  << trashData
                  << bookmarkData;

    QPair<CategoryData, QSet<QUrl> > favoriteInfo(favoriteData, *favoriteSet);

    mCThread.compute(computingData, favoriteInfo, extentionPermissionsCategoryData);
}

void UBFeaturesController::createNpApiFeature(const QString &str)
{
    Q_ASSERT(QFileInfo(str).exists() && QFileInfo(str).isDir());

    QString widgetName = QFileInfo(str).fileName();

    featuresModel->addItem(UBFeature(QString(appData.categoryFeature().getFullVirtualPath() + "/Web/" + widgetName)
                                     , QImage(UBGraphicsWidgetItem::iconFilePath(QUrl::fromLocalFile(str)))
                                     , widgetName
                                     , QUrl::fromLocalFile(str)
                                     , FEATURE_INTERACTIVE));
}

void UBFeaturesController::initHardcodedData()
{
    rootData = CategoryData(CategoryData::PathData() //Static library paths for root category. Scanning data
                            .insertr(CategoryData::Library, QUrl())
                            //UBFeature represented root element
                            , UBFeature("/root"                                   //Virtual Path
                                        ,QImage(":images/libpalette/home.png")     //Icon
                                        ,"root"                                    //Translation name
                                        , QUrl()                                   //Main path in file system
                                        , FEATURE_CATEGORY                         //UBFeature's type
                                        , UBFeature::NO_P)                         //UBFeature's permissions
                            //permissions for category subfolders. Scanning data
                            , CategoryData::PermissionsData()
                            .insertr(CategoryData::Library, UBFeature::NO_P)
                            .insertr(CategoryData::UserDefined, UBFeature::NO_P));

    audiosData = CategoryData(CategoryData::PathData() //Static library paths for Audios category. Scanning data
                              .insertr(CategoryData::Library,     QUrl::fromLocalFile(UBSettings::settings()->applicationAudiosLibraryDirectory()))
                              .insertr(CategoryData::UserDefined, QUrl::fromLocalFile(UBSettings::settings()->userAudioDirectory()))
                              //UBFeature represented Audios element
                              , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Audios"   //Virtual Path
                                          ,QImage(":images/libpalette/AudiosCategory.svg")           //Icon
                                          , tr("Audios")                                             //Translation name
                                          , QUrl::fromLocalFile(UBSettings::settings()->
                                                                applicationAudiosLibraryDirectory()) //Main path in file system
                                          , FEATURE_CATEGORY                                         //UBFeature's type
                                          , UBFeature::WRITE_P)                                      //UBFeature's permissions
                              //permissions for category subfolders. Scanning data
                              , CategoryData::PermissionsData()
                              .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                              .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    moviesData = CategoryData(CategoryData::PathData()  //Static library paths for Videos category. Scanning data
                              .insertr(CategoryData::Library,     QUrl::fromLocalFile(UBSettings::settings()->applicationVideosLibraryDirectory()))
                              .insertr(CategoryData::UserDefined, QUrl::fromLocalFile(UBSettings::settings()->userVideoDirectory()))
                              //UBFeature represented Movies element
                              , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Movies"   //Virtual Path
                                          ,QImage(":images/libpalette/MoviesCategory.svg")           //Icon
                                          , tr("Movies")                                             //Translation name
                                          , QUrl::fromLocalFile(UBSettings::settings()->
                                                                applicationVideosLibraryDirectory()) //Main path in file system
                                          , FEATURE_CATEGORY                                         //UBFeature's type
                                          , UBFeature::WRITE_P)                                      //UBFeature's permissions
                              //permissions for category subfolders. Scanning data
                              , CategoryData::PermissionsData()
                              .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                              .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    picturesData = CategoryData(CategoryData::PathData()  //Static library paths for Pictures category. Scanning data
                                .insertr(CategoryData::Library,     QUrl::fromLocalFile(UBSettings::settings()->applicationImageLibraryDirectory()))
                                .insertr(CategoryData::UserDefined, QUrl::fromLocalFile(UBSettings::settings()->userImageDirectory()))
                                //UBFeature represented Pictures element
                                , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Pictures" //Virtual Path
                                            ,QImage(":images/libpalette/PicturesCategory.svg")         //Icon
                                            , tr("Pictures")                                           //Translation name
                                            , QUrl::fromLocalFile(UBSettings::settings()->
                                                                  applicationImageLibraryDirectory())  //Main path in file system
                                            , FEATURE_CATEGORY                                         //UBFeature's type
                                            , UBFeature::WRITE_P)                                      //UBFeature's permissions
                                //permissions for category subfolders. Scanning data
                                , CategoryData::PermissionsData()
                                .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                                .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    flashData = CategoryData(CategoryData::PathData() //Static library paths for Flash category. Scanning data
                             .insertr(CategoryData::Library,     QUrl::fromLocalFile(UBSettings::settings()->applicationAnimationsLibraryDirectory()))
                             .insertr(CategoryData::UserDefined, QUrl::fromLocalFile(UBSettings::settings()->userAnimationDirectory()))
                             //UBFeature represented Animations element
                             , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Animations"   //Virtual Path
                                         ,QImage(":images/libpalette/FlashCategory.svg")                //Icon
                                         , tr("Animations")                                             //Translation name
                                         , QUrl::fromLocalFile(UBSettings::settings()->
                                                               applicationAnimationsLibraryDirectory()) //Main path in file system
                                         , FEATURE_CATEGORY                                             //UBFeature's type
                                         , UBFeature::NO_P)                                             //UBFeature's permissions
                             //permissions for category subfolders. Scanning data
                             , CategoryData::PermissionsData()
                             .insertr(CategoryData::Library, UBFeature::NO_P)
                             .insertr(CategoryData::UserDefined, UBFeature::NO_P));

    appData = CategoryData(CategoryData::PathData()   //Static library paths for Applications category. Scanning data
                           .insertr(CategoryData::Library,  QUrl::fromLocalFile(UBSettings::settings()->applicationApplicationsLibraryDirectory()))
                           .insertr(CategoryData::UserDefined, QUrl::fromLocalFile(UBSettings::settings()->userApplicationDirectory()))
                           //UBFeature represented Applications element
                           , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Applications"   //Virtual Path
                                       ,QImage(":images/libpalette/ApplicationsCategory.svg")           //Icon
                                       , tr("Applications")                                             //Translation name
                                       , QUrl::fromLocalFile(UBSettings::settings()->
                                                             applicationApplicationsLibraryDirectory())        //Main path in file system
                                       , FEATURE_CATEGORY                                               //UBFeature's type
                                       , UBFeature::NO_P)                                               //UBFeature's permissions
                           //permissions for category subfolders. Scanning data
                           , CategoryData::PermissionsData()
                           .insertr(CategoryData::Library, UBFeature::NO_P)
                           .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    interactivityData = CategoryData(CategoryData::PathData() //Static library paths for Interactivities category. Scanning data
                                     .insertr(CategoryData::Library,     QUrl::fromLocalFile(UBSettings::settings()->applicationInteractivesDirectory()))
                                     .insertr(CategoryData::UserDefined, QUrl::fromLocalFile(UBSettings::settings()->userInteractiveDirectory()))
                                     //UBFeature represented Interactivities element
                                     , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Interactivities"     //Virtual Path
                                                 ,QImage(":images/libpalette/InteractivesCategory.svg")                //Icon
                                                 , tr("Interactivities")                                               //Translation name
                                                 , QUrl::fromLocalFile(UBSettings::settings()->
                                                                       applicationInteractivesDirectory())             //Main path in file system
                                                 , FEATURE_CATEGORY                                                    //UBFeature's type
                                                 , UBFeature::NO_P)                                                    //UBFeature's permissions
                                     //permissions for category subfolders. Scanning data
                                     , CategoryData::PermissionsData()
                                     .insertr(CategoryData::Library, UBFeature::NO_P)
                                     .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    shapesData = CategoryData(CategoryData::PathData() //Static library paths for Shapes category. Scanning data
                              .insertr(CategoryData::Library, QUrl::fromLocalFile(UBSettings::settings()->applicationShapeLibraryDirectory()))
                              //UBFeature represented Shapes element
                              , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Shapes"   //Virtual Path
                                          ,QImage(":images/libpalette/ShapesCategory.svg")           //Icon
                                          , tr("Shapes")                                             //Translation name
                                          , QUrl::fromLocalFile(UBSettings::settings()->
                                                                applicationShapeLibraryDirectory())  //Main path in file system
                                          , FEATURE_CATEGORY                                         //UBFeature's type
                                          , UBFeature::NO_P)                                         //UBFeature's permissions
                              //permissions for category subfolders. Scanning data
                              , CategoryData::PermissionsData()
                              .insertr(CategoryData::Library, UBFeature::NO_P)
                              .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    favoriteData = CategoryData(CategoryData::PathData() //Static library paths for Favorites category. Scanning data
                                //UBFeature represented Favorites element
                                , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Favorites"   //Virtual Path
                                            ,QImage(":images/libpalette/FavoritesCategory.svg")           //Icon
                                            , tr("Favorites")                                             //Translation name
                                            , QUrl()                                                      //Main path in file system
                                            , FEATURE_FAVORITE                                             //UBFeature's type
                                            , UBFeature::NO_P)                                            //UBFeature's permissions
                                //permissions for category subfolders. Scanning data
                                , CategoryData::PermissionsData()
                                .insertr(CategoryData::Library, UBFeature::NO_P)
                                .insertr(CategoryData::UserDefined, UBFeature::NO_P));

    webSearchData = CategoryData(CategoryData::PathData()  //Static library paths for Web search category. Scanning data
                                 .insertr(CategoryData::Library, QUrl::fromLocalFile(UBSettings::settings()->userSearchDirectory()))
                                 //UBFeature represented Web search element
                                 , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Web search"   //Virtual Path
                                             ,QImage(":images/libpalette/WebSearchCategory.svg")            //Icon
                                             , tr("Web search")                                             //Translation name
                                             , QUrl::fromLocalFile(UBSettings::settings()->
                                                                   userSearchDirectory())                   //Main path in file system
                                             , FEATURE_CATEGORY                                             //UBFeature's type
                                             , UBFeature::NO_P)                                             //UBFeature's permissions
                                 //permissions for category subfolders. Scanning data
                                 , CategoryData::PermissionsData()
                                 .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                                 .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));


    bookmarkData = CategoryData(CategoryData::PathData() //Path for incoming user web content.
                                .insertr(CategoryData::Library, QUrl::fromLocalFile(UBSettings::settings()->userBookmarkDirectory()))
                                //UBFeature represented Trash element
                                , UBFeature(rootData.categoryFeature().getFullVirtualPath()
                                            + "/Bookmarks"                //Virtual Path
                                            , QImage(":images/libpalette/BookmarkCategory.svg")         //Icon
                                            , tr("Bookmarks")                                             //Translation name
                                            , QUrl::fromLocalFile(UBSettings::settings()->userBookmarkDirectory())  //Main path in file system
                                            , FEATURE_CATEGORY                                      //UBFeature's type
                                            , UBFeature::WRITE_P)                                   //UBFeature's permissions
                                //permissions for category subfolders. Scanning data
                                , CategoryData::PermissionsData()
                                .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                                .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    trashData = CategoryData(CategoryData::PathData() //Static library paths for Trash category. Scanning data
                             .insertr(CategoryData::Library, QUrl::fromLocalFile(UBSettings::userTrashDirPath()))
                             //UBFeature represented Trash element
                             , UBFeature(rootData.categoryFeature().getFullVirtualPath() + "/Trash"   //Virtual Path
                                         ,QImage(":images/libpalette/TrashCategory.svg")           //Icon
                                         , tr("Trash")                                             //Translation name
                                         , QUrl::fromLocalFile(UBSettings::userTrashDirPath())     //Main path in file system
                                         , FEATURE_TRASH                                           //UBFeature's type
                                         , UBFeature::WRITE_P)                                     //UBFeature's permissions
                             //permissions for category subfolders. Scanning data
                             , CategoryData::PermissionsData()
                             .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                             .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));

    webFolderData = CategoryData(CategoryData::PathData() //Path for incoming user web content.
                                 .insertr(CategoryData::Library, QUrl::fromLocalFile(UBSettings::settings()->userWidgetPath()))
                                 //UBFeature represented Trash element
                                 , UBFeature(appData.categoryFeature().getFullVirtualPath()
                                             + "/"
                                             + UBSettings::userRelativeWidgetPath()                //Virtual Path
                                             ,QImage(":images/libpalette/TrashCategory.svg")         //Icon
                                             , tr("Web")                                             //Translation name
                                             , QUrl::fromLocalFile(UBSettings::settings()->
                                                                   userWidgetPath())                 //Main path in file system
                                             , FEATURE_CATEGORY                                      //UBFeature's type
                                             , UBFeature::WRITE_P)                                   //UBFeature's permissions
                                 //permissions for category subfolders. Scanning data
                                 , CategoryData::PermissionsData()
                                 .insertr(CategoryData::Library, UBFeature::WRITE_P | UBFeature::DELETE_P)
                                 .insertr(CategoryData::UserDefined, UBFeature::WRITE_P | UBFeature::DELETE_P));



    extentionPermissionsCategoryData << webFolderData;
}

void UBFeaturesController::loadHardcodedItemsToModel()
{
    featuresList->clear();
    featuresList->append(rootData.categoryFeature());

    *featuresList << audiosData.categoryFeature().markedWithSortKey("01")
                    << moviesData.categoryFeature().markedWithSortKey("02")
                    << picturesData.categoryFeature().markedWithSortKey("03")
                    << flashData.categoryFeature().markedWithSortKey("04")
                    << interactivityData.categoryFeature().markedWithSortKey("05")
                    << appData.categoryFeature().markedWithSortKey("06")
                    << shapesData.categoryFeature().markedWithSortKey("07")
                    << favoriteData.categoryFeature().markedWithSortKey("08")
                    << webSearchData.categoryFeature().markedWithSortKey("09")
                    << bookmarkData.categoryFeature().markedWithSortKey("10")
                    << trashData.categoryFeature().markedWithSortKey("11");

    //filling favoriteList
    loadFavoriteList();

    QList <UBToolsManager::UBToolDescriptor> tools = UBToolsManager::manager()->allTools();

    foreach (UBToolsManager::UBToolDescriptor tool, tools) {
        featuresList->append(UBFeature(appData.categoryFeature().getFullVirtualPath() + "/" + tool.label, tool.icon.toImage(), tool.label, QUrl(tool.id), FEATURE_INTERNAL, UBFeature::NO_P));
        if (favoriteSet->find(QUrl(tool.id)) != favoriteSet->end()) {
            featuresList->append(UBFeature(favoriteData.categoryFeature().getFullVirtualPath() + "/" + tool.label, tool.icon.toImage(), tool.label, QUrl(tool.id), FEATURE_INTERNAL, UBFeature::NO_P));
        }
    }
}

void UBFeaturesController::loadFavoriteList()
{
    favoriteSet = new QSet<QUrl>();
    QFile file( UBSettings::userDataDirectory() + "/favorites.dat" );
    if ( file.exists() )
    {
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        int elementsNumber;
        in >> elementsNumber;
        for ( int i = 0; i < elementsNumber; ++i)
        {
            QUrl path;
            in >> path;
            favoriteSet->insert( path );
        }
    }
}

void UBFeaturesController::saveFavoriteList()
{
    QFile file( UBSettings::userDataDirectory() + "/favorites.dat" );
    file.resize(0);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out << favoriteSet->size();
    for ( QSet<QUrl>::iterator it = favoriteSet->begin(); it != favoriteSet->end(); ++it )
    {
        out << (*it);
    }
    file.close();
}

QString UBFeaturesController::uniqNameForFeature(const UBFeature &feature, const QString &pName, const QString &pExtention) const
{
    Q_ASSERT(featuresList);

    QStringList resultList;
    QString parentVirtualPath = feature.getFullVirtualPath();
    QString resultName;

    qDebug() << "start";
    foreach (UBFeature curFeature, *featuresList) {

        if (curFeature.getFullVirtualPath().startsWith(feature.getFullVirtualPath())) {

            QString curResultName = curFeature.getFullVirtualPath();
            if (!parentVirtualPath.endsWith("/")) {
                parentVirtualPath.append("/");
            }
            //Cut virtual path prevfix
            int i = curResultName.indexOf(feature.getFullVirtualPath());
            if (i != -1) {
                curResultName = curResultName.right(curFeature.getFullVirtualPath().count() - i - parentVirtualPath.count());
            }
            //if directory has children, emptying the name;
            i = curResultName.indexOf("/");
            if (i != -1) {
                curResultName = "";
            }

            if (!curResultName.isEmpty()) {
                resultList.append(curResultName);
            }

            qDebug() << curResultName;
        }
    }

    if (!resultList.contains(pName + pExtention, Qt::CaseInsensitive)) {
        resultName = pName + pExtention;

    } else {
        for (int i = 0; i < 16777215; i++) {
            QString probeName = pName + "_" + QString::number(i) + pExtention;
            if (!resultList.contains(probeName, Qt::CaseInsensitive)) {
                resultName = probeName;
                break;
            }
        }
    }
    qDebug() << "result name is " << resultName;

    return resultName;
}

QString UBFeaturesController::adjustName(const QString &str)
{
    if (str.isNull()) {
        return QString();
    }

    QString resultStr = str;
    QRegExp invalidSymbols("[\\/\\s\\:\\?\\*\\|\\<\\>\\\"]+");

    return resultStr.replace(invalidSymbols, "_");
}

void UBFeaturesController::addToFavorite( const QUrl &path )
{
    QString filePath = fileNameFromUrl( path );
    if ( favoriteSet->find( path ) == favoriteSet->end() )
    {
        QFileInfo fileInfo( filePath );
        QString fileName = fileInfo.fileName();
        UBFeatureElementType type = fileTypeFromUrl(fileInfo.absoluteFilePath());
        UBFeature elem(favoriteData.categoryFeature().getFullVirtualPath() + "/" + fileName, getIcon(filePath, type), fileName, path, fileTypeFromUrl(filePath) );
        favoriteSet->insert( path );
        saveFavoriteList();

        if ( !elem.getVirtualPath().isEmpty() && !elem.getVirtualPath().isNull())
        featuresModel->addItem( elem );
    }
}

void UBFeaturesController::removeFromFavorite( const QUrl &path, bool deleteManualy)
{
    if ( favoriteSet->find( path ) != favoriteSet->end() )
    {
        favoriteSet->erase( favoriteSet->find( path ) );
        saveFavoriteList();
    }

    if (deleteManualy) {
        featuresModel->deleteFavoriteItem(path.toString());
    }
}

QString UBFeaturesController::fileNameFromUrl( const QUrl &url )
{
    QString fileName = url.toString();
    if ( fileName.contains( "uniboardTool://" ) )
        return fileName;
    return url.toLocalFile();
}


UBFeatureElementType UBFeaturesController::fileTypeFromUrl(const QString &path)
{
    QFileInfo fileInfo(path);

    if ( path.contains("uniboardTool://"))
        return FEATURE_INTERNAL;

    if (!fileInfo.exists()) {
        return FEATURE_INVALID;
    }

    UBFeatureElementType fileType = FEATURE_INVALID;
    QString fileName = fileInfo.fileName();
    QString mimeString = UBFileSystemUtils::mimeTypeFromFileName(fileName);

    if ( mimeString.contains("application")) {
        if (mimeString.contains("application/search")) {
            fileType = FEATURE_SEARCH;
        } else if (mimeString.contains("application/x-shockwave-flash")) {
            fileType = FEATURE_FLASH;
        } else {
            fileType = FEATURE_INTERACTIVE;
        }
    } else if (mimeString.contains("audio")) {
        fileType = FEATURE_AUDIO;
    } else if (mimeString.contains("video")) {
        fileType = FEATURE_VIDEO;
    } else if (mimeString.contains("image")) {
        fileType = FEATURE_IMAGE;
    } else if (mimeString.contains("bookmark")) {
        fileType = FEATURE_BOOKMARK;
    } else if (mimeString.contains("link")){
        fileType = FEATURE_LINK;
    } else if (fileInfo.isDir()) {
        fileType = FEATURE_FOLDER;
    } else {
        fileType = FEATURE_INVALID;
    }

    return fileType;
}

QImage UBFeaturesController::getIcon(const QString &path, UBFeatureElementType pFType = FEATURE_INVALID)
{
    if (pFType == FEATURE_FOLDER) {
        return QImage(":images/libpalette/folder.svg");
    } else if (pFType == FEATURE_INTERACTIVE || pFType == FEATURE_SEARCH) {
        return QImage(UBGraphicsWidgetItem::iconFilePath(QUrl::fromLocalFile(path)));
    } else if (pFType == FEATURE_INTERNAL) {
        return QImage(UBToolsManager::manager()->iconFromToolId(path));
    } else if (pFType == FEATURE_FLASH) {
        return QImage(":images/libpalette/FlashIcon.svg");
    } else if (pFType == FEATURE_AUDIO) {
        return QImage(":images/libpalette/soundIcon.svg");
    } else if (pFType == FEATURE_VIDEO) {
        return QImage(":images/libpalette/movieIcon.svg");
    } else if (pFType == FEATURE_BOOKMARK) {
        return QImage(":images/libpalette/bookmarkIcon.svg");
    } else if (pFType == FEATURE_LINK) {
        return QImage(":images/trapFlashPalette/trappedIcon.svg");
    }
    else if (pFType == FEATURE_IMAGE) {
            QImage pix(path);
        if (pix.isNull()) {
            pix = QImage(":images/libpalette/notFound.png");
        } else {
            pix = pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth, pix.width()));
        }
        return pix;
    }

    return QImage(":images/libpalette/notFound.png");
}

QString UBFeaturesController::categoryNameForVirtualPath(const QString &str)
{
    QString result;
    int ind = str.lastIndexOf("/");
    if (ind != -1) {
        result = str.right(str.count() - ind - 1);
    }
    return result;
}

QImage UBFeaturesController::createThumbnail(const QString &path)
{
    QString thumbnailPath = path;
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName(path);

    if (mimetype.contains("audio" )) {
        thumbnailPath = ":images/libpalette/soundIcon.svg";
    }
    else if (mimetype.contains("video")) {
        thumbnailPath = ":images/libpalette/movieIcon.svg";
    }
    else if(mimetype.contains("bookmark")){
        thumbnailPath = ":images/libpalette/bookmarkIcon.svg";
    }
    else if(mimetype.contains("link")){
        thumbnailPath = ":images/trapFlashPalette/trappedIcon.svg";
    }
    else {
        QImage pix(path);
        if (!pix.isNull()) {
            pix = pix.scaledToWidth(qMin(UBSettings::maxThumbnailWidth, pix.width()), Qt::SmoothTransformation);
            return pix;

        } else {
            thumbnailPath = ":images/libpalette/notFound.png";
        }

    }

    return QImage(thumbnailPath);
}

void UBFeaturesController::importImage(const QImage &image, const QString &fileName)
{
    importImage(image, currentElement, fileName);
}

void UBFeaturesController::createBookmark(const QString& fileName, const QString &urlString)
{
    QString lFileName = UBFileSystemUtils::cleanName(fileName);
    QString bookmarkDirectory = bookmarkData.categoryFeature().getFullPath().toLocalFile() + "/";
    QString mFileName = bookmarkDirectory + lFileName + ".bkm";
    int counter = 1;
    while(QFileInfo(mFileName).exists()){
        if(counter == 1)
            mFileName=mFileName.replace(QString(".bkm"),"");
        else
            mFileName=mFileName.replace(QString("-%1.bkm").arg(counter++),"");
        mFileName=mFileName.append(QString("-%1.bkm").arg(counter));
    }

    QFile file(mFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(urlString.toAscii());
    file.close();

    QImage thumb = createThumbnail(mFileName);
    UBFeature resultItem =  UBFeature(bookmarkData.categoryFeature().getFullVirtualPath() + "/" + mFileName.replace(bookmarkDirectory,""), thumb , mFileName, QUrl::fromLocalFile(mFileName), FEATURE_BOOKMARK );

    featuresModel->addItem(resultItem);

}

void UBFeaturesController::createLink(const QString& fileName, const QString& urlString, QSize& size, QString mimeType, QString bEmbedCode)
{
    QString name = fileName;
    if(name.indexOf(".") != -1)
        name = name.left(name.indexOf("."));

    CategoryData categoryData = getDestinationCategoryForMimeType((!mimeType.isNull() ? mimeType : UBFileSystemUtils::mimeTypeFromFileName(urlString)));
    QString lFileName = categoryData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name + ".lnk";

    int counter = 1;
    while(QFileInfo(lFileName).exists()){
        if(counter == 1)
            lFileName=lFileName.replace(QString(".lnk"),"");
        else
            lFileName=lFileName.replace(QString("-%1.lnk").arg(counter++),"");
        lFileName=lFileName.append(QString("-%1.lnk").arg(counter));
    }

    QFile file(lFileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QXmlStreamWriter xmlWriter(&file);

    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("link");

    xmlWriter.writeTextElement("src", urlString);
    xmlWriter.writeTextElement("width", QString("%1").arg(size.width()));
    xmlWriter.writeTextElement("height", QString("%1").arg(size.height()));
    xmlWriter.writeTextElement("html", bEmbedCode);

    xmlWriter.writeEndElement();

    xmlWriter.writeEndDocument();


    file.close();

    QImage thumb = createThumbnail(lFileName);
    UBFeature resultItem =  UBFeature(categoryData.categoryFeature().getFullVirtualPath() + "/" + lFileName.replace(categoryData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/",""), thumb , lFileName, QUrl::fromLocalFile(lFileName), FEATURE_LINK );

    featuresModel->addItem(resultItem);
}

void UBFeaturesController::importImage( const QImage &image, const UBFeature &destination, const QString &fileName )
{
    QString mFileName = fileName;
    QString filePath;
    UBFeature dest = destination;
    if (mFileName.isNull()) {
        QDateTime now = QDateTime::currentDateTime();
        static int imageCounter = 0;
        mFileName  = tr("ImportedImage") + "-" + now.toString("dd-MM-yyyy hh-mm-ss");

        filePath = picturesData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + mFileName;

        if (QFile::exists(filePath+".png"))
            mFileName += QString("-[%1]").arg(++imageCounter);
        else
            imageCounter = 0;

        mFileName += ".png";
    }

    dest = picturesData.categoryFeature();
    filePath = picturesData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + mFileName;
    image.save(filePath);

    QImage thumb = createThumbnail( filePath );
    UBFeature resultItem =  UBFeature( dest.getFullVirtualPath() + "/" + mFileName, thumb, mFileName,
        QUrl::fromLocalFile( filePath ), FEATURE_ITEM );

    featuresModel->addItem(resultItem);

}

QStringList UBFeaturesController::getFileNamesInFolders()
{
    QStringList strList;

    Q_ASSERT(curListModel);

    for (int i = 0; i < curListModel->rowCount(QModelIndex()); i++) {
        QModelIndex ind = curListModel->index(i, 0);
        if (!ind.isValid()) {
            qDebug() << "incorrect model index catched";
            continue;
        }
        UBFeature curFeature = curListModel->data(ind, Qt::UserRole + 1).value<UBFeature>();
        if (curFeature.getType() == FEATURE_FOLDER) {
            strList << QFileInfo(curFeature.getFullPath().toLocalFile()).fileName();
        }
    }

    return strList;
}

void UBFeaturesController::addNewFolder(QString name)
{
    QString path = currentElement.getFullPath().toLocalFile() + "/" + name;

    if(currentElement.getFullVirtualPath() == audiosData.categoryFeature().getFullVirtualPath())
        path = audiosData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name;
    else if(currentElement.getFullVirtualPath() == moviesData.categoryFeature().getFullVirtualPath())
        path = moviesData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name;
    else if(currentElement.getFullVirtualPath() == picturesData.categoryFeature().getFullVirtualPath())
        path = picturesData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name;
    else if(currentElement.getFullVirtualPath() == flashData.categoryFeature().getFullVirtualPath())
        path = flashData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name;
    else if(currentElement.getFullVirtualPath() == interactivityData.categoryFeature().getFullVirtualPath())
        path = interactivityData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name;
    else if(currentElement.getFullVirtualPath() == appData.categoryFeature().getFullVirtualPath())
        path = appData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + name;

    if ("/root" == currentElement.getFullVirtualPath())
    {
        return;
    }

    if(!QFileInfo(path).exists()) {
        QDir().mkpath(path);
    }
    UBFeature newFeatureFolder = UBFeature( currentElement.getFullVirtualPath() + "/" + name, QImage(":images/libpalette/folder.svg"),
                                            name, QUrl::fromLocalFile( path ), FEATURE_FOLDER );

    featuresModel->addItem(newFeatureFolder);
    featuresProxyModel->invalidate();
}

void UBFeaturesController::addItemToPage(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath() );
}

void UBFeaturesController::addItemAsBackground(const UBFeature &item)
{
    UBApplication::boardController->downloadURL( item.getFullPath(), QString(), QPointF(), QSize(), true );
}

CategoryData UBFeaturesController::getDestinationCategoryForUrl( const QUrl &url )
{
    QString mimetype = UBFileSystemUtils::mimeTypeFromFileName( url.toString() );
    return getDestinationCategoryForMimeType(mimetype);
}

CategoryData UBFeaturesController::getDestinationCategoryForMimeType(const QString &pMmimeType,const QUrl& sourceUrl)
{
    if ( pMmimeType.contains("audio") )
        return audiosData;
    if ( pMmimeType.contains("video") )
        return moviesData;
    else if ( pMmimeType.contains("image") || pMmimeType.isEmpty())
        return picturesData;
    else if ( pMmimeType.contains("application") )
    {
        if ( pMmimeType.contains( "x-shockwave-flash") )
            return flashData;
        else{
            QString source = sourceUrl.toString();
            if(source.length()){
                if(source.contains("?type=application"))
                    return appData;
                else
                    return interactivityData;
            }
            else
                return webFolderData;
        }
    }

    return CategoryData();
}

QString UBFeaturesController::getFeaturePathByName(const QString &featureName) const
{
    QString videoPickerWidgetPath;
    foreach (UBFeature curFeature, *featuresList)
    {
        if (curFeature.getName().contains(featureName))
            videoPickerWidgetPath = curFeature.getFullPath().toLocalFile();
    }
    Q_ASSERT(!videoPickerWidgetPath.isEmpty());
    return videoPickerWidgetPath;
}

void UBFeaturesController::addDownloadedFile(const QUrl &sourceUrl, const QByteArray &pData, const QString pContentSource, const QString pTitle)
{
    CategoryData destData = getDestinationCategoryForMimeType(pContentSource,sourceUrl);

    if (destData.isNull()) {
        return;
    }

    UBFeature dest = destData.categoryFeature();

    if (dest == UBFeature())
        return;

    QString fileName;
    QString filePath;

    if(dest == picturesData.categoryFeature()) {

        QString UniqName = uniqNameForFeature(dest, adjustName(pTitle), ".jpg");
        fileName =  !UniqName.isNull()
                ? UniqName
                : tr("ImportedImage") + "-" + QDateTime::currentDateTime().toString("dd-MM-yyyy hh-mm-ss")+ ".jpg";

        filePath = picturesData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + fileName;

        QImage img;
        img.loadFromData(pData);
        importImage(img, fileName);
    }
    else {
        QString url = sourceUrl.toString();
        if(url.indexOf("?") != -1){
            url = url.left(url.indexOf("?"));
            fileName = pTitle.isEmpty() ? QFileInfo( url ).fileName() : pTitle;
            filePath = QDir::tempPath() + "/" + QUuid::createUuid();
        }
        else{
            fileName = pTitle.isEmpty() ? QFileInfo( url ).fileName() : pTitle;
            filePath = destData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + fileName;
        }

        QFile file( filePath );
        if ( file.open(QIODevice::WriteOnly ))
        {
            file.write(pData);
            file.close();

            if(sourceUrl.toString().indexOf("?") != -1){
                // this is a zipped widget extract it
                filePath = QDir::tempPath() + "/" + fileName;
                UBFileSystemUtils::expandZipToDir(file, QDir(filePath));
                // check if it something like xxxx.wgt/xxx.wgt situation due to a wrong way to zip widgets
                QStringList list = QDir(QDir::tempPath() + "/" + fileName).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
                QString tmpWidgetDirectory = filePath;
                if(list.count() == 1 && list.at(0).contains(".wgt"))
                    tmpWidgetDirectory = filePath + "/" + list.at(0);
                filePath = destData.pathData().value(CategoryData::UserDefined).toLocalFile() + "/" + fileName;
                UBFileSystemUtils::copyDir(tmpWidgetDirectory, filePath);
            }


            UBFeature downloadedFeature = UBFeature(dest.getFullVirtualPath() + "/" + fileName, getIcon( filePath, fileTypeFromUrl(filePath)),
                                                    fileName, QUrl::fromLocalFile(filePath), FEATURE_ITEM);
            if (downloadedFeature != UBFeature()) {
                featuresModel->addItem(downloadedFeature);
            }
        }
    }

}

UBFeature UBFeaturesController::moveItemToFolder( const QUrl &url, const UBFeature &destination )
{
    QString sourcePath = url.toLocalFile();

    Q_ASSERT( QFileInfo( sourcePath ).exists() );

    CategoryData destData = getDestinationCategoryForUrl(url);

    if (destData.isNull()) {
        return UBFeature();
    }

    UBFeature possibleDest = destData.categoryFeature();

    UBFeature dest = destination;

    if ( destination != trashData.categoryFeature() &&
        !destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive ) )
    {
        dest = possibleDest;
    }

    QString name = QFileInfo( sourcePath ).fileName();
    QString destPath = dest.getFullPath().toLocalFile();
    QString destVirtualPath = dest.getFullVirtualPath();
    QString newFullPath = destPath + "/" + name;
    if ( sourcePath.compare( newFullPath, Qt::CaseInsensitive ) )
    {
        QFile( sourcePath ).copy( newFullPath );
        deleteItem( url );
    }

    QImage thumb = getIcon( newFullPath );

    UBFeatureElementType type = FEATURE_ITEM;
    if ( UBFileSystemUtils::mimeTypeFromFileName( newFullPath ).contains("application") )
        type = FEATURE_INTERACTIVE;
    UBFeature newElement( destVirtualPath + "/" + name, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
    return newElement;
}

void UBFeaturesController::rescanModel()
{
    featuresModel->removeRows(0, featuresList->count());

    //Could implement infolder scanning for better perfomance
    loadHardcodedItemsToModel();
    refreshModels();
}

void UBFeaturesController::siftElements(const QString &pSiftValue)
{
    featuresProxyModel->setFilterFixedString(pSiftValue);
    featuresProxyModel->invalidate();

    featuresPathModel->setPath(pSiftValue);
    featuresPathModel->invalidate();

    if (currentElement != rootData.categoryFeature()) {
        featuresProxyModel->sort(0);
    }
}

UBFeature UBFeaturesController::getFeature(const QModelIndex &index, const QString &listName)
{
//    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel *>(pOnView->model());
    QAbstractItemModel *model = 0;
    if (listName == UBFeaturesWidget::objNamePathList) {
        model = featuresPathModel;
    } else if (listName == UBFeaturesWidget::objNameFeatureList) {
        model = curListModel;
    }

    if (model) {
        return model->data(index, Qt::UserRole + 1).value<UBFeature>();
    }

    return UBFeature();


//    return pOnView->model()->data(index, Qt::UserRole + 1).value<UBFeature>();  /*featuresSearchModel->data(index, Qt::UserRole + 1).value<UBFeature>()*/;
}

void UBFeaturesController::searchStarted(const QString &pattern, QListView *pOnView)
{
    if (pattern.isEmpty()) {

        pOnView->setModel(featuresProxyModel);
        featuresProxyModel->invalidate();
        curListModel = featuresProxyModel;
    } else if ( pattern.size() > 1 ) {

        //        featuresSearchModel->setFilterPrefix(currentElement.getFullVirtualPath());
        featuresSearchModel->setFilterWildcard( "*" + pattern + "*" );
        pOnView->setModel(featuresSearchModel );
        featuresSearchModel->invalidate();
        curListModel = featuresSearchModel;
    }
}

void UBFeaturesController::refreshModels()
{
    featuresProxyModel->invalidate();
    featuresSearchModel->invalidate();
    featuresPathModel->invalidate();
}

UBFeature UBFeaturesController::copyItemToFolder( const QUrl &url, const UBFeature &destination )
{
    QString sourcePath = url.toLocalFile();

    Q_ASSERT( QFileInfo( sourcePath ).exists() );

    CategoryData destData = getDestinationCategoryForUrl(url);

    if (destData.isNull()) {
        return UBFeature();
    }

    UBFeature possibleDest = destData.categoryFeature();
    UBFeature dest = destination;

    if ( destination != trashData.categoryFeature() &&
        !destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive ) )
    {
        dest = possibleDest;
    }

    QString name = QFileInfo(sourcePath).fileName();
    QString destPath = dest.getFullPath().toLocalFile();
    QString destVirtualPath = dest.getFullVirtualPath();
    QString newFullPath = destPath + "/" + name;
    if (!sourcePath.compare(newFullPath, Qt::CaseInsensitive))
        QFile(sourcePath).copy(newFullPath);

    QImage thumb = getIcon(newFullPath);

    UBFeatureElementType type = FEATURE_ITEM;
    if (UBFileSystemUtils::mimeTypeFromFileName(newFullPath).contains("application"))
        type = FEATURE_INTERACTIVE;
    UBFeature newElement( destVirtualPath + "/" + name, thumb, name, QUrl::fromLocalFile( newFullPath ), type );
    return newElement;
}

QString UBFeaturesController::moveExternalData(const QUrl &url, const UBFeature &destination)
{
    QString sourcePath = url.toLocalFile();

    Q_ASSERT( QFileInfo(sourcePath ).exists());

    CategoryData destData = getDestinationCategoryForUrl(url);

    if (destData.isNull()) {
        return QString();
    }

    UBFeature possibleDest = destData.categoryFeature();
    UBFeature dest = destination;

    if ( destination != trashData.categoryFeature() && destination != UBFeature()
       /*&& !destination.getFullVirtualPath().startsWith( possibleDest.getFullVirtualPath(), Qt::CaseInsensitive )*/ )
    {
        dest = possibleDest;
    }

    UBFeatureElementType type = fileTypeFromUrl(sourcePath);

    if (type == FEATURE_FOLDER) {
        return QString();
    }

    QString name = QFileInfo(sourcePath).fileName();
    QString destPath = dest.getFullPath().toLocalFile();

    if(dest == picturesData.categoryFeature())
        destPath =  picturesData.pathData().value(CategoryData::UserDefined).toLocalFile();
    if(dest == audiosData.categoryFeature())
        destPath =  audiosData.pathData().value(CategoryData::UserDefined).toLocalFile();
    if(dest == moviesData.categoryFeature())
        destPath =  moviesData.pathData().value(CategoryData::UserDefined).toLocalFile();

    QString destVirtualPath = dest.getFullVirtualPath();
    QString newFullPath = destPath + "/" + name;

    if (!sourcePath.compare(newFullPath, Qt::CaseInsensitive) || !UBFileSystemUtils::copy(sourcePath, newFullPath)) {
        return QString();
    }

    Q_ASSERT(QFileInfo(newFullPath).exists());

    QImage thumb = getIcon(newFullPath, type);
    UBFeature newElement(destVirtualPath + "/" + name, thumb, name, QUrl::fromLocalFile(newFullPath), type);

    featuresModel->addItem(newElement);

    return newFullPath;
}

void UBFeaturesController::deleteItem(const QUrl &url)
{
    QString path = url.toLocalFile();
    Q_ASSERT( QFileInfo( path ).exists() );

    QString thumbnailPath = UBFileSystemUtils::thumbnailPath( path );
    if ( thumbnailPath.length() && QFileInfo( thumbnailPath ).exists()) {
        if (QFileInfo(thumbnailPath).isFile()) {
            QFile::remove(thumbnailPath);
        } else if (QFileInfo(thumbnailPath).isDir()){
            if (!UBFileSystemUtils::deleteDir(thumbnailPath)) {
                qDebug() << "Not able to delete directory";
             }
        }
    }
    if (QFileInfo(path).isFile()) {
        QFile::remove( path );

    } else if (QFileInfo(path).isDir()) {
        UBFileSystemUtils::deleteDir(path);
    }
}

void UBFeaturesController::deleteItem(const UBFeature &pFeature)
{
    QUrl sourceUrl = pFeature.getFullPath();
    featuresModel->deleteItem(pFeature);
    deleteItem(sourceUrl);
}

bool UBFeaturesController::isTrash( const QUrl &url )
{
    return url.toLocalFile().startsWith(trashData.categoryFeature().getFullPath().toLocalFile());
}
void UBFeaturesController::moveToTrash(UBFeature feature, bool deleteManualy)
{
    featuresModel->moveData(feature, trashData.categoryFeature(), Qt::MoveAction, deleteManualy);
    removeFromFavorite(feature.getFullPath(), true);
    featuresModel->deleteFavoriteItem(UBFeaturesController::fileNameFromUrl(feature.getFullPath()));
}

UBFeaturesController::~UBFeaturesController()
{
    if (featuresList) {
        delete featuresList;
    }
}

void UBFeaturesController::assignFeaturesListView(UBFeaturesListView *pList)
{
    pList->setDragDropMode( QAbstractItemView::DragDrop );
    pList->setSelectionMode( QAbstractItemView::ContiguousSelection );

    pList->setResizeMode( QListView::Adjust );
    pList->setViewMode( QListView::IconMode );

    pList->setIconSize(QSize(UBFeaturesWidget::defaultThumbnailSize, UBFeaturesWidget::defaultThumbnailSize));
    pList->setGridSize(QSize(UBFeaturesWidget::defaultThumbnailSize + 20, UBFeaturesWidget::defaultThumbnailSize + 20));

    itemDelegate = new UBFeaturesItemDelegate(this, pList);
    pList->setItemDelegate(itemDelegate);

    pList->setModel(featuresProxyModel);
    curListModel = featuresProxyModel;
}

void UBFeaturesController::assignPathListView(UBFeaturesListView *pList)
{
    pList->setViewMode(QListView::IconMode );
    pList->setIconSize(QSize(UBFeaturesWidget::defaultThumbnailSize - 10, UBFeaturesWidget::defaultThumbnailSize - 10));
    pList->setGridSize(QSize(UBFeaturesWidget::defaultThumbnailSize + 10, UBFeaturesWidget::defaultThumbnailSize - 10));
    pList->setFixedHeight(60);
    pList->setSelectionMode(QAbstractItemView::NoSelection);
    pList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    pList->setFlow(QListView::LeftToRight);
    pList->setWrapping(false);
    pList->setDragDropMode(QAbstractItemView::DropOnly);

    pList->setModel( featuresPathModel);

    pathItemDelegate = new UBFeaturesPathItemDelegate(this);
    pList->setItemDelegate(pathItemDelegate);
}


