/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#include "UBDocumentController.h"

#include <QtCore>
#include <QtGui>

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "adaptors/UBExportPDF.h"
#include "adaptors/UBThumbnailAdaptor.h"

#include "adaptors/UBMetadataDcSubsetAdaptor.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBDrawingController.h"


#include "gui/UBThumbnailView.h"
#include "gui/UBDocumentTreeWidget.h"
#include "gui/UBMousePressFilter.h"
#include "gui/UBMessageWindow.h"
#include "gui/UBMainWindow.h"
#include "gui/UBDocumentToolsPalette.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsSvgItem.h"
#include "domain/UBGraphicsPixmapItem.h"

#include "document/UBDocumentProxy.h"

#include "ui_documents.h"
#include "ui_mainWindow.h"

#include "core/memcheck.h"

UBDocumentTreeNode::UBDocumentTreeNode(Type pType, const QString &pName, const QString &pDisplayName, UBDocumentProxy *pProxy ) :
    mType(pType)
  , mName(pName)
  , mDisplayName(pDisplayName)
  , mProxy(pProxy)
{
    if (pDisplayName.isEmpty()) {
        mDisplayName = mName;
    }
    mParent = 0;
}

void UBDocumentTreeNode::addChild(UBDocumentTreeNode *pChild)
{
    if (pChild) {
        mChildren += pChild;
        pChild->mParent = this;
    }
}

void UBDocumentTreeNode::removeChild(int index)
{
    if (index < 0 || index > mChildren.count() - 1) {
        return;
    }

    UBDocumentTreeNode *curChild = mChildren[index];
    while (curChild->mChildren.count()) {
        curChild->removeChild(0);
    }

    mChildren.removeAt(index);
}

UBDocumentTreeNode *UBDocumentTreeNode::moveTo(const QString &pPath)
{
    UBDocumentTreeNode *parentNode = this;
    QStringList pathList = pPath.split("/", QString::SkipEmptyParts);

    bool searchingNode = true;
    while (!pathList.isEmpty())
    {
        QString curLevelName = pathList.takeFirst();
        if (searchingNode) {
            searchingNode = false;
            foreach (UBDocumentTreeNode *curChild, parentNode->children()) {
                if (curChild->nodeName() == curLevelName) {
                    searchingNode = true;
                    parentNode = curChild;
                    break;
                }
            }
        }

        if (!searchingNode) {
            UBDocumentTreeNode *newChild = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, curLevelName);
            parentNode->addChild(newChild);
            parentNode = newChild;
        }
    }

    return parentNode;
}

UBDocumentTreeNode *UBDocumentTreeNode::clone()
{
    return new UBDocumentTreeNode(this->mType, this->mName, this->mDisplayName, this->mProxy);
}

QString UBDocumentTreeNode::dirPathInHierarchy()
{
    QString result;
    UBDocumentTreeNode *curNode = this;
    //protect the 2nd level items
    while (curNode->parentNode() && !curNode->isTopLevel()) {
        result.prepend(curNode->parentNode()->nodeName() + "/");
        curNode = curNode->parentNode();
    }

    if (result.endsWith("/")) {
        result.truncate(result.count() - 1);
    }

    return result;
}

UBDocumentTreeNode::~UBDocumentTreeNode()
{
    foreach (UBDocumentTreeNode *curChildren, mChildren) {
        delete(curChildren);
        curChildren = 0;
    }
    if (mProxy)
        delete mProxy;
}

UBDocumentTreeModel::UBDocumentTreeModel(QObject *parent) :
    QAbstractItemModel(parent)
  , mRootNode(0)
{
    UBDocumentTreeNode *rootNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, "root");

    QString trashName = UBSettings::trashedDocumentGroupNamePrefix;

    UBDocumentTreeNode *myDocsNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, UBPersistenceManager::myDocumentsName, tr("My documents"));
    rootNode->addChild(myDocsNode);
//    mMyDocuments = createIndex(0, 0, myDocsNode);
    UBDocumentTreeNode *modelsNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, UBPersistenceManager::modelsName, tr("Models"));
    rootNode->addChild(modelsNode);
//    mModels = createIndex(1, 0, modelsNode);
    UBDocumentTreeNode *trashNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, trashName, tr("Trash"));
    rootNode->addChild(trashNode);
//    mTrash = createIndex(2, 0, trashNode);
    UBDocumentTreeNode *untitledDocumentsNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, UBPersistenceManager::untitledDocumentsName, tr("Untitled documents"));
    myDocsNode->addChild(untitledDocumentsNode);
//    mUntitledDocuments = createIndex(0, 0, untitledDocumentsNode);

    setRootNode(rootNode);

    mMyDocuments =  index(0, 0, QModelIndex());
    mModels =  index(1, 0, QModelIndex());
    mTrash =  index(2, 0, QModelIndex());
    mUntitledDocuments = index(0, 0, mMyDocuments);
}

QModelIndex UBDocumentTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!mRootNode || row < 0 || column < 0) {
        return QModelIndex();
    }

    UBDocumentTreeNode *nodeParent = nodeFromIndex(parent);
    if (!nodeParent) {
        return QModelIndex();
    }

    UBDocumentTreeNode *requiredNode = nodeParent->children().at(row);
    if(!requiredNode) {
        return QModelIndex();
    }

    QModelIndex resIndex = createIndex(row, column, requiredNode);

    return resIndex;
}

QModelIndex UBDocumentTreeModel::parent(const QModelIndex &child) const
{
    UBDocumentTreeNode *nodeChild = nodeFromIndex(child);
    if (!nodeChild) {
        return QModelIndex();
    }

    UBDocumentTreeNode *nodeParent = nodeChild->parentNode();
    if (!nodeParent) {
        return QModelIndex();
    }

    UBDocumentTreeNode *nodePreParent = nodeParent->parentNode();
    if (!nodePreParent) {
        return QModelIndex();
    }

    int row = nodePreParent->children().indexOf(nodeParent);

    QModelIndex resIndex = createIndex(row, 0, nodeParent);

    return resIndex;
}

int UBDocumentTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    UBDocumentTreeNode *nodeParent = nodeFromIndex(parent);
    if (!nodeParent) {
        return 0;
    }

    return nodeParent->children().count();
}

int UBDocumentTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant UBDocumentTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    UBDocumentTreeNode *dataNode = nodeFromIndex(index);
    if (!dataNode)
        return QVariant();

    if (role == Qt::UserRole + 1) {
        return QVariant::fromValue(dataNode);
    }

    if (index.column() == 0) {
        if (role == Qt::DecorationRole) {
            if (mCurrentNode && mCurrentNode == dataNode) {
                return QIcon(":images/currentDocument.png");
            } else {
                switch (static_cast<int>(dataNode->nodeType())) {
                case UBDocumentTreeNode::Catalog :
                    return QIcon(":images/folder.png");
                case UBDocumentTreeNode::Document :
                    return QIcon(":images/toolbar/board.png");
                }
            }
        } else if (role == Qt::DisplayRole) {
            return dataNode->displayName();
        }
    }

    return QVariant();
}

bool UBDocumentTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags UBDocumentTreeModel::flags (const QModelIndex &index) const
{
    Qt::ItemFlags resultFlags = QAbstractItemModel::flags(index);
    UBDocumentTreeNode *indexNode = nodeFromIndex(index);
    if ( index.isValid() )
    {
        if (!indexNode->isRoot() && !indexNode->isTopLevel()) {
            resultFlags |= Qt::ItemIsDragEnabled;
        }

        if (indexNode->nodeType() == UBDocumentTreeNode::Catalog) {
            resultFlags |= Qt::ItemIsDropEnabled;
        }
    }

    return resultFlags;
}

QStringList UBDocumentTreeModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list" << "image/png" << "image/tiff" << "image/gif" << "image/jpeg";
    return types;
}

QMimeData *UBDocumentTreeModel::mimeData (const QModelIndexList &indexes) const
{
    UBDocumentTreeMimeData *mimeData = new UBDocumentTreeMimeData();
    QList <QModelIndex> indexList;
    QList<QUrl> urlList;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            indexList.append(index);
            urlList.append(QUrl());
        }
    }

    mimeData->setUrls(urlList);
    mimeData->setIndexes(indexList);

    return mimeData;
}

bool UBDocumentTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
//    qDebug() << "drop mime data catched";
//    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    const UBDocumentTreeMimeData *mimeData = qobject_cast<const UBDocumentTreeMimeData*>(data);
    if (!mimeData) {
        qDebug() << "Incorrect mimeData, only internal one supported";
        return false;
    }
    if (!parent.isValid() && action != Qt::MoveAction) {
        return false;
    }
    UBDocumentTreeNode *newParentNode = nodeFromIndex(parent);

    if (!newParentNode) {
        qDebug() << "incorrect incoming parent node;";
        return false;
    }

    QList<QModelIndex> incomingIndexes = mimeData->indexes();

    foreach (QModelIndex curIndex, incomingIndexes) {
        UBDocumentTreeNode* sourceParentNode = nodeFromIndex(curIndex);
        if(sourceParentNode->parentNode()->nodeName() == "Models"){
            QString newDocumentPath = UBPersistenceManager::persistenceManager()->generateUniqueDocumentPath();
            UBFileSystemUtils::copyDir(sourceParentNode->proxyData()->persistencePath(),newDocumentPath);
            UBDocumentProxy* doc = UBPersistenceManager::persistenceManager()->createDocument(QString(),QString(),false,newDocumentPath,sourceParentNode->proxyData()->pageCount());
            doc->setMetaData(UBSettings::documentGroupName,nodeFromIndex(parent)->dirPathInHierarchy());
            UBPersistenceManager::persistenceManager()->persistDocumentMetadata(doc);
        }
        else{
            addElementByIndex(curIndex, parent);
            removeRows(curIndex.row(), 1, curIndex.parent());
        }
    }

    qDebug() << "custom mime data row " << row << "column" << column;


    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    return false;
}

