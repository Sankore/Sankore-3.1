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



#ifndef UBFEATURESCONTROLLER_H
#define UBFEATURESCONTROLLER_H

#include <QMetaType>
#include <QObject>
#include <QWidget>
#include <QSet>
#include <QVector>
#include <QString>
#include <QPixmap>
#include <QMap>
#include <QUrl>
#include <QByteArray>
#include <QtGui>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "frameworks/UBTrashRegistery.h"

class UBFeaturesModel;
class UBFeaturesItemDelegate;
class UBFeaturesPathItemDelegate;
class UBFeaturesProxyModel;
class UBFeaturesSearchProxyModel;
class UBFeaturesPathProxyModel;
class UBFeaturesListView;
class UBFeature;


enum UBFeatureElementType
{
    FEATURE_CATEGORY,
    FEATURE_VIRTUALFOLDER,
    FEATURE_FOLDER,
    FEATURE_INTERACTIVE,
    FEATURE_INTERNAL,
    FEATURE_ITEM,
    FEATURE_AUDIO,
    FEATURE_VIDEO,
    FEATURE_IMAGE,
    FEATURE_FLASH,
    FEATURE_TRASH,
    FEATURE_FAVORITE,
    FEATURE_SEARCH,
    FEATURE_BOOKMARK,
    FEATURE_LINK,
    FEATURE_INVALID,
    FEATURE_RTE
};

enum UBFeatureBackgroundDisposition
{
    Center,
    Adjust,
    Mosaic,
    Fill,
    Extend
};

class UBFeature
{
public:
    enum Permission {
        NO_P      = 0x0  //0000
        ,DELETE_P = 0x1  //0001
        , WRITE_P = 0x2  //0010
        , ALL_P   = 0xF  //1111
    };
    Q_DECLARE_FLAGS(Permissions, Permission)

    UBFeature() {;}
//    UBFeature(const UBFeature &f);
    UBFeature(const QString &url
              , const QImage &icon
              , const QString &name
              , const QUrl &realPath
              , UBFeatureElementType type = FEATURE_CATEGORY
              , Permissions pOwnPermissions = ALL_P
              , QString pSortKey = QString());
//    UBFeature();
    virtual ~UBFeature();
    QString getName() const { return mName; }


    QString getDisplayName() const {return mDisplayName;}
    QImage getThumbnail() const {return mThumbnail;}
    QString getVirtualPath() const { return virtualDir; }
    //QString getPath() const { return mPath; };
    QUrl getFullPath() const { return mPath; }
    QString getFullVirtualPath() const { return  virtualDir + "/" + mName; }
    QString getUrl() const;
    QString getSortKey() const {return mSortKey;}
    void setFullPath(const QUrl &newPath) {mPath = newPath;}
    void setFullVirtualPath(const QString &newVirtualPath) {virtualDir = newVirtualPath;}

    //issue 1474 - NNE - 20131121
    /**
      * Change the current name of the feature.
      * @arg newName The new name of the feature.
      */
    void setName(const QString &newName);

    UBFeatureElementType getType() const { return elementType; }
    UBFeature &markedWithSortKey(const QString &str);

    // Issue 1684 - CFA - 20131125
    const UBFeatureBackgroundDisposition& backgroundDisposition() const;
    void setBackgroundDisposition(UBFeatureBackgroundDisposition disposition);