bool UBDocumentTreeModel::removeRow(int row, const QModelIndex &parent)
{
    qDebug() << "remove row catched";
    return QAbstractItemModel::removeRow(row, parent);
}

bool UBDocumentTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row + count > rowCount(parent))
        return false;

    beginRemoveRows( parent, row, row + count - 1);

    UBDocumentTreeNode *parentNode = nodeFromIndex(parent);
    for (int i = row; i < row + count; i++) {
        UBDocumentTreeNode *curChildNode = parentNode->children().at(i);
        QModelIndex curChildIndex = parent.child(i, 0);
        if (curChildNode) {
            if (rowCount(curChildIndex)) {
                while (rowCount(curChildIndex)) {
                    removeRows(0, 1, curChildIndex);
                }
            }

        }
        parentNode->removeChild(i);
    }

    endRemoveRows();
    return true;
}

QModelIndex UBDocumentTreeModel::indexForNode(UBDocumentTreeNode *pNode) const
{
    if (pNode == 0) {
        return QModelIndex();
    }

    qDebug() << "searching for node" << pNode;

    return pIndexForNode(QModelIndex(), pNode);
}

QPersistentModelIndex UBDocumentTreeModel::persistentIndexForNode(UBDocumentTreeNode *pNode)
{
    return QPersistentModelIndex(indexForNode(pNode));
}

UBDocumentTreeNode *UBDocumentTreeModel::findProxy(UBDocumentProxy *pSearch, UBDocumentTreeNode *pParent) const
{
    foreach (UBDocumentTreeNode *curNode, pParent->children()) {
        if (curNode->proxyData() == pSearch) {
            return curNode;
        } else if (curNode->children().count()) {
            return findProxy(pSearch, curNode);
        }
    }

    return 0;
}

QModelIndex UBDocumentTreeModel::pIndexForNode(const QModelIndex &parent, UBDocumentTreeNode *pNode) const
{
    for (int i = 0; i < rowCount(parent); i++) {
        QModelIndex curIndex = index(i, 0, parent);
        qDebug() << "current index" << curIndex.internalPointer();
        if (curIndex.internalPointer() == pNode) {
            return curIndex;
        } else if (rowCount(curIndex) > 0) {
            return pIndexForNode(curIndex, pNode);
        }
    }
    return QModelIndex();
}

bool UBDocumentTreeModel::removeChildFromModel(UBDocumentTreeNode *child, UBDocumentTreeModel *parent)
{
    return true;
}

//bool UBDocumentTreeModel::insertRow(int row, const QModelIndex &parent)
//{
//    if (!parent.isValid()) {
//        return false;
//    }

//    UBDocumentTreeNode *currentParent = nodeFromIndex(parent);

//    if (row < 0 || row >= currentParent->children().count()) {
//        return false;
//    }

//    beginInsertRows(parent, row, row + 1);
////    curr

//    endInsertRows();
//    return true;
//}

void UBDocumentTreeModel::addNode(UBDocumentTreeNode *pFreeNode, UBDocumentTreeNode *pParent)
{
    QModelIndex tstParent = indexForNode(pParent);

    if (!tstParent.isValid() || pParent->nodeType() != UBDocumentTreeNode::Catalog) {
        return;
    }
    beginInsertRows(tstParent, pParent->children().size(), pParent->children().size());
    pParent->addChild(pFreeNode);
    endInsertRows();


//    foreach (UBDocumentTreeNode *curNode, pFreeNode->children()) {
//        addNode(curNode, pFreeNode);
//    }
}

QModelIndex UBDocumentTreeModel::addNode(UBDocumentTreeNode *pFreeNode, const QModelIndex &pParent)
{
    UBDocumentTreeNode *tstParent = nodeFromIndex(pParent);

    if (!pParent.isValid() || tstParent->nodeType() != UBDocumentTreeNode::Catalog) {
        return QModelIndex();
    }
    int newIndex = tstParent->children().size();
    beginInsertRows(pParent, newIndex, newIndex);
    tstParent->addChild(pFreeNode);
    endInsertRows();

    return createIndex(newIndex, 0, pFreeNode);
}

void UBDocumentTreeModel::addElementByIndex(const QModelIndex &NewChild, const QModelIndex &parent)
{
    UBDocumentTreeNode *tstParent = nodeFromIndex(parent);
    UBDocumentTreeNode *tstChild = nodeFromIndex(NewChild);

    if (!tstParent || !tstChild) {
        return;
    }
    beginInsertRows(parent, rowCount(parent), rowCount(parent));
    UBDocumentTreeNode *clonedtstChild = tstChild->clone();
    tstParent->addChild(clonedtstChild);
    if (clonedtstChild->proxyData()) {
        clonedtstChild->proxyData()->setMetaData(UBSettings::documentGroupName, clonedtstChild->dirPathInHierarchy());
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(clonedtstChild->proxyData());
    }
    endInsertRows();

    if (rowCount(NewChild)) {
        QModelIndex newChildParent = createIndex(rowCount(parent), 0, clonedtstChild);
        for (int i = 0; i < rowCount(NewChild); i++) {
            QModelIndex newLittleChild = NewChild.child(i, 0);
            addElementByIndex(newLittleChild, newChildParent);
        }
    }
}

void UBDocumentTreeModel::setCurrentDocument(UBDocumentProxy *pDocument)
{
    UBDocumentTreeNode *testCurNode = findProxy(pDocument, mRootNode);

    if (testCurNode) {
        setCurrentNode(testCurNode);
    }

//    reset();
}

QModelIndex UBDocumentTreeModel::indexForProxy(UBDocumentProxy *pSearch) const
{
    UBDocumentTreeNode *proxy = findProxy(pSearch, mRootNode);
    if (!proxy) {
        return QModelIndex();
    }

    return indexForNode(proxy);
}

void UBDocumentTreeModel::setRootNode(UBDocumentTreeNode *pRoot)
{
    mRootNode = pRoot;
    reset();
}

UBDocumentProxy *UBDocumentTreeModel::proxyForIndex(const QModelIndex &pIndex) const
{
    UBDocumentTreeNode *node = nodeFromIndex(pIndex);
    if (!node) {
        return 0;
    }

    return node->proxyData();
}

QString UBDocumentTreeModel::virtualDirForIndex(const QModelIndex &pIndex) const
{
    QString result;
    UBDocumentTreeNode *curNode = nodeFromIndex(pIndex);
    //protect the 2nd level items
    while (curNode->parentNode() && !curNode->isTopLevel()) {
        result.prepend(curNode->parentNode()->nodeName() + "/");
        curNode = curNode->parentNode();
    }

    if (result.endsWith("/")) {
        result.truncate(result.count() - 1);
    }

    return result;
}

QStringList UBDocumentTreeModel::nodeNameList(const QModelIndex &pIndex) const
{
    QStringList result;

    UBDocumentTreeNode *catalog = nodeFromIndex(pIndex);
    if (catalog->nodeType() != UBDocumentTreeNode::Catalog) {
        return QStringList();
    }

    foreach (UBDocumentTreeNode *curNode, catalog->children()) {
        result << curNode->nodeName();
    }

    return result;
}

bool UBDocumentTreeModel::newFolderAllowed(const QModelIndex &pIndex)  const
{
    UBDocumentTreeNode *tstNode = nodeFromIndex(pIndex);
    if (tstNode->isRoot() || tstNode->isTopLevel() || tstNode->parentNode()->nodeType() != UBDocumentTreeNode::Catalog) {
        return false;
    }

    return true;
}

QModelIndex UBDocumentTreeModel::goTo(const QString &dir)
{
    QStringList pathList = dir.split("/", QString::SkipEmptyParts);

    if (pathList.isEmpty()) {
        return untitledDocumentsIndex();
    }

    if (pathList.first() != UBPersistenceManager::myDocumentsName
            && pathList.first() != UBSettings::trashedDocumentGroupNamePrefix
            && pathList.first() != UBPersistenceManager::modelsName) {
        pathList.prepend(UBPersistenceManager::myDocumentsName);
    }

    QModelIndex parentIndex;

    bool searchingNode = true;
    while (!pathList.isEmpty())
    {
        QString curLevelName = pathList.takeFirst();
        if (searchingNode) {
            searchingNode = false;
            for (int i = 0; i < rowCount(parentIndex); ++i) {
                QModelIndex curChildIndex = index(i, 0, parentIndex);
                if (nodeFromIndex(curChildIndex)->nodeName() == curLevelName) {
                    searchingNode = true;
                    parentIndex = curChildIndex;
                    break;
                }
            }
        }

        if (!searchingNode) {
            UBDocumentTreeNode *newChild = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, curLevelName);
            parentIndex = addNode(newChild, parentIndex);
        }
    }

    return parentIndex;
}

UBDocumentTreeNode *UBDocumentTreeModel::nodeFromIndex(const QModelIndex &pIndex) const
{
    if (pIndex.isValid()) {
        return static_cast<UBDocumentTreeNode*>(pIndex.internalPointer());
    } else {
        return mRootNode;
    }
}

UBDocumentTreeModel::~UBDocumentTreeModel()
{
    delete mRootNode;
}

UBDocumentTreeView::UBDocumentTreeView(QWidget *parent) : QTreeView(parent) {

}

void UBDocumentTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
    event->acceptProposedAction();
}

void UBDocumentTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);
}

void UBDocumentTreeView::dropEvent(QDropEvent *event)
{
    qDebug() << "drop event catchded";
    event->setDropAction(Qt::MoveAction);
    QTreeView::dropEvent(event);
}

void UBDocumentTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    qDebug();

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

UBDocumentController::UBDocumentController(UBMainWindow* mainWindow)
   : UBDocumentContainer(mainWindow->centralWidget())
   , mSelectionType(None)
   , mParentWidget(mainWindow->centralWidget())
   , mBoardController(UBApplication::boardController)
   , mDocumentUI(0)
   , mMainWindow(mainWindow)
   , mDocumentWidget(0)
   , mIsClosing(false)
   , mToolsPalette(0)
   , mToolsPalettePositionned(false)
   , mTrashTi(0)
   , mDocumentTrashGroupName(tr("Trash"))
   , mDefaultDocumentGroupName(tr("Untitled Documents"))
   , mCurrentTreeDocument(0)
{

    setupViews();
    setupToolbar();
    this->selectDocument(UBApplication::boardController->selectedDocument());
    connect(this, SIGNAL(exportDone()), mMainWindow, SLOT(onExportDone()));
    connect(this, SIGNAL(documentThumbnailsUpdated(UBDocumentContainer*)), this, SLOT(refreshDocumentThumbnailsView(UBDocumentContainer*)));
}

UBDocumentController::~UBDocumentController()
{
   if (mDocumentUI)
       delete mDocumentUI;
}

void UBDocumentController::createNewDocument()
{
//    UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();

    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();
    if (firstSelectedTreeIndex().isValid())
    {
        QString groupName = pManager->mDocumentTreeStructureModel->virtualDirForIndex(firstSelectedTreeIndex());
        UBDocumentProxy *document = pManager->createDocument(groupName);
        selectDocument(document);
    }
}


UBDocumentProxyTreeItem* UBDocumentController::findDocument(UBDocumentProxy* proxy)
{
    QTreeWidgetItemIterator it(mDocumentUI->documentTreeWidget);

    while (*it)
    {
        UBDocumentProxyTreeItem *treeItem = dynamic_cast<UBDocumentProxyTreeItem*>((*it));

        if (treeItem && treeItem->proxy() == proxy)
            return treeItem;

        ++it;
    }

    return 0;
}


void UBDocumentController::selectDocument(UBDocumentProxy* proxy, bool setAsCurrentDocument)
{
    if (proxy==NULL)
    {
        setDocument(NULL);
        return;
    }

    if (setAsCurrentDocument) {
        QModelIndex treeIndex = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->indexForProxy(proxy);
        if (treeIndex.isValid()) {
            qDebug() << "valid index";
        }

        UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->setCurrentDocument(proxy);
        QModelIndex indexCurrentDoc = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->indexForProxy(proxy);
        setSelectedAndExpanded(indexCurrentDoc, true);
    }

    mSelectionType = Document;
    setDocument(proxy);
}

void UBDocumentController::setSelectedAndExpanded(const QModelIndex &pIndex, bool pExpand)
{
    QModelIndex indexCurrentDoc = pIndex;
    mDocumentUI->documentTreeView->selectionModel()->select(indexCurrentDoc, pExpand
                                                            ? QItemSelectionModel::Select
                                                            : QItemSelectionModel::Deselect);
    while (indexCurrentDoc.parent().isValid()) {
        mDocumentUI->documentTreeView->setExpanded(indexCurrentDoc.parent(), pExpand);
        indexCurrentDoc = indexCurrentDoc.parent();
    }
}

void UBDocumentController::createNewDocumentGroup()
{
    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();

    if (!pManager->mDocumentTreeStructureModel->newFolderAllowed(firstSelectedTreeIndex()))  {
        return;
    }

    int i = 0;
    QString newFolderName = tr("New Folder");
    QStringList siblingNames = pManager->mDocumentTreeStructureModel->nodeNameList(firstSelectedTreeIndex().parent());
    while (siblingNames.contains(newFolderName))
    {
        newFolderName = tr("New Folder") + " " + QVariant(++i).toString();
    }

    pManager->mDocumentTreeStructureModel->addNode(new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, newFolderName)
                                                   , firstSelectedTreeIndex().parent());
}


UBDocumentProxy* UBDocumentController::selectedDocumentProxy()
{
    UBDocumentProxyTreeItem* proxyItem = selectedDocumentProxyTreeItem();
    return proxyItem ? proxyItem->proxy() : 0;
}

QList<UBDocumentProxy*> UBDocumentController::selectedProxies()
{
    QList<UBDocumentProxy*> result;

    foreach (QModelIndex curIndex, mDocumentUI->documentTreeView->selectionModel()->selectedIndexes()) {
        result << UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->proxyForIndex(curIndex);
    }

    return result;
}

QModelIndexList UBDocumentController::selectedTreeIndexes()
{
    return mDocumentUI->documentTreeView->selectionModel()->selectedIndexes();
}

UBDocumentProxy* UBDocumentController::firstSelectedTreeProxy()
{
    return selectedProxies().count() ? selectedProxies().first() : 0;
}

UBDocumentProxyTreeItem* UBDocumentController::selectedDocumentProxyTreeItem()
{
    if (mDocumentUI && mDocumentUI->documentTreeWidget)
    {
        QList<QTreeWidgetItem *> selectedItems = mDocumentUI->documentTreeWidget->selectedItems();

        foreach (QTreeWidgetItem * item, selectedItems)
        {
            UBDocumentProxyTreeItem* proxyItem = dynamic_cast<UBDocumentProxyTreeItem*>(item);

            if (proxyItem)
            {
                return proxyItem;
            }
        }
    }

    return 0;
}


UBDocumentGroupTreeItem* UBDocumentController::selectedDocumentGroupTreeItem()
{
    QList<QTreeWidgetItem *> selectedItems = mDocumentUI->documentTreeWidget->selectedItems();

    foreach (QTreeWidgetItem * item, selectedItems)
    {
        UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);

        if (groupItem)
        {
            return groupItem;
        }
        else
        {
            UBDocumentGroupTreeItem* parent = dynamic_cast<UBDocumentGroupTreeItem*>(item->parent());
            if (parent)
            {
                return parent;
            }
        }
    }

    return 0;
}

void UBDocumentController::TreeViewSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    UBDocumentTreeNode *currentTreeNode = current.data(Qt::UserRole + 1).value<UBDocumentTreeNode*>();
    UBDocumentProxy *currentDocumentProxy = currentTreeNode->proxyData();
    QList<const QPixmap*> thumbs;

    if (currentDocumentProxy)
    {
        UBThumbnailAdaptor::load(currentDocumentProxy, thumbs);
        qDebug() << "document proxy taken" << currentDocumentProxy->name();
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;

    QGraphicsPixmapItem *selection = 0;

    QStringList labels;

    if (currentDocumentProxy)
    {
        emit setDocument(currentDocumentProxy);

        for (int i = 0; i < currentDocumentProxy->pageCount(); i++)
        {
            const QPixmap* pix = thumbs.at(i);
            QGraphicsPixmapItem *pixmapItem = new UBSceneThumbnailPixmap(*pix, currentDocumentProxy, i); // deleted by the tree widget

            if (currentDocumentProxy == mBoardController->selectedDocument() && mBoardController->activeSceneIndex() == i)
            {
                selection = pixmapItem;
            }

            items << pixmapItem;
            int pageIndex = pageFromSceneIndex(i);
            if(pageIndex)
                labels << tr("Page %1").arg(pageIndex);
            else
                labels << tr("Title page");

            itemsPath.append(QUrl::fromLocalFile(currentDocumentProxy->persistencePath() + QString("/pages/%1").arg(UBDocumentContainer::pageFromSceneIndex(i))));
        }
    }

    mDocumentUI->thumbnailWidget->setGraphicsItems(items, itemsPath, labels, UBApplication::mimeTypeUniboardPage);

    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();
    if ((proxyTi && (proxyTi->parent() == mTrashTi)) || (currentDocumentProxy && currentDocumentProxy->groupName() == "Models"))
        mDocumentUI->thumbnailWidget->setDragEnabled(false);
    else
        mDocumentUI->thumbnailWidget->setDragEnabled(true);

    mDocumentUI->thumbnailWidget->ensureVisible(0, 0, 10, 10);

    if (selection) {
        disconnect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));
        UBSceneThumbnailPixmap *currentScene = dynamic_cast<UBSceneThumbnailPixmap*>(selection);
        if (currentScene)
            mDocumentUI->thumbnailWidget->hightlightItem(currentScene->sceneIndex());
        connect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));
    }

    mMainWindow->actionExport->setEnabled(true);
    QApplication::restoreOverrideCursor();

}