    bool isFolder() const;
    bool allowedCopy() const;
    bool isDeletable() const;
    bool inTrash() const;
    Permissions getPermissions() const {return mOwnPermissions;}
    bool testPermissions(Permissions pPermissions) const {return mOwnPermissions & pPermissions;}
    void setPermissions(Permissions pPermissions) {mOwnPermissions = pPermissions;}
    void addPermissions(Permissions pPermissions) {mOwnPermissions |= pPermissions;}
    void unsetPermissions(Permissions pPermissions) {mOwnPermissions &= ~pPermissions;}
    bool operator ==( const UBFeature &f )const;
    bool operator !=( const UBFeature &f )const;
    const QMap<QString,QString> & getMetadata() const { return metadata; }
    void setMetadata( const QMap<QString,QString> &data ) { metadata = data; }

private:
    QString getNameFromVirtualPath(const QString &pVirtPath);
    QString getVirtualDirFromVirtualPath(const QString &pVirtPath);

private:
    QString virtualDir;
    QString virtualPath;
    QImage mThumbnail;
    QString mName;
    QString mDisplayName;
    QUrl mPath;
    UBFeatureElementType elementType;
    QMap<QString,QString> metadata;
    Permissions mOwnPermissions;
    QString mSortKey;
    UBFeatureBackgroundDisposition mDisposition;
};
Q_DECLARE_METATYPE( UBFeature )
Q_DECLARE_OPERATORS_FOR_FLAGS(UBFeature::Permissions)

//Describe files in virtual file system
struct ComputingData
{
    ComputingData (const QUrl &pPath, UBFeature pFeature,UBFeature::Permissions pPermissions)
        : path(pPath)
        , feature(pFeature)
        , permissions(pPermissions)
    {;}
    ComputingData() {;}

    QUrl path;
    //Permissions for all subdirectories
    UBFeature feature;
    UBFeature::Permissions permissions;
};



// Data class containing all the data about hardcoded items from library palette
struct CategoryData
{
    enum pathType {
        Library = 0
        , UserDefined = 1
    };
    CategoryData() : mIsNull(true) {;}

    struct PathData : public QMultiMap<pathType, QUrl>
    {
        PathData& insertr (pathType pType, QUrl pUrl) {
            insertMulti(pType, pUrl);
            return *this;
        }
    };

    struct PermissionsData : public QMap<pathType, UBFeature::Permissions>
    {
        PermissionsData& insertr (pathType pType, UBFeature::Permissions pPermissions) {
            insert(pType, pPermissions);
            return *this;
        }
    };

    CategoryData (const PathData &pPathData
                  , UBFeature pFeature
                  , PermissionsData psubFolderPermissions)
        : mPathData(pPathData)
        , mCategoryFeature(pFeature)
        , mSubFolderPermissions(psubFolderPermissions)
        , mIsNull(false)
    {;}

    PathData pathData() const {return mPathData;}
    UBFeature categoryFeature() const {return mCategoryFeature;}
//    UBFeature::Permissions subFolderPermissions() const {return mSubFolderPermissions;}
    UBFeature::Permissions librarySubPermissions() const {return mSubFolderPermissions.value(Library, UBFeature::ALL_P);}
    UBFeature::Permissions UserSubPermissions() const {return mSubFolderPermissions.value(UserDefined, UBFeature::ALL_P);}
    UBFeature::Permissions subFolderPermissionsForType(pathType pType) const {return mSubFolderPermissions.value(pType, UBFeature::ALL_P);}
    bool isNull() const {return mIsNull;}

private:
    PathData mPathData;
    //Permissions for all subdirectories
    UBFeature mCategoryFeature;
//    UBFeature::Permissions mSubFolderPermissions;
    PermissionsData mSubFolderPermissions;
    bool mIsNull;
};

class UBFeaturesComputingThread : public QThread
{
    Q_OBJECT
public:
    explicit UBFeaturesComputingThread(QObject *parent = 0);
    virtual ~UBFeaturesComputingThread();
    void compute(const QList<CategoryData> &pcategoryList, const QPair<CategoryData, QSet<QUrl> > &pFavoriteInfo, const QList<CategoryData> &extData);

protected:
    void run();

signals:
    void sendFeature(UBFeature pFeature);
    void featureSent();
    void scanStarted();
    void scanFinished();
    void maxFilesCountEvaluated(int max);
    void scanCategory(const QString &str);
    void scanPath(const QString &str);

public slots:

private:
    void scanFS(const QUrl & currentPath
                , const QString & currVirtualPath
                , const QPair<CategoryData, QSet<QUrl> > &pfavoriteInfo
                , UBFeature::Permissions pPermissions
                , const QList<CategoryData> &extData);
    void scanAll(QList<CategoryData> pScanningData
                 , const QPair<CategoryData
                 , QSet<QUrl> > &pFavoriteInfo
                 , const QList<CategoryData> &extData);
    int featuresCount(const QUrl &pPath);
    int featuresCountAll(QList<CategoryData> pScanningData);

private:
    QMutex mMutex;
    QWaitCondition mWaitCondition;
    QUrl mScanningPath;
    QString mScanningVirtualPath;
    QList<CategoryData> mScanningData;
    QList<CategoryData> mExceptionData;
    QPair<CategoryData, QSet<QUrl> > mFavoriteInfo;
    bool restart;
    bool abort;
};