void UBDocumentController::TreeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    if (selected.indexes().count()) {
        QModelIndex NewSelectedRow = selected.indexes().first();
        TreeViewSelectionChanged(NewSelectedRow, QModelIndex());
    }
}

void UBDocumentController::itemSelectionChanged()
{
    reloadThumbnails();

    if (selectedDocumentProxy())
        mSelectionType = Document;
    else if (selectedDocumentGroupTreeItem())
        mSelectionType = Folder;
    else
        mSelectionType = None;

    selectionChanged();
}


void UBDocumentController::setupViews()
{

    if (!mDocumentWidget)
    {
        mDocumentWidget = new QWidget(mMainWindow->centralWidget());
        mMainWindow->addDocumentsWidget(mDocumentWidget);

        mDocumentUI = new Ui::documents();

        mDocumentUI->setupUi(mDocumentWidget);

        int thumbWidth = UBSettings::settings()->documentThumbnailWidth->get().toInt();

        mDocumentUI->documentZoomSlider->setValue(thumbWidth);
        mDocumentUI->thumbnailWidget->setThumbnailWidth(thumbWidth);

        connect(mDocumentUI->documentZoomSlider, SIGNAL(valueChanged(int)), this,
                SLOT(documentZoomSliderValueChanged(int)));

        connect(mMainWindow->actionOpen, SIGNAL(triggered()), this, SLOT(openSelectedItem()));
        connect(mMainWindow->actionNewFolder, SIGNAL(triggered()), this, SLOT(createNewDocumentGroup()));
        connect(mMainWindow->actionNewDocument, SIGNAL(triggered()), this, SLOT(createNewDocument()));

        connect(mMainWindow->actionImport, SIGNAL(triggered(bool)), this, SLOT(importFile()));

        QMenu* addMenu = new QMenu(mDocumentWidget);
        mAddFolderOfImagesAction = addMenu->addAction(tr("Add Folder of Images"));
        mAddImagesAction = addMenu->addAction(tr("Add Images"));
        mAddFileToDocumentAction = addMenu->addAction(tr("Add Pages from File"));

        connect(mAddFolderOfImagesAction, SIGNAL(triggered(bool)), this, SLOT(addFolderOfImages()));
        connect(mAddFileToDocumentAction, SIGNAL(triggered(bool)), this, SLOT(addFileToDocument()));
        connect(mAddImagesAction, SIGNAL(triggered(bool)), this, SLOT(addImages()));

        foreach (QWidget* menuWidget,  mMainWindow->actionDocumentAdd->associatedWidgets())
        {
            QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

            if (tb && !tb->menu())
            {
                tb->setObjectName("ubButtonMenu");
                tb->setPopupMode(QToolButton::InstantPopup);

                QMenu* menu = new QMenu(mDocumentWidget);

                menu->addAction(mAddFolderOfImagesAction);
                menu->addAction(mAddImagesAction);
                menu->addAction(mAddFileToDocumentAction);

                tb->setMenu(menu);
            }
        }

        QMenu* exportMenu = new QMenu(mDocumentWidget);

        UBDocumentManager *documentManager = UBDocumentManager::documentManager();
        for (int i = 0; i < documentManager->supportedExportAdaptors().length(); i++)
        {
            UBExportAdaptor* adaptor = documentManager->supportedExportAdaptors()[i];
            QAction *currentExportAction = exportMenu->addAction(adaptor->exportName());
            currentExportAction->setData(i);
            connect(currentExportAction, SIGNAL(triggered (bool)), this, SLOT(exportDocument()));
            exportMenu->addAction(currentExportAction);
        }

        foreach (QWidget* menuWidget,  mMainWindow->actionExport->associatedWidgets())
        {
            QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

            if (tb && !tb->menu())
            {
                tb->setObjectName("ubButtonMenu");
                tb->setPopupMode(QToolButton::InstantPopup);

                tb->setMenu(exportMenu);
            }
        }

#ifdef Q_WS_MAC
        mMainWindow->actionDelete->setShortcut(QKeySequence(Qt::Key_Backspace));
#endif

        connect(mMainWindow->actionDelete, SIGNAL(triggered()), this, SLOT(deleteSelectedItem()));
        connect(mMainWindow->actionDuplicate, SIGNAL(triggered()), this, SLOT(duplicateSelectedItem()));
        connect(mMainWindow->actionRename, SIGNAL(triggered()), this, SLOT(renameSelectedItem()));
        connect(mMainWindow->actionAddToWorkingDocument, SIGNAL(triggered()), this, SLOT(addToDocument()));

        loadDocumentProxies();

        mDocumentUI->documentTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        mDocumentUI->documentTreeWidget->setDragEnabled(true);
        mDocumentUI->documentTreeWidget->viewport()->setAcceptDrops(true);
        mDocumentUI->documentTreeWidget->setDropIndicatorShown(true);
        mDocumentUI->documentTreeWidget->setIndentation(18); // 1.5 * /resources/style/treeview-branch-closed.png width
        mDocumentUI->documentTreeWidget->setDragDropMode(QAbstractItemView::InternalMove);

//        connect(mDocumentUI->documentTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
//        connect(mDocumentUI->documentTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(itemChanged(QTreeWidgetItem *, int)));
//        connect(mDocumentUI->documentTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(itemClicked(QTreeWidgetItem *, int)));

        mDocumentUI->documentTreeView->setModel(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel);
//        mDocumentUI->documentTreeView->setDragDropMode(QAbstractItemView::InternalMove);
        mDocumentUI->documentTreeView->setDragEnabled(true);
        mDocumentUI->documentTreeView->setAcceptDrops(true);
        mDocumentUI->documentTreeView->viewport()->setAcceptDrops(true);
        mDocumentUI->documentTreeView->setDropIndicatorShown(true);

        connect(mDocumentUI->documentTreeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(TreeViewSelectionChanged(QModelIndex,QModelIndex)));
        connect(mDocumentUI->documentTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(TreeViewSelectionChanged(QItemSelection,QItemSelection)));

        connect(mDocumentUI->thumbnailWidget, SIGNAL(sceneDropped(UBDocumentProxy*, int, int)), this, SLOT(moveSceneToIndex ( UBDocumentProxy*, int, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(resized()), this, SLOT(thumbnailViewResized()));
//        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseDoubleClick(QGraphicsItem*, int)), this, SLOT(pageDoubleClicked(QGraphicsItem*, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseDoubleClick(QGraphicsItem*,int)), this, SLOT(thumbnailPageDoubleClicked(QGraphicsItem*,int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseClick(QGraphicsItem*, int)), this, SLOT(pageClicked(QGraphicsItem*, int)));

        connect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentCreated(UBDocumentProxy*)), this, SLOT(addDocumentInTree(UBDocumentProxy*)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*)), this, SLOT(updateDocumentInTree(UBDocumentProxy*)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneCreated(UBDocumentProxy*, int)), this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneWillBeDeleted(UBDocumentProxy*, int)), this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

        mDocumentUI->thumbnailWidget->setBackgroundBrush(UBSettings::documentViewLightColor);

        #ifdef Q_WS_MACX
            mMessageWindow = new UBMessageWindow(NULL);
        #else
            mMessageWindow = new UBMessageWindow(mDocumentUI->thumbnailWidget);
        #endif

        mMessageWindow->hide();

    }
}


QWidget* UBDocumentController::controlView()
{
    return mDocumentWidget;
}


void UBDocumentController::setupToolbar()
{
    UBApplication::app()->insertSpaceToToolbarBeforeAction(mMainWindow->documentToolBar, mMainWindow->actionBoard);
    connect(mMainWindow->actionDocumentTools, SIGNAL(triggered()), this, SLOT(toggleDocumentToolsPalette()));
}

void UBDocumentController::setupPalettes()
{

    mToolsPalette = new UBDocumentToolsPalette(controlView());

    mToolsPalette->hide();

    bool showToolsPalette = !mToolsPalette->isEmpty();
    mMainWindow->actionDocumentTools->setVisible(showToolsPalette);

    if (showToolsPalette)
    {
        mMainWindow->actionDocumentTools->trigger();
    }
}


void UBDocumentController::show()
{
    selectDocument(mBoardController->selectedDocument());

    selectionChanged();

    if(!mToolsPalette)
        setupPalettes();
}


void UBDocumentController::hide()
{
    // NOOP
}


void UBDocumentController::openSelectedItem()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

    if (selectedItems.count() > 0)
    {
        UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (selectedItems.last());

        if (thumb)
        {
            UBDocumentProxy* proxy = thumb->proxy();

            if (proxy && isOKToOpenDocument(proxy))
            {
                UBApplication::applicationController->showBoard();
            }
        }
    }
    else
    {
        UBDocumentProxy* proxy = selectedDocumentProxy();

        if (proxy && isOKToOpenDocument(proxy))
        {
            mBoardController->setActiveDocumentScene(proxy);
            UBApplication::applicationController->showBoard();
        }
    }

    QApplication::restoreOverrideCursor();
}