class UBFeaturesController : public QObject
{
friend class UBFeaturesWidget;

Q_OBJECT

public:
    UBFeaturesController(QWidget *parentWidget);
    virtual ~UBFeaturesController();

    QList <UBFeature>* getFeatures() const {return featuresList;}

    void initHardcodedData();
    void loadHardcodedItemsToModel();

    void addItemToPage(const UBFeature &item);
    void addItemAsBackground(UBFeature &item, bool isFromPalette = true);
    void addItemAsDefaultBackground(UBFeature &item, bool isFromPalette = true);
    const UBFeature& getCurrentElement()const {return currentElement;}
    void setCurrentElement( const UBFeature &elem ) {currentElement = elem;}
    UBFeature getTrashElement () const { return trashData.categoryFeature(); }
    UBFeature getBookmarkElement() const {return bookmarkData.categoryFeature();}
    UBFeature getRootElement() const {return rootData.categoryFeature();}

    void addDownloadedFile( const QUrl &sourceUrl, const QByteArray &pData, const QString pContentSource, const QString pTitle );

    UBFeature moveItemToFolder( const QUrl &url, const UBFeature &destination );
    UBFeature copyItemToFolder( const QUrl &url, const UBFeature &destination );
    QString moveExternalData(const QUrl &url, const UBFeature &destination);

    void rescanModel();
    void siftElements(const QString &pSiftValue);
    //TODO make less complicated for betteer maintainence
    UBFeature getFeature(const QModelIndex &index, const QString &listName);
    void searchStarted(const QString &pattern, QListView *pOnView);
    void refreshModels();

    void deleteItem( const QUrl &url );
    void deleteItem(const UBFeature &pFeature);
    bool isTrash( const QUrl &url );
    void moveToTrash(UBFeature feature, bool deleteManualy = false);
    void addToFavorite( const QUrl &path );
    void removeFromFavorite(const QUrl &path, bool deleteManualy = false);
    void importImage(const QImage &image, const QString &fileName = QString());
    void importImage( const QImage &image, const UBFeature &destination, const QString &fileName = QString() );
    void createBookmark(const QString& fileName, const QString &urlString);
    void createLink(const QString& fileName, const QString& urlString, QSize &size, QString mimeType, QString bEmbedCode);
    bool newFolderAllowed() const {return currentElement.isFolder() && currentElement.testPermissions(UBFeature::WRITE_P);}
    QStringList getFileNamesInFolders();

    static UBFeatureElementType fileTypeFromUrl( const QString &path );

    static QString fileNameFromUrl( const QUrl &url );
    static QImage getIcon( const QString &path, UBFeatureElementType pFType );
    static char featureTypeSplitter() {return ':';}
    static QString categoryNameForVirtualPath(const QString &str);

    static const QString virtualRootName;

    //issue 1699 - NNE - 20140224
    static void setRTEIsLoaded(bool isLoaded);
    static bool RTEIsLoaded();
    //issue 1699 - NNE - 20140224 : END

    void assignFeaturesListView(UBFeaturesListView *pList);
    void assignPathListView(UBFeaturesListView *pList);

    //issue 1474 - NNE - 20131120
    /**
      * Simple wrapper that remove the associated entry of the feature in the trash registery.
      * @arg feature The feature to remove from the trash registery.
      */
    void removeFromTrashRegistery(UBFeature feature);