void UBDocumentController::duplicateSelectedItem()
{
    if (UBApplication::applicationController->displayMode() != UBApplicationController::Document)
        return;

    if (mSelectionType == Page)
    {
        QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();
        QList<int> selectedSceneIndexes;
        foreach (QGraphicsItem *item, selectedItems)
        {
            UBSceneThumbnailPixmap *thumb = dynamic_cast<UBSceneThumbnailPixmap*>(item);
            if (thumb)
            {
                UBDocumentProxy *proxy = thumb->proxy();

                if (proxy)
                {
                    int sceneIndex = thumb->sceneIndex();
                    selectedSceneIndexes << sceneIndex;
                }
            }
        }
        if (selectedSceneIndexes.count() > 0)
        {
            duplicatePages(selectedSceneIndexes);
            emit documentThumbnailsUpdated(this);
            selectedDocument()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(selectedDocument());
            mDocumentUI->thumbnailWidget->selectItemAt(selectedSceneIndexes.last() + selectedSceneIndexes.size());
        }
    }
    else
    {
        UBDocumentProxy* source = selectedDocumentProxy();
        UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();

        if (source && group)
        {
                QString docName = source->metaData(UBSettings::documentName).toString();

                showMessage(tr("Duplicating Document %1").arg(docName), true);

            UBDocumentProxy* duplicatedDoc = UBPersistenceManager::persistenceManager()->duplicateDocument(source);
            duplicatedDoc->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(duplicatedDoc);

            selectDocument(duplicatedDoc, false);

            showMessage(tr("Document %1 copied").arg(docName), false);
        }
    }
}

void UBDocumentController::moveDocumentToTrash(UBDocumentGroupTreeItem* groupTi, UBDocumentProxyTreeItem *proxyTi)
{
    int index = proxyTi->parent()->indexOfChild(proxyTi);
    index --;

    if (index >= 0)
    {
        if (proxyTi->proxy() == mBoardController->selectedDocument())
        {
            selectDocument(((UBDocumentProxyTreeItem*)proxyTi->parent()->child(index))->proxy(), true);
        }
        else
            proxyTi->parent()->child(index)->setSelected(true);
    }
    else if (proxyTi->parent()->childCount() > 1)
    {
        if (proxyTi->proxy() == mBoardController->selectedDocument())
        {
            selectDocument(((UBDocumentProxyTreeItem*)proxyTi->parent()->child(1))->proxy(), true);
        }
        else
            proxyTi->parent()->child(1)->setSelected(true);
    }
    else
    {
        if (proxyTi->proxy() == mBoardController->selectedDocument())
        {
            bool documentFound = false;
            for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
            {
                QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
                UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
                if (!groupItem->isTrashFolder())
                {
                    for(int j=0; j<groupItem->childCount(); j++)
                    {
                        if (((UBDocumentProxyTreeItem*)groupItem->child(j))->proxy() != mBoardController->selectedDocument())
                        {
                            selectDocument(((UBDocumentProxyTreeItem*)groupItem->child(0))->proxy(), true);
                            documentFound = true;
                            break;
                        }
                    }
                }
                if (documentFound)
                    break;
            }
            if (!documentFound)
            {
                UBDocumentProxy *document = UBPersistenceManager::persistenceManager()->createDocument(groupTi->groupName());
                selectDocument(document, true);
            }
        }
        else
            proxyTi->parent()->setSelected(true);
    }

    QString oldGroupName = proxyTi->proxy()->metaData(UBSettings::documentGroupName).toString();
    proxyTi->proxy()->setMetaData(UBSettings::documentGroupName, UBSettings::trashedDocumentGroupNamePrefix + oldGroupName);
    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxyTi->proxy());

    proxyTi->parent()->removeChild(proxyTi);
    mTrashTi->addChild(proxyTi);
    proxyTi->setFlags(proxyTi->flags() ^ Qt::ItemIsEditable);
}

void UBDocumentController::moveFolderToTrash(UBDocumentGroupTreeItem* groupTi)
{
    bool changeCurrentDocument = false;
    for (int i = 0; i < groupTi->childCount(); i++)
    {
        UBDocumentProxyTreeItem* proxyTi  = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
        if (proxyTi && proxyTi->proxy() && proxyTi->proxy() == mBoardController->selectedDocument())
        {
            changeCurrentDocument = true;
            break;
        }
    }

    QList<UBDocumentProxyTreeItem*> toBeDeleted;

    for (int i = 0; i < groupTi->childCount(); i++)
    {
        UBDocumentProxyTreeItem* proxyTi = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
        if (proxyTi && proxyTi->proxy())
            toBeDeleted << proxyTi;
    }

    for (int i = 0; i < toBeDeleted.count(); i++)
    {
        UBDocumentProxyTreeItem* proxyTi = toBeDeleted.at(i);

        showMessage(QString("Deleting %1").arg(proxyTi->proxy()->metaData(UBSettings::documentName).toString()));
        // Move document to trash
        QString oldGroupName = proxyTi->proxy()->metaData(UBSettings::documentGroupName).toString();
        proxyTi->proxy()->setMetaData(UBSettings::documentGroupName, UBSettings::trashedDocumentGroupNamePrefix + oldGroupName);
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxyTi->proxy());

        groupTi->removeChild(proxyTi);
        mTrashTi->addChild(proxyTi);
        proxyTi->setFlags(proxyTi->flags() ^ Qt::ItemIsEditable);

        showMessage(QString("%1 deleted").arg(groupTi->groupName()));
    }

    // dont remove default group
    if (!groupTi->isDefaultFolder())
    {
        int index = mDocumentUI->documentTreeWidget->indexOfTopLevelItem(groupTi);

        if (index >= 0)
        {
            mDocumentUI->documentTreeWidget->takeTopLevelItem(index);
        }
    }

    if (changeCurrentDocument)
    {
        bool documentFound = false;
        for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
            UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
            if (!groupItem->isTrashFolder() && groupItem != groupTi)
            {
                for(int j=0; j<groupItem->childCount(); j++)
                {
                    if (((UBDocumentProxyTreeItem*)groupItem->child(j))->proxy() != mBoardController->selectedDocument())
                    {
                        selectDocument(((UBDocumentProxyTreeItem*)groupItem->child(0))->proxy(), true);
                        documentFound = true;
                        break;
                    }
                }
            }
            if (documentFound)
                break;
        }
        if (!documentFound)
        {
            UBDocumentProxy *document = UBPersistenceManager::persistenceManager()->createDocument( mDefaultDocumentGroupName );
            selectDocument(document, true);
        }
    }

    reloadThumbnails();
}

void UBDocumentController::deleteSelectedItem()
{
    if (mSelectionType == Page)
    {
        QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

        deletePages(selectedItems);
    }
    else
    {

        UBDocumentProxyTreeItem *proxyTi = selectedDocumentProxyTreeItem();

        UBDocumentGroupTreeItem* groupTi = selectedDocumentGroupTreeItem();

        if (proxyTi && proxyTi->proxy() && proxyTi->parent())
        {
            if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Document"), tr("Are you sure you want to remove the document '%1'?").arg(proxyTi->proxy()->metaData(UBSettings::documentName).toString())))
            {
                if (proxyTi->parent() != mTrashTi)
                {
                    moveDocumentToTrash(groupTi, proxyTi);
                }
                else
                {
                    // We have to physically delete document
                    proxyTi->parent()->removeChild(proxyTi);
                    UBPersistenceManager::persistenceManager()->deleteDocument(proxyTi->proxy());

                    if (mTrashTi->childCount()==0)
                        selectDocument(NULL);
                    else
                        selectDocument(((UBDocumentProxyTreeItem*)mTrashTi->child(0))->proxy());
                    reloadThumbnails();
                }
            }
        }
        else if (groupTi)
        {
            if (groupTi == mTrashTi)
            {
                if(UBApplication::mainWindow->yesNoQuestion(tr("Empty Trash"), tr("Are you sure you want to empty trash?")))
                {
                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                    QList<UBDocumentProxyTreeItem*> toBeDeleted;

                    for (int i = 0; i < groupTi->childCount(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
                        if (proxyTi && proxyTi->proxy())
                            toBeDeleted << proxyTi;
                    }

                    showMessage(tr("Emptying trash"));

                    for (int i = 0; i < toBeDeleted.count(); i++)
                    {
                        UBDocumentProxyTreeItem* proxyTi = toBeDeleted.at(i);

                        proxyTi->parent()->removeChild(proxyTi);
                        UBPersistenceManager::persistenceManager()->deleteDocument(proxyTi->proxy());
                    }

                    showMessage(tr("Emptied trash"));

                    QApplication::restoreOverrideCursor();
                    mMainWindow->actionDelete->setEnabled(false);
                }
            }
            else
            {
                if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Folder"), tr("Are you sure you want to remove the folder '%1' and all its content?").arg(groupTi->groupName())))
                {
                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                    moveFolderToTrash(groupTi);
                    QApplication::restoreOverrideCursor();
                }
            }
        }
    }
}


void UBDocumentController::exportDocument()
{
    QAction *currentExportAction = qobject_cast<QAction *>(sender());
    QVariant actionData = currentExportAction->data();
    UBExportAdaptor* selectedExportAdaptor = UBDocumentManager::documentManager()->supportedExportAdaptors()[actionData.toInt()];

//    UBDocumentProxy* proxy = selectedDocumentProxy();
    UBDocumentProxy* proxy = firstSelectedTreeProxy();

//    if (proxy)
//    {
        selectedExportAdaptor->persist(proxy);
        emit exportDone();
//    }
//    else
//    {
//       showMessage(tr("No document selected!"));
//    }
}

void UBDocumentController::exportDocumentSet()
{

}

void UBDocumentController::documentZoomSliderValueChanged (int value)
{
    mDocumentUI->thumbnailWidget->setThumbnailWidth(value);

    UBSettings::settings()->documentThumbnailWidth->set(value);
}


void UBDocumentController::loadDocumentProxies()
{
    QList<QPointer<UBDocumentProxy> > proxies = UBPersistenceManager::persistenceManager()->documentProxies;

    QStringList emptyGroupNames = UBSettings::settings()->value("Document/EmptyGroupNames", QStringList()).toStringList();

    mDocumentUI->documentTreeWidget->clear();

    QMap<QString, UBDocumentGroupTreeItem*> groupNamesMap;

    UBDocumentGroupTreeItem* emptyGroupNameTi = 0;

    mTrashTi = new UBDocumentGroupTreeItem(0, false); // deleted by the tree widget
    mTrashTi->setGroupName(mDocumentTrashGroupName);
    mTrashTi->setIcon(0, QIcon(":/images/trash.png"));

    foreach (QPointer<UBDocumentProxy> proxy, proxies)
    {
        if (proxy)
        {
            QString docGroup = proxy->metaData(UBSettings::documentGroupName).toString();

            bool isEmptyGroupName = false;
            bool isInTrash = false;

            if (docGroup.isEmpty()) // #see https://trac.assembla.com/uniboard/ticket/426
            {
                docGroup = mDefaultDocumentGroupName;
                isEmptyGroupName = true;
            }
            else if (docGroup.startsWith(UBSettings::trashedDocumentGroupNamePrefix))
            {
                isInTrash = true;
            }

            QString docName = proxy->metaData(UBSettings::documentName).toString();

            if (emptyGroupNames.contains(docGroup))
                emptyGroupNames.removeAll(docGroup);

            if (!groupNamesMap.contains(docGroup) && !isInTrash)
            {
                UBDocumentGroupTreeItem* docGroupItem = new UBDocumentGroupTreeItem(0, !isEmptyGroupName); // deleted by the tree widget
                groupNamesMap.insert(docGroup, docGroupItem);
                docGroupItem->setGroupName(docGroup);

                if (isEmptyGroupName)
                    emptyGroupNameTi = docGroupItem;
            }

            UBDocumentGroupTreeItem* docGroupItem;
            if (isInTrash)
                docGroupItem = mTrashTi;
            else
                docGroupItem = groupNamesMap.value(docGroup);

            QTreeWidgetItem* docItem = new UBDocumentProxyTreeItem(docGroupItem, proxy, !isInTrash);
            docItem->setText(0, docName);

            if (mBoardController->selectedDocument() == proxy)
            {
                mDocumentUI->documentTreeWidget->expandItem(docGroupItem);
                mDocumentUI->documentTreeWidget->setCurrentItem(docGroupItem);
            }
        }
    }

    foreach (const QString emptyGroupName, emptyGroupNames)
    {
        UBDocumentGroupTreeItem* docGroupItem = new UBDocumentGroupTreeItem(0); // deleted by the tree widget
        groupNamesMap.insert(emptyGroupName, docGroupItem);
        docGroupItem->setGroupName(emptyGroupName);
    }

    QList<QString> groupNamesList = groupNamesMap.keys();
    qSort(groupNamesList);

    foreach (const QString groupName, groupNamesList)
    {
        UBDocumentGroupTreeItem* ti = groupNamesMap.value(groupName);

        if (ti != emptyGroupNameTi)
            mDocumentUI->documentTreeWidget->addTopLevelItem(ti);
    }

    if (emptyGroupNameTi)
        mDocumentUI->documentTreeWidget->addTopLevelItem(emptyGroupNameTi);

    mDocumentUI->documentTreeWidget->addTopLevelItem(mTrashTi);
}


void UBDocumentController::itemClicked(QTreeWidgetItem * item, int column )
{
    Q_UNUSED(item);
    Q_UNUSED(column);

    selectDocument(selectedDocumentProxy(), false);
    itemSelectionChanged();
}


void UBDocumentController::itemChanged(QTreeWidgetItem * item, int column)
{
    UBDocumentProxyTreeItem* proxyItem = dynamic_cast<UBDocumentProxyTreeItem*>(item);

    disconnect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*))
            , this, SLOT(updateDocumentInTree(UBDocumentProxy*)));

    if (proxyItem)
    {
        if (proxyItem->proxy()->metaData(UBSettings::documentName).toString() != item->text(column))
        {
            proxyItem->proxy()->setMetaData(UBSettings::documentName, item->text(column));
            UBPersistenceManager::persistenceManager()->persistDocumentMetadata(proxyItem->proxy());
        }
    }
    else
    {
        // it is a group
        UBDocumentGroupTreeItem* editedGroup = dynamic_cast<UBDocumentGroupTreeItem*>(item);
        if (editedGroup)
        {
            for (int i = 0; i < item->childCount(); i++)
            {
                UBDocumentProxyTreeItem* childItem = dynamic_cast<UBDocumentProxyTreeItem*>(item->child(i));

                if (childItem)
                {
                    QString groupName;
                    if (0 != (item->flags() & Qt::ItemIsEditable))
                    {
                        childItem->proxy()->setMetaData(UBSettings::documentGroupName, item->text(column));
                        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(childItem->proxy());
                    }
                }
            }
        }
    }

    connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*)),
            this, SLOT(updateDocumentInTree(UBDocumentProxy*)));
}


void UBDocumentController::importFile()
{
    UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();
    UBDocumentManager *docManager = UBDocumentManager::documentManager();

    if (group)
    {
        QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
        QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File"),
                defaultPath, docManager->importFileFilter());

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QApplication::processEvents();
        QFileInfo fileInfo(filePath);
        UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

        if (filePath.length() > 0)
        {
            UBDocumentProxy* createdDocument = 0;
            QApplication::processEvents();
            QFile selectedFile(filePath);

            QString groupName = group->groupName();

            if (groupName == mDefaultDocumentGroupName || fileInfo.suffix() != "ubz")
                groupName = "";

            showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

            createdDocument = docManager->importFile(selectedFile, groupName);

            if (createdDocument)
            {
                selectDocument(createdDocument, false);
            }
            else
            {
                showMessage(tr("Failed to import file ... "));
            }
        }

        QApplication::restoreOverrideCursor();
    }
}

void UBDocumentController::addFolderOfImages()
{
    UBDocumentProxy* document = selectedDocumentProxy();

    if (document)
    {
        QString defaultPath = UBSettings::settings()->lastImportFolderPath->get().toString();

        QString imagesDir = QFileDialog::getExistingDirectory(mParentWidget, tr("Import all Images from Folder"), defaultPath);
        QDir parentImageDir(imagesDir);
        parentImageDir.cdUp();

        UBSettings::settings()->lastImportFolderPath->set(QVariant(parentImageDir.absolutePath()));

        if (imagesDir.length() > 0)
        {
            QDir dir(imagesDir);

            int importedImageNumber
                  = UBDocumentManager::documentManager()->addImageDirToDocument(dir, document);

            if (importedImageNumber == 0)
            {
                showMessage(tr("Folder does not contain any image files"));
                UBApplication::applicationController->showDocument();
            }
            else
            {
                document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
                UBMetadataDcSubsetAdaptor::persist(document);
                reloadThumbnails();
            }
        }
    }
}


void UBDocumentController::addFileToDocument()
{
    UBDocumentProxy* document = selectedDocumentProxy();

    if (document)
    {
         addFileToDocument(document);
         reloadThumbnails();
    }
}


bool UBDocumentController::addFileToDocument(UBDocumentProxy* document)
{
    QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
    QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File"), defaultPath, UBDocumentManager::documentManager()->importFileFilter());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    QFileInfo fileInfo(filePath);
    UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

    bool success = false;

    if (filePath.length() > 0)
    {
        QApplication::processEvents(); // NOTE: We performed this just a few lines before. Is it really necessary to do it again here??
        QFile selectedFile(filePath);

        showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

        QStringList fileNames;
        fileNames << filePath;
        success = UBDocumentManager::documentManager()->addFilesToDocument(document, fileNames);

        if (success)
        {
            document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(document);
        }
        else
        {
            showMessage(tr("Failed to import file ... "));
        }
    }

    QApplication::restoreOverrideCursor();

    return success;
}


void UBDocumentController::moveSceneToIndex(UBDocumentProxy* proxy, int source, int target)
{
    if (UBDocumentContainer::movePageToIndex(source, target))
    {
        proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(proxy);

        mDocumentUI->thumbnailWidget->hightlightItem(target);
    }
}


void UBDocumentController::thumbnailViewResized()
{
    int maxWidth = qMin(UBSettings::maxThumbnailWidth, mDocumentUI->thumbnailWidget->width());

    mDocumentUI->documentZoomSlider->setMaximum(maxWidth);
}