    //issue 1474 - NNE - 20131210
    /**
      * Return the translation for the category data.
      * This function exists to avoid to break previous translations by
      * move the tr() string.
      *
      * \param name The original name of the category data.
      * \return The name of the category data according to the curent user's language.
      */
    QString getTranslationNameForCategoryData(const QString& name) const;

    //issue 1474 - NNE - 20131213
    /**
      * Return the associated entry in the trash register
      *
      * \param name The name of the entry
      *
      * \return The associated entry if exist, otherwise null
      */
    RegisteryEntry getRegisteryEntry(const QString &name) const;

public:
    //Hardcoded toplevel data
    CategoryData rootData;
    CategoryData audiosData;
    CategoryData moviesData;
    CategoryData picturesData;
    CategoryData appData;
    CategoryData flashData;
    CategoryData shapesData;
    CategoryData interactivityData;
    CategoryData favoriteData;
    CategoryData webSearchData;
    CategoryData trashData;
    CategoryData webFolderData;
    CategoryData bookmarkData;

    QList<CategoryData> topLevelCategoryData;
    QList<CategoryData> extentionPermissionsCategoryData;

signals:
    void maxFilesCountEvaluated(int pLimit);
    void scanStarted();
    void scanFinished();
    void featureAddedFromThread();
    void scanCategory(const QString &);
    void scanPath(const QString &);

private slots:
    /**
      * Add a new folder in the currentFeature.
      * @arg name The name of the new folder.
      */
    void addNewFolder(QString name);

    //issue 1474 - NNE - 20131121
    /**
      * Add a new folder in the feature given in parameter.
      * @arg name The name of the new folder.
      * @arg from The feature where the folder will be added.
      */
    void addNewFolder(QString name, const UBFeature from);

    void scanFS();
    void createNpApiFeature(const QString &str);

//issue 1474 - NNE - 20131119
public slots:
    /**
      * Action performs when a feature or several features have to be restored.
      * @arg features The features to restore.
      */
    void restoreFeature(const QVector<UBFeature> features);

    bool restoreFolderFeature(UBFeature &feature, RegisteryEntry entry);

    bool restoreFileFeature(UBFeature &feature, RegisteryEntry entry);
//issue 1474 - NNE - 20131119 : END

private:

    UBFeaturesItemDelegate *itemDelegate;
    UBFeaturesPathItemDelegate *pathItemDelegate;

    UBFeaturesModel *featuresModel;
    UBFeaturesProxyModel *featuresProxyModel;
    UBFeaturesSearchProxyModel *featuresSearchModel;
    UBFeaturesPathProxyModel *featuresPathModel;

    QAbstractItemModel *curListModel;
    UBFeaturesComputingThread mCThread;

    //issue 1474 - NNE - 20131025
    UBTrashRegistery mTrashRegistery;///< This the trash registery. It is used to restore the elements in the trash.

    //issue 1699 - NNE - 20140224
    static bool mRTEIsLoaded;

private:

    static QImage createThumbnail(const QString &path);
    //void addImageToCurrentPage( const QString &path );
    void loadFavoriteList();
    void saveFavoriteList();
    QString uniqNameForFeature(const UBFeature &feature, const QString &pName = "Imported", const QString &pExtention = "") const;
    QString adjustName(const QString &str);

    QList <UBFeature> *featuresList;

    int mLastItemOffsetIndex;
    UBFeature currentElement;

    QSet <QUrl> *favoriteSet;

public:
    CategoryData getDestinationCategoryForUrl( const QUrl &url );
    CategoryData getDestinationCategoryForMimeType(const QString &pMmimeType, const QUrl &sourceUrl = QUrl());
    QString getFeaturePathByName(const QString &featureName) const;

    //issue 1474 - NNE - 20131119
    UBFeature getFeatureByPath(const QString &path) const;
    UBFeature getFeatureByFullPath(const QString &path) const;

};

#endif