void UBDocumentController::pageSelectionChanged()
{
//    if (mIsClosing)
//        return;

//    bool pageSelected = mDocumentUI->thumbnailWidget->selectedItems().count() > 0;

//    if (pageSelected)
//        mSelectionType = Page;
//    else
//        mSelectionType = None;

//    selectionChanged();
}


void UBDocumentController::selectionChanged()
{
    if (mIsClosing)
        return;

    int pageCount = -1;

    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();

    if (proxyTi && proxyTi->proxy())
        pageCount = proxyTi->proxy()->pageCount();

    bool pageSelected = (mSelectionType == Page);
    bool groupSelected = (mSelectionType == Folder);
    bool docSelected = (mSelectionType == Document);

    bool trashSelected = false;
    if (groupSelected && selectedDocumentGroupTreeItem())
        trashSelected = selectedDocumentGroupTreeItem()->isTrashFolder();

    if ((docSelected || pageSelected) && proxyTi)
        trashSelected = dynamic_cast<UBDocumentGroupTreeItem*>(proxyTi->parent())->isTrashFolder();

    bool defaultGroupSelected = false;
    if (groupSelected && selectedDocumentGroupTreeItem())
        defaultGroupSelected = selectedDocumentGroupTreeItem()->isDefaultFolder();

    mMainWindow->actionNewDocument->setEnabled((groupSelected || docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionExport->setEnabled((docSelected || pageSelected) && !trashSelected);
    bool firstSceneSelected = false;
    if(docSelected)
        mMainWindow->actionDuplicate->setEnabled(!trashSelected);
    else if(pageSelected){
        QList<QGraphicsItem*> selection = mDocumentUI->thumbnailWidget->selectedItems();
        if(pageCount == 1)
            mMainWindow->actionDuplicate->setEnabled(!trashSelected && pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(0)));
        else{
            for(int i = 0; i < selection.count() && !firstSceneSelected; i += 1){
                if(dynamic_cast<UBSceneThumbnailPixmap*>(selection.at(i))->sceneIndex() == 0){
                    mMainWindow->actionDuplicate->setEnabled(!trashSelected && pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(0)));
                    firstSceneSelected = true;
                }
            }
            if(!firstSceneSelected)
                mMainWindow->actionDuplicate->setEnabled(!trashSelected);
        }
    }
    else
        mMainWindow->actionDuplicate->setEnabled(false);

    mMainWindow->actionOpen->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionRename->setEnabled((groupSelected || docSelected) && !trashSelected && !defaultGroupSelected);

    mMainWindow->actionAddToWorkingDocument->setEnabled(pageSelected
            && !(selectedDocumentProxy() == mBoardController->selectedDocument()) && !trashSelected);

    bool deleteEnabled = false;
    if (trashSelected)
    {
        if (docSelected)
            deleteEnabled = true;
        else if (groupSelected && selectedDocumentGroupTreeItem())
        {
            if (selectedDocumentGroupTreeItem()->childCount() > 0)
                deleteEnabled = true;
        }
    }
    else
    {
        deleteEnabled = groupSelected || docSelected || pageSelected;
    }

    if (pageSelected && (pageCount == mDocumentUI->thumbnailWidget->selectedItems().count()))
    {
        deleteEnabled = false;
    }

    if(pageSelected && firstSceneSelected)
        deleteEnabled = false;

    mMainWindow->actionDelete->setEnabled(deleteEnabled);

    if (trashSelected)
    {
        if (docSelected)
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/toolbar/deleteDocument.png"));
            mMainWindow->actionDelete->setText(tr("Delete"));
        }
        else
        {
            mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
            mMainWindow->actionDelete->setText(tr("Empty"));
        }
    }
    else
    {
        mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
        mMainWindow->actionDelete->setText(tr("Trash"));
    }

    mMainWindow->actionDocumentAdd->setEnabled((docSelected || pageSelected) && !trashSelected);
    mMainWindow->actionImport->setEnabled(!trashSelected);

}


void UBDocumentController::documentSceneChanged(UBDocumentProxy* proxy, int pSceneIndex)
{
    Q_UNUSED(pSceneIndex);

    if (proxy == selectedDocumentProxy())
    {
        reloadThumbnails();
    }
    TreeViewSelectionChanged(firstSelectedTreeIndex(), QModelIndex());
}


void UBDocumentController::pageDoubleClicked(QGraphicsItem* item, int index)
{
    Q_UNUSED(item);
    Q_UNUSED(index);

    bool pageSelected = (mSelectionType == Page);
    bool groupSelected = (mSelectionType == Folder);
    bool docSelected = (mSelectionType == Document);

    bool trashSelected = false;
    if (groupSelected && selectedDocumentGroupTreeItem())
        trashSelected = selectedDocumentGroupTreeItem()->isTrashFolder();
    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();
    if ((docSelected || pageSelected) && proxyTi)
        trashSelected = dynamic_cast<UBDocumentGroupTreeItem*>(proxyTi->parent())->isTrashFolder();
    if (trashSelected) return;

    openSelectedItem();
}

void UBDocumentController::thumbnailPageDoubleClicked(QGraphicsItem* item, int index)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    UBSceneThumbnailPixmap* thumb = qgraphicsitem_cast<UBSceneThumbnailPixmap*> (item);

    if (thumb) {
        UBDocumentProxy* proxy = thumb->proxy();
        if (proxy && isOKToOpenDocument(proxy)) {
            mBoardController->setActiveDocumentScene(proxy, index);
            UBApplication::applicationController->showBoard();
        }
    }

    QApplication::restoreOverrideCursor();
}


void UBDocumentController::pageClicked(QGraphicsItem* item, int index)
{
    Q_UNUSED(item);
    Q_UNUSED(index);

    pageSelectionChanged();
}


void UBDocumentController::closing()
{
    mIsClosing = true;

    QStringList emptyGroups;

    for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);

        if (item->childCount() == 0)
        {
            UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
            if (groupItem)
            {
                QString groupName = groupItem->groupName();
                if (!emptyGroups.contains(groupName) && groupName != mDocumentTrashGroupName)
                    emptyGroups << groupName;
            }
        }
    }

    UBSettings::settings()->setValue("Document/EmptyGroupNames", emptyGroups);

}

void UBDocumentController::addToDocument()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

    if (selectedItems.count() > 0)
    {
        int oldActiveSceneIndex = mBoardController->activeSceneIndex();

        QList<QPair<UBDocumentProxy*, int> > pageInfoList;

        foreach (QGraphicsItem* item, selectedItems)
        {
            UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (item);

            if (thumb &&  thumb->proxy())
            {
                QPair<UBDocumentProxy*, int> pageInfo(thumb->proxy(), thumb->sceneIndex());
                pageInfoList << pageInfo;
            }
        }

        for (int i = 0; i < pageInfoList.length(); i++)
        {
            mBoardController->addScene(pageInfoList.at(i).first, pageInfoList.at(i).second, true);
        }

        int newActiveSceneIndex = selectedItems.count() == mBoardController->selectedDocument()->pageCount() ? 0 : oldActiveSceneIndex + 1;
        mDocumentUI->thumbnailWidget->selectItemAt(newActiveSceneIndex, false);
        selectDocument(mBoardController->selectedDocument());
        mBoardController->selectedDocument()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(mBoardController->selectedDocument());

        UBApplication::applicationController->showBoard();
    }

    QApplication::restoreOverrideCursor();
}


void UBDocumentController::addDocumentInTree(UBDocumentProxy* pDocument)
{
    QString documentName = pDocument->name();
    QString documentGroup = pDocument->groupName();
    if (documentGroup.isEmpty())
    {
        documentGroup = mDefaultDocumentGroupName;
    }
    UBDocumentGroupTreeItem* group = 0;
    if (documentGroup.startsWith(UBSettings::trashedDocumentGroupNamePrefix))
    {
        group = mTrashTi;
    }
    else
    {
        for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
            UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
            if (groupItem->groupName() == documentGroup)
            {
                group = groupItem;
                break;
            }
        }
    }

    if (group == 0)
    {
        group = new UBDocumentGroupTreeItem(0); // deleted by the tree widget
        group->setGroupName(documentGroup);
        mDocumentUI->documentTreeWidget->addTopLevelItem(group);
    }

    UBDocumentProxyTreeItem *ti = new UBDocumentProxyTreeItem(group, pDocument, !group->isTrashFolder());
    ti->setText(0, documentName);
}


void UBDocumentController::updateDocumentInTree(UBDocumentProxy* pDocument)
{
    QTreeWidgetItemIterator it(mDocumentUI->documentTreeWidget);
    while (*it)
    {
        UBDocumentProxyTreeItem* pi = dynamic_cast<UBDocumentProxyTreeItem*>((*it));

        if (pi && pi->proxy() == pDocument)
        {
            pi->setText(0, pDocument->name());
            break;
        }
        ++it;
    }

}


QStringList UBDocumentController::allGroupNames()
{
    QStringList result;

    for (int i = 0; i < mDocumentUI->documentTreeWidget->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = mDocumentUI->documentTreeWidget->topLevelItem(i);
        UBDocumentGroupTreeItem* groupItem = dynamic_cast<UBDocumentGroupTreeItem*>(item);
        result << groupItem->groupName();
    }

    return result;
}


void UBDocumentController::renameSelectedItem()
{
    if (mDocumentUI->documentTreeWidget->selectedItems().count() > 0)
        mDocumentUI->documentTreeWidget->editItem(mDocumentUI->documentTreeWidget->selectedItems().at(0));
}


bool UBDocumentController::isOKToOpenDocument(UBDocumentProxy* proxy)
{
    //check version
    QString docVersion = proxy->metaData(UBSettings::documentVersion).toString();

    if (docVersion.isEmpty() || docVersion.startsWith("4.1") || docVersion.startsWith("4.2")
            || docVersion.startsWith("4.3") || docVersion.startsWith("4.4") || docVersion.startsWith("4.5")
            || docVersion.startsWith("4.6")) // TODO UB 4.7 update if necessary
    {
        return true;
    }
    else
    {
        if (UBApplication::mainWindow->yesNoQuestion(tr("Open Document"),
                tr("The document '%1' has been generated with a newer version of Sankore (%2). By opening it, you may lose some information. Do you want to proceed?")
                    .arg(proxy->metaData(UBSettings::documentName).toString())
                    .arg(docVersion)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


void UBDocumentController::showMessage(const QString& message, bool showSpinningWheel)
{
    if (mMessageWindow)
    {
        int margin = UBSettings::boardMargin;

        QRect newSize = mDocumentUI->thumbnailWidget->geometry();

        #ifdef Q_WS_MACX
            QPoint point(newSize.left() + margin, newSize.bottom() - mMessageWindow->height() - margin);
            mMessageWindow->move(mDocumentUI->thumbnailWidget->mapToGlobal(point));
        #else
            mMessageWindow->move(margin, newSize.height() - mMessageWindow->height() - margin);
        #endif

        mMessageWindow->showMessage(message, showSpinningWheel);
    }
}


void UBDocumentController::hideMessage()
{
    if (mMessageWindow)
        mMessageWindow->hideMessage();
}


void UBDocumentController::addImages()
{
    UBDocumentProxy* document = selectedDocumentProxy();

    if (document)
    {
        QString defaultPath = UBSettings::settings()->lastImportFolderPath->get().toString();

        QString extensions;

        foreach (QString ext, UBSettings::settings()->imageFileExtensions)
        {
            extensions += " *.";
            extensions += ext;
        }

        QStringList images = QFileDialog::getOpenFileNames(mParentWidget, tr("Add all Images to Document"),
                defaultPath, tr("All Images (%1)").arg(extensions));

        if (images.length() > 0)
        {
            QFileInfo firstImage(images.at(0));

            UBSettings::settings()->lastImportFolderPath->set(QVariant(firstImage.absoluteDir().absolutePath()));

            int importedImageNumber
                = UBDocumentManager::documentManager()->addFilesToDocument(document, images);

            if (importedImageNumber == 0)
            {
                UBApplication::showMessage(tr("Selection does not contain any image files!"));
                UBApplication::applicationController->showDocument();
            }
            else
            {
                document->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
                UBMetadataDcSubsetAdaptor::persist(document);
                reloadThumbnails();
            }
        }
    }
}


void UBDocumentController::toggleDocumentToolsPalette()
{
    if (!mToolsPalette->isVisible() && !mToolsPalettePositionned)
    {
        mToolsPalette->adjustSizeAndPosition();
        int left = controlView()->width() - 20 - mToolsPalette->width();
        int top = (controlView()->height() - mToolsPalette->height()) / 2;

        mToolsPalette->setCustomPosition(true);
        mToolsPalette->move(left, top);

        mToolsPalettePositionned = true;
    }

    bool visible = mToolsPalette->isVisible();
    mToolsPalette->setVisible(!visible);
}


void UBDocumentController::cut()
{
    // TODO - implemented me
}


void UBDocumentController::copy()
{
    // TODO - implemented me
}


void UBDocumentController::paste()
{
    // TODO - implemented me
}


void UBDocumentController::focusChanged(QWidget *old, QWidget *current)
{
    Q_UNUSED(old);

    if (current == mDocumentUI->thumbnailWidget)
    {
        if (mDocumentUI->thumbnailWidget->selectedItems().count() > 0)
            mSelectionType = Page;
        else
            mSelectionType = None;
    }
    else if (current == mDocumentUI->documentTreeWidget)
    {
        if (selectedDocumentProxy())
            mSelectionType = Document;
        else if (selectedDocumentGroupTreeItem())
            mSelectionType = Folder;
        else
            mSelectionType = None;
    }
    else if (current == mDocumentUI->documentZoomSlider)
    {
        if (mDocumentUI->thumbnailWidget->selectedItems().count() > 0)
            mSelectionType = Page;
        else
            mSelectionType = None;
    }
    else
    {
        if (old != mDocumentUI->thumbnailWidget &&
            old != mDocumentUI->documentTreeWidget &&
            old != mDocumentUI->documentZoomSlider)
        {
            mSelectionType = None;
        }
    }

    selectionChanged();
}

void UBDocumentController::deletePages(QList<QGraphicsItem *> itemsToDelete)
{
    if (itemsToDelete.count() > 0)
    {
        QList<int> sceneIndexes;
        UBDocumentProxy* proxy = 0;

        foreach (QGraphicsItem* item, itemsToDelete)
        {
            UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (item);

            if (thumb)
            {
                proxy = thumb->proxy();
                if (proxy)
                {
                    sceneIndexes.append(thumb->sceneIndex());
                }
            }
        }

        if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Page"), tr("Are you sure you want to remove %n page(s) from the selected document '%1'?", "", sceneIndexes.count()).arg(proxy->metaData(UBSettings::documentName).toString())))
        {
            UBDocumentContainer::deletePages(sceneIndexes);

            proxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(proxy);

            int minIndex = proxy->pageCount() - 1;
            foreach (int i, sceneIndexes)
                 minIndex = qMin(i, minIndex);

            mDocumentUI->thumbnailWidget->selectItemAt(minIndex);
        }
    }
}

int UBDocumentController::getSelectedItemIndex()
{
    QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();

    if (selectedItems.count() > 0)
    {
        UBSceneThumbnailPixmap* thumb = dynamic_cast<UBSceneThumbnailPixmap*> (selectedItems.last());
        return thumb->sceneIndex();
    }
    else return -1;
}

bool UBDocumentController::pageCanBeMovedUp(int page)
{
    if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool())
        return page >= 2;
    else
        return page >= 1;
}

bool UBDocumentController::pageCanBeMovedDown(int page)
{
    if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool())
        return page != 0 && page < selectedDocument()->pageCount() - 1;
    else
        return page < selectedDocument()->pageCount() - 1;
}

bool UBDocumentController::pageCanBeDuplicated(int page)
{
    return page != 0;
}

bool UBDocumentController::pageCanBeDeleted(int page)
{
    return page != 0;
}

void UBDocumentController::setDocument(UBDocumentProxy *document, bool forceReload)
{
    UBDocumentContainer::setDocument(document, forceReload);
//    UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->setCurrentDocument(document);
}

void UBDocumentController::refreshDocumentThumbnailsView(UBDocumentContainer*)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QList<QGraphicsItem*> items;
    QList<QUrl> itemsPath;

    UBDocumentProxy *proxy = selectedDocumentProxy();
    QGraphicsPixmapItem *selection = 0;

    QStringList labels;

    if (proxy)
    {
        setDocument(proxy);

        for (int i = 0; i < selectedDocument()->pageCount(); i++)
        {
            const QPixmap* pix = pageAt(i);
            QGraphicsPixmapItem *pixmapItem = new UBSceneThumbnailPixmap(*pix, proxy, i); // deleted by the tree widget

            if (proxy == mBoardController->selectedDocument() && mBoardController->activeSceneIndex() == i)
            {
                selection = pixmapItem;
            }

            items << pixmapItem;
            int pageIndex = pageFromSceneIndex(i);
            if(pageIndex)
                labels << tr("Page %1").arg(pageIndex);
            else
                labels << tr("Title page");

            itemsPath.append(QUrl::fromLocalFile(proxy->persistencePath() + QString("/pages/%1").arg(UBDocumentContainer::pageFromSceneIndex(i))));
        }
    }

    mDocumentUI->thumbnailWidget->setGraphicsItems(items, itemsPath, labels, UBApplication::mimeTypeUniboardPage);

    UBDocumentProxyTreeItem* proxyTi = selectedDocumentProxyTreeItem();
    if (proxyTi && (proxyTi->parent() == mTrashTi))
        mDocumentUI->thumbnailWidget->setDragEnabled(false);
    else
        mDocumentUI->thumbnailWidget->setDragEnabled(true);

    mDocumentUI->thumbnailWidget->ensureVisible(0, 0, 10, 10);

    if (selection) {
        disconnect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));
        UBSceneThumbnailPixmap *currentScene = dynamic_cast<UBSceneThumbnailPixmap*>(selection);
        if (currentScene)
            mDocumentUI->thumbnailWidget->hightlightItem(currentScene->sceneIndex());
        connect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));
    }

    QApplication::restoreOverrideCursor();
}
