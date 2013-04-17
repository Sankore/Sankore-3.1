/*
 * Copyright (C) 2012 Webdoc SA
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
#include "core/UBMimeData.h"

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

static bool lessThan(UBDocumentTreeNode *lValue, UBDocumentTreeNode *rValue)
{
    if (lValue->nodeType() == UBDocumentTreeNode::Catalog) {
        if (rValue->nodeType() == UBDocumentTreeNode::Catalog) {
            return lValue->nodeName() < rValue->nodeName();
        } else {
            return true;
        }
    } else {
        if (rValue->nodeType() == UBDocumentTreeNode::Catalog) {
            return false;
        } else {
            Q_ASSERT(lValue->proxyData());
            Q_ASSERT(rValue->proxyData());

            QDateTime lTime = lValue->proxyData()->documentDate();
            QDateTime rTime = rValue->proxyData()->documentDate();

            return lTime > rTime;
        }
    }

    return false;
}

UBDocumentReplaceDialog::UBDocumentReplaceDialog(const QString &pIncommingName, const QStringList &pFileList, QWidget *parent, Qt::WindowFlags pFlags)
    : QDialog(parent, pFlags)
    , mFileNameList(pFileList)
    , mIncommingName(pIncommingName)
    , acceptText(tr("Accept"))
    , replaceText(tr("Replace"))
    , cancelText(tr("Cancel"))
    , mLabelText(0)
{
    this->setStyleSheet("background:white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QVBoxLayout *labelLayout = new QVBoxLayout();

    mLabelText = new QLabel(labelTextWithName(pIncommingName), this);
    mLineEdit = new QLineEdit(this);
    mLineEdit->setText(pIncommingName);
    mLineEdit->selectedText();

    mValidator = new QRegExpValidator(QRegExp("[^\\/\\:\\?\\*\\|\\<\\>\\\"]{1,}"), this);
    mLineEdit->setValidator(mValidator);
    labelLayout->addWidget(mLabelText);
    labelLayout->addWidget(mLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    acceptButton = new QPushButton(acceptText, this);
    QPushButton *cancelButton = new QPushButton(cancelText, this);
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(labelLayout);
    mainLayout->addLayout(buttonLayout);

    acceptButton->setEnabled(false);

    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(mLineEdit, SIGNAL(textEdited(QString)), this, SLOT(reactOnTextChanged(QString)));

    reactOnTextChanged(mIncommingName);
}

void UBDocumentReplaceDialog::setRegexp(const QRegExp pRegExp)
{
    mValidator->setRegExp(pRegExp);
}
bool UBDocumentReplaceDialog::validString(const QString &pStr)
{
    Q_UNUSED(pStr);
    return mLineEdit->hasAcceptableInput();
}

void UBDocumentReplaceDialog::setFileNameAndList(const QString &fileName, const QStringList &pLst)
{
    mFileNameList = pLst;
    mIncommingName = fileName;
    mLabelText->setText(labelTextWithName(fileName));
    mLineEdit->setText(fileName);
    mLineEdit->selectedText();
}

QString UBDocumentReplaceDialog::labelTextWithName(const QString &documentName) const
{
    return tr("The name %1 is allready used.\nKeeping this name will replace the document.\nProviding a new name will create a new document.")
            .arg(documentName);
}

void UBDocumentReplaceDialog::accept()
{
    QDialog::accept();
}
void UBDocumentReplaceDialog::reject()
{
    mLineEdit->clear();
    emit closeDialog();

    QDialog::reject();
}

void UBDocumentReplaceDialog::reactOnTextChanged(const QString &pStr)
{
//     if !mFileNameList.contains(pStr.trimmed(), Qt::CaseSensitive)

    if (!validString(pStr)) {
        acceptButton->setEnabled(false);
        mLineEdit->setStyleSheet("background:#FFB3C8;");
        acceptButton->setEnabled(false);

    } else if (mFileNameList.contains(pStr.trimmed(), Qt::CaseSensitive)) {
        acceptButton->setEnabled(true);
        mLineEdit->setStyleSheet("background:#FFB3C8;");
        acceptButton->setText(replaceText);

    } else {
        acceptButton->setEnabled(true);
        mLineEdit->setStyleSheet("background:white;");
        acceptButton->setText(acceptText);
    }
}

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

void UBDocumentTreeNode::insertChild(int pIndex, UBDocumentTreeNode *pChild)
{
    if (pChild) {
        mChildren.insert(pIndex, pChild);
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
    delete curChild;
}

UBDocumentTreeNode *UBDocumentTreeNode::clone()
{
    return new UBDocumentTreeNode(this->mType
                                  , this->mName
                                  , this->mDisplayName
                                  , this->mProxy ? new UBDocumentProxy(*this->mProxy)
                                                 : 0);
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

    mRoot = index(0, 0, QModelIndex());
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
    if (!nodeParent || row > nodeParent->children().count() - 1) {
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

    if (index.column() == 0) {

        switch (role) {
        case (Qt::DecorationRole) :
            if (mCurrentNode && mCurrentNode == dataNode) {
                return QIcon(":images/currentDocument.png");
            } else {
                if (index == trashIndex()) {
                    return QIcon(":images/trash.png");
                } else if (isConstant(index)) {
                    return QIcon(":images/libpalette/ApplicationsCategory.svg");
                }
                switch (static_cast<int>(dataNode->nodeType())) {
                case UBDocumentTreeNode::Catalog :
                    return QIcon(":images/folder.png");
                case UBDocumentTreeNode::Document :
                    return QIcon(":images/toolbar/board.png");
                }
            }
            break;

        case (Qt::DisplayRole) :
            return dataNode->displayName();
            break;

        case (Qt::UserRole +1):
            return QVariant::fromValue(dataNode);
            break;

        case (Qt::FontRole) :
            if (isConstant(index)) {
                QFont font;
                font.setBold(true);
                return font;
            }
            break;

        case (Qt::ForegroundRole) :
            if (isConstant(index)) {
                return Qt::darkGray;
            }
            break;

        case (Qt::BackgroundRole) :
            if (isConstant(index)) {
                return QBrush(0xD9DFEB);
            }
            if (mHighLighted.isValid() && index == mHighLighted) {
                return QBrush(0x6682B5);
            }
            break;
        }
    }

    return QVariant();
}

bool UBDocumentTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    switch (role) {
    case Qt::EditRole:
        if (!index.isValid() || value.toString().isEmpty()) {
            return false;
        }
        setNewName(index, value.toString());
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags UBDocumentTreeModel::flags (const QModelIndex &index) const
{
    Qt::ItemFlags resultFlags = QAbstractItemModel::flags(index);
    UBDocumentTreeNode *indexNode = nodeFromIndex(index);

    if ( index.isValid() ) {
        if (!indexNode->isRoot() && !isConstant(index)) {
            if (!inTrash(index)) {
                resultFlags |= Qt::ItemIsEditable;
            }
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
    if (data->hasFormat(UBApplication::mimeTypeUniboardPage)) {
        UBDocumentTreeNode *curNode = nodeFromIndex(index(row - 1, column, parent));
        UBDocumentProxy *targetDocProxy = curNode->proxyData();
        const UBMimeData *ubMime = qobject_cast <const UBMimeData*>(data);
        if (!targetDocProxy || !ubMime || !ubMime->items().count()) {
            qDebug() << "an error ocured while parsing " << UBApplication::mimeTypeUniboardPage;
            return false;
        }

        int count = 0;
        int total = ubMime->items().size();

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        foreach (UBMimeDataItem sourceItem, ubMime->items())
        {
            count++;

            UBApplication::applicationController->showMessage(tr("Copying page %1/%2").arg(count).arg(total), true);

            // TODO UB 4.x Move following code to some controller class
            UBGraphicsScene *scene = UBPersistenceManager::persistenceManager()->loadDocumentScene(sourceItem.documentProxy(), sourceItem.sceneIndex());
            if (scene) {
                foreach (QUrl relativeFile, scene->relativeDependencies()) {
                    QString source = scene->document()->persistencePath() + "/" + relativeFile.toString();
                    QString target = targetDocProxy->persistencePath() + "/" + relativeFile.toString();

                    QFileInfo fi(target);
                    QDir d = fi.dir();

                    d.mkpath(d.absolutePath());
                    QFile::copy(source, target);
                }

                UBPersistenceManager::persistenceManager()->insertDocumentSceneAt(targetDocProxy, scene, targetDocProxy->pageCount());

                //due to incorrect generation of thumbnails of invisible scene I've used direct copying of thumbnail files
                //it's not universal and good way but it's faster
                QString from = sourceItem.documentProxy()->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", sourceItem.sceneIndex());
                QString to  = targetDocProxy->persistencePath() + UBFileSystemUtils::digitFileFormat("/page%1.thumbnail.jpg", targetDocProxy->pageCount());
                QFile::remove(to);
                if (!UBFileSystemUtils::copy(from, to)) {
                    qDebug() << "can't copy from " << from << "to" << to;
                }
            }
        }

        QApplication::restoreOverrideCursor();

        UBApplication::applicationController->showMessage(tr("%1 pages copied", "", total).arg(total), false);

        return true;
    }



    const UBDocumentTreeMimeData *mimeData = qobject_cast<const UBDocumentTreeMimeData*>(data);
    if (!mimeData) {
        qDebug() << "Incorrect mimeData, only internal one supported";
        return false;
    }
    if (!parent.isValid()) {
        return false;
    }
    UBDocumentTreeNode *newParentNode = nodeFromIndex(parent);

    if (!newParentNode) {
        qDebug() << "incorrect incoming parent node;";
        return false;
    }

    QList<QModelIndex> incomingIndexes = mimeData->indexes();

    foreach (QModelIndex curIndex, incomingIndexes) {
        QModelIndex clonedTopLevel = copyIndexToNewParent(curIndex, parent, action == Qt::MoveAction ? aReference : aContentCopy);
        if (nodeFromIndex(curIndex) == mCurrentNode && action == Qt::MoveAction) {
            emit currentIndexMoved(clonedTopLevel, curIndex);
        }
    }

    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    return true;
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
        mNewDocuments.removeAll(curChildNode->proxyData());
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

    return pIndexForNode(QModelIndex(), pNode);
}

QPersistentModelIndex UBDocumentTreeModel::persistentIndexForNode(UBDocumentTreeNode *pNode)
{
    return QPersistentModelIndex(indexForNode(pNode));
}

UBDocumentTreeNode *UBDocumentTreeModel::findProxy(UBDocumentProxy *pSearch, UBDocumentTreeNode *pParent) const
{
    foreach (UBDocumentTreeNode *curNode, pParent->children())
    {
        if (UBDocumentTreeNode::Catalog != curNode->nodeType())
        {
            if (curNode->proxyData()->theSameDocument(pSearch))
                return curNode;
        }
        else if (curNode->children().count())
        {
            UBDocumentTreeNode *recursiveDescendResult = findProxy(pSearch, curNode);
            if (recursiveDescendResult)
                return findProxy(pSearch, curNode);
        }
    }

    return 0;
}

QModelIndex UBDocumentTreeModel::pIndexForNode(const QModelIndex &parent, UBDocumentTreeNode *pNode) const
{
    for (int i = 0; i < rowCount(parent); i++) {
        QModelIndex curIndex = index(i, 0, parent);
        if (curIndex.internalPointer() == pNode) {
            return curIndex;
        } else if (rowCount(curIndex) > 0) {
            QModelIndex recursiveDescendIndex = pIndexForNode(curIndex, pNode);
            if (recursiveDescendIndex.isValid()) {
                return recursiveDescendIndex;
            }
        }
    }
    return QModelIndex();
}

QPersistentModelIndex UBDocumentTreeModel::copyIndexToNewParent(const QModelIndex &source, const QModelIndex &newParent, eCopyMode pMode)
{
    UBDocumentTreeNode *nodeParent = nodeFromIndex(newParent);
    UBDocumentTreeNode *nodeSource = nodeFromIndex(source);

    if (!nodeParent || !nodeSource) {
        return QModelIndex();
    }

    beginInsertRows(newParent, rowCount(newParent), rowCount(newParent));

    UBDocumentTreeNode *clonedNodeSource = 0;
    switch (static_cast<int>(pMode)) {
    case aReference:
        clonedNodeSource = nodeSource->clone();
        if (mNewDocuments.contains(nodeSource->proxyData())) { //update references for session documents
            mNewDocuments << clonedNodeSource->proxyData();

            UBPersistenceManager::persistenceManager()->reassignDocProxy(clonedNodeSource->proxyData(), nodeSource->proxyData());
        }
        break;

    case aContentCopy:
        UBDocumentProxy* duplicatedProxy = 0;
        if (nodeSource->nodeType() == UBDocumentTreeNode::Document && nodeSource->proxyData()) {
            duplicatedProxy = UBPersistenceManager::persistenceManager()->duplicateDocument(nodeSource->proxyData());
            duplicatedProxy->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
            UBMetadataDcSubsetAdaptor::persist(duplicatedProxy);
        }
        clonedNodeSource = new UBDocumentTreeNode(nodeSource->nodeType()
                                                  , nodeSource->nodeName()
                                                  , nodeSource->displayName()
                                                  , duplicatedProxy);
        break;
    }

    // Determine whether to provide a name with postfix if the name in current level allready exists
    QString newName = clonedNodeSource->nodeName();
    if ((source.parent() != newParent
            || pMode != aReference)
            && (newParent != trashIndex() || !inTrash(newParent))) {
        newName = adjustNameForParentIndex(newName, newParent);
        clonedNodeSource->setNodeName(newName);
    }

    if (clonedNodeSource->proxyData()) {
        clonedNodeSource->proxyData()->setMetaData(UBSettings::documentGroupName, virtualPathForIndex(newParent));
        clonedNodeSource->proxyData()->setMetaData(UBSettings::documentName, newName);
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(clonedNodeSource->proxyData());
    }

    addNode(clonedNodeSource, newParent);
//    nodeParent->addChild(clonedNodeSource);
    endInsertRows();

    QPersistentModelIndex newParentIndex = createIndex(rowCount(newParent), 0, clonedNodeSource);

    if (rowCount(source)) {
        for (int i = 0; i < rowCount(source); i++) {
            QModelIndex curNewParentIndexChild = source.child(i, 0);
            copyIndexToNewParent(curNewParentIndexChild, newParentIndex, pMode);
        }
    }

    return newParentIndex;
}

void UBDocumentTreeModel::moveIndex(const QModelIndex &source, const QModelIndex &newParent)
{
    UBDocumentTreeNode *sourceNode = nodeFromIndex(source);
    QPersistentModelIndex clonedTopLevel = copyIndexToNewParent(source, newParent);
    if (sourceNode == mCurrentNode) {
        mCurrentNode = nodeFromIndex(clonedTopLevel);
        emit currentIndexMoved(clonedTopLevel, source);
    }
    removeRow(source.row(), source.parent());
}

void UBDocumentTreeModel::moveNode(const QModelIndex &source, const QModelIndex &newParent)
{
    Q_ASSERT(source.parent().isValid());

    UBDocumentTreeNode *sourceNode = nodeFromIndex(source);
    UBDocumentTreeNode *newParentNode = nodeFromIndex(newParent);

    int destinationPosition = positionForParent(sourceNode, newParentNode);

    if (source.row() != destinationPosition || source.parent() == newParent) {
        beginMoveRows(source.parent(), source.row(), source.row(), newParent, destinationPosition);
        sourceNode->parentNode()->children().removeAt(source.row());
        newParentNode->insertChild(destinationPosition, sourceNode);
        endMoveRows();
    }
}

void UBDocumentTreeModel::setCurrentDocument(UBDocumentProxy *pDocument)
{
    UBDocumentTreeNode *testCurNode = findProxy(pDocument, mRootNode);

    if (testCurNode) {
        setCurrentNode(testCurNode);
    }
}

QModelIndex UBDocumentTreeModel::indexForProxy(UBDocumentProxy *pSearch) const
{
//    qDebug() << "looking for proxy" << pSearch;
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

QString UBDocumentTreeModel::virtualPathForIndex(const QModelIndex &pIndex) const
{
    UBDocumentTreeNode *curNode = nodeFromIndex(pIndex);
    Q_ASSERT(curNode);

    return virtualDirForIndex(pIndex) + "/" + curNode->nodeName();
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

bool UBDocumentTreeModel::newNodeAllowed(const QModelIndex &pSelectedIndex)  const
{
    if (!pSelectedIndex.isValid()) {
        return false;
    }

    if (inTrash(pSelectedIndex) || pSelectedIndex == trashIndex()) {
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

bool UBDocumentTreeModel::inTrash(const QModelIndex &index) const
{
    return isDescendantOf(index, trashIndex());
}

bool UBDocumentTreeModel::inModel(const QModelIndex &index) const
{
    return isDescendantOf(index, modelsIndex());
}

bool UBDocumentTreeModel::inUntitledDocuments(const QModelIndex &index) const
{
    return isDescendantOf(index, untitledDocumentsIndex());
}

void UBDocumentTreeModel::addDocument(UBDocumentProxy *pProxyData, const QModelIndex &pParent)
{
    if (!pProxyData) {
        return;
    }
    QString docName = pProxyData->metaData(UBSettings::documentName).toString();
    QString docGroupName = pProxyData->metaData(UBSettings::documentGroupName).toString();

    if (docName.isEmpty()) {
        return;
    }

    QModelIndex lParent = pParent;
    UBDocumentTreeNode *freeNode = new UBDocumentTreeNode(UBDocumentTreeNode::Document
                                                          , docName
                                                          , QString()
                                                          , pProxyData);
    if (!pParent.isValid()) {
        lParent = goTo(docGroupName);
    }

    addNode(freeNode, lParent);
}

void UBDocumentTreeModel::addNewDocument(UBDocumentProxy *pProxyData, const QModelIndex &pParent)
{
    addDocument(pProxyData, pParent);
    mNewDocuments << pProxyData;
}

void UBDocumentTreeModel::addCatalog(const QString &pName, const QModelIndex &pParent)
{
    if (pName.isEmpty() || !pParent.isValid()) {
        return;
    }

    UBDocumentTreeNode *catalogNode = new UBDocumentTreeNode(UBDocumentTreeNode::Catalog, pName);
    addNode(catalogNode, pParent);
}

void UBDocumentTreeModel::setNewName(const QModelIndex &index, const QString &newName)
{
    if (!index.isValid()) {
        return;
    }

    UBDocumentTreeNode *indexNode = nodeFromIndex(index);

    QString magicSeparator = "+!##s";
    if (isCatalog(index)) {
        QString fullNewName = newName;
        if (!newName.contains(magicSeparator)) {
            indexNode->setNodeName(newName);
            QString virtualDir = virtualDirForIndex(index);
            fullNewName.prepend(virtualDir.isEmpty() ? "" : virtualDir + magicSeparator);
        }
        for (int i = 0; i < rowCount(index); i++) {
            QModelIndex subIndex = this->index(i, 0, index);
            setNewName(subIndex, fullNewName + magicSeparator + subIndex.data().toString());
        }

    } else if (isDocument(index)) {
        Q_ASSERT(indexNode->proxyData());

        int prefixIndex = newName.lastIndexOf(magicSeparator);
        if (prefixIndex != -1) {
            QString newDocumentGroupName = newName.left(prefixIndex).replace(magicSeparator, "/");
            indexNode->proxyData()->setMetaData(UBSettings::documentGroupName, newDocumentGroupName);
        } else {
            indexNode->setNodeName(newName);
            indexNode->proxyData()->setMetaData(UBSettings::documentName, newName);
        }

        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(indexNode->proxyData());
    }
}

QString UBDocumentTreeModel::adjustNameForParentIndex(const QString &pName, const QModelIndex &pIndex)
{
    int i = 0;
    QString newName = pName;
    QStringList siblingNames = nodeNameList(pIndex);
    while (siblingNames.contains(newName)) {
        newName = pName + " " + QVariant(++i).toString();
    }

    return newName;
}

bool UBDocumentTreeModel::isDescendantOf(const QModelIndex &pPossibleDescendant, const QModelIndex &pPossibleAncestor) const
{
    if (!pPossibleDescendant.isValid()) {
        return false;
    }

    QModelIndex ancestor = pPossibleDescendant;
    while (ancestor.parent().isValid()) {
        ancestor = ancestor.parent();
        if (ancestor == pPossibleAncestor) {
            return true;
        }
    }

    return false;
}

QModelIndex UBDocumentTreeModel::addNode(UBDocumentTreeNode *pFreeNode, const QModelIndex &pParent, eAddItemMode pMode)
{
    UBDocumentTreeNode *tstParent = nodeFromIndex(pParent);

    if (!pParent.isValid() || tstParent->nodeType() != UBDocumentTreeNode::Catalog) {
        return QModelIndex();
    }
    int newIndex = pMode == aDetectPosition ? positionForParent(pFreeNode, tstParent): tstParent->children().size();
    beginInsertRows(pParent, newIndex, newIndex);
    tstParent->insertChild(newIndex, pFreeNode);
    endInsertRows();

    return createIndex(newIndex, 0, pFreeNode);
}

int UBDocumentTreeModel::positionForParent(UBDocumentTreeNode *pFreeNode, UBDocumentTreeNode *pParentNode)
{
    Q_ASSERT(pFreeNode);
    Q_ASSERT(pParentNode);
    Q_ASSERT(pParentNode->nodeType() == UBDocumentTreeNode::Catalog);

    int c = -1;
    int childCount = pParentNode->children().count();
    while (c <= childCount) {
        if (++c == childCount || lessThan(pFreeNode, pParentNode->children().at(c))) {
            break;
        }
    }
    return c == -1 ? childCount : c;
}

UBDocumentTreeNode *UBDocumentTreeModel::nodeFromIndex(const QModelIndex &pIndex) const
{
    if (pIndex.isValid()) {
        return static_cast<UBDocumentTreeNode*>(pIndex.internalPointer());
    } else {
        return mRootNode;
    }
}

void UBDocumentTreeModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(order)
    Q_UNUSED(column)

//  QModelIndex parentIndex = index(0, column, QModelIndex());
    sortChilds(mRoot);
}

void UBDocumentTreeModel::sortChilds(const QModelIndex &parentIndex)
{
    int current_column = 0;
    int current_row = 0;

    QList <UBDocumentTreeNode*> catalogsForSort;
    QList <UBDocumentTreeNode*> documentsForSort;

    for (current_row = 0; current_row < rowCount(parentIndex); current_row++)
    {
        QModelIndex currentIndex = index(current_row, current_column, parentIndex);
        if (isCatalog(currentIndex))
            catalogsForSort << nodeFromIndex(currentIndex);
        else
            documentsForSort << nodeFromIndex(currentIndex);
    }

    sortIndexes(catalogsForSort);
    sortIndexes(documentsForSort);



    foreach (UBDocumentTreeNode *node, catalogsForSort)
    {
        sortChilds(indexForNode(node));
    }

    foreach(UBDocumentTreeNode *node, catalogsForSort)
    {
        QModelIndex currentIndex = indexForNode(node);
        moveIndex(currentIndex, parentIndex);
    }

    for (int i = documentsForSort.count()-1; i >= 0 ; i--)
    {
        QModelIndex currentIndex = indexForNode(documentsForSort.at(i));
        moveIndex(currentIndex, parentIndex);
    }
}


void UBDocumentTreeModel::sortIndexes(QList<UBDocumentTreeNode *> &unsortedIndexList)
{
    qStableSort(unsortedIndexList.begin(), unsortedIndexList.end(), nodeLessThan);
}

bool UBDocumentTreeModel::nodeLessThan(const UBDocumentTreeNode *firstIndex, const UBDocumentTreeNode *secondIndex)
{
    return firstIndex->nodeName() < secondIndex->nodeName();
}

UBDocumentTreeModel::~UBDocumentTreeModel()
{
    delete mRootNode;
}

UBDocumentTreeView::UBDocumentTreeView(QWidget *parent) : QTreeView(parent)
{
    setObjectName("UBDocumentTreeView");
    setRootIsDecorated(true);

//    QScrollBar *hScroller = horizontalScrollBar();
//    if (hScroller)
//    {
//        connect(hScroller, SIGNAL(rangeChanged(int, int)), this, SLOT(hSliderRangeChanged(int, int)));
//    }

    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(adjustSize()));
    connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(adjustSize()));
    connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(adjustSize()));
}

void UBDocumentTreeView::setSelectedAndExpanded(const QModelIndex &pIndex, bool pExpand)
{
    if (!pIndex.isValid()) {
        return;
    }

    QModelIndex indexCurrentDoc = pIndex;
    clearSelection();
     selectionModel()->select(indexCurrentDoc, pExpand
                             ? QItemSelectionModel::Select
                             : QItemSelectionModel::Deselect);
    setCurrentIndex(pExpand
                    ? indexCurrentDoc
                    : QModelIndex());

    while (indexCurrentDoc.parent().isValid()) {
        setExpanded(indexCurrentDoc.parent(), pExpand);
        indexCurrentDoc = indexCurrentDoc.parent();
    }
}

void UBDocumentTreeView::onModelIndexChanged(const QModelIndex &pNewIndex, const QModelIndex &pOldIndex)
{
    Q_UNUSED(pOldIndex)
    setSelectedAndExpanded(pNewIndex, true);
}

void UBDocumentTreeView::hSliderRangeChanged(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);

    QScrollBar *hScroller = horizontalScrollBar();
    if (hScroller)
    {
        hScroller->triggerAction(QAbstractSlider::SliderToMaximum);
    }
}

void UBDocumentTreeView::dragEnterEvent(QDragEnterEvent *event)
{
    QTreeView::dragEnterEvent(event);
    event->accept();
    event->acceptProposedAction();
}

void UBDocumentTreeView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    UBDocumentTreeModel *docModel = qobject_cast<UBDocumentTreeModel*>(model());
    docModel->setHighLighted(QModelIndex());
    update();
}

void UBDocumentTreeView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage)) {
        UBDocumentTreeModel *docModel = qobject_cast<UBDocumentTreeModel*>(model());
        QModelIndex targetIndex = indexAt(event->pos());
        if (!docModel || !docModel->isDocument(targetIndex) || docModel->inTrash(targetIndex)) {
            event->ignore();
            docModel->setHighLighted(QModelIndex());
            updateIndexEnvirons(targetIndex);
            return;
        }
        docModel->setHighLighted(targetIndex);
        updateIndexEnvirons(targetIndex);
    }
    QTreeView::dragMoveEvent(event);
    event->setAccepted(isAcceptable(selectedIndexes().first(), indexAt(event->pos())));
}

void UBDocumentTreeView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(UBApplication::mimeTypeUniboardPage)) {
        UBDocumentTreeModel *docModel = qobject_cast<UBDocumentTreeModel*>(model());
        QModelIndex targetIndex = indexAt(event->pos());
        qDebug() << "target index drop event" << docModel->nodeFromIndex(targetIndex)->nodeName();
        if (!docModel || !docModel->isDocument(targetIndex)) {
            event->ignore();
            return;
        }
        docModel->setHighLighted(QModelIndex());
        event->setDropAction(Qt::CopyAction);
    } else {
        event->setDropAction(acceptableAction(selectedIndexes().first(), indexAt(event->pos())));
    }
    QTreeView::dropEvent(event);
    adjustSize();
}

void UBDocumentTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);
}

void UBDocumentTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
//    //managing current index inside the application. There is no common solution for that
//    Q_UNUSED(parent)
//    Q_UNUSED(start)
//    Q_UNUSED(end)

//    for (int i = start; i <= end; i++) {
//        if (currentIndex() == model()->index(i, 0, parent)) {
//            qDebug() << "catched";
//        }
//    }

////    setState(CollapsingState);
//    updateGeometries();

    QTreeView::rowsAboutToBeRemoved(parent, start, end);
}

bool UBDocumentTreeView::isAcceptable(const QModelIndex &dragIndex, const QModelIndex &atIndex)
{
    if (fullModel()->inModel(dragIndex)) {
        if (atIndex == fullModel()->modelsIndex() || fullModel()->inModel(atIndex)) {
            return false; //do not accept drop from model to itself
        }
    }

    return true;
}

Qt::DropAction UBDocumentTreeView::acceptableAction(const QModelIndex &dragIndex, const QModelIndex &atIndex)
{
    if (fullModel()->inModel(dragIndex)) {
        if (atIndex == fullModel()->trashIndex() || fullModel()->inTrash(atIndex)) {
            return Qt::MoveAction; //do not accept drop from model to trash, only "delete" command accepted
        }
        return Qt::CopyAction;
    } else {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

void UBDocumentTreeView::updateIndexEnvirons(const QModelIndex &index)
{
    QRect updateRect = visualRect(index);
    const int multipler = 3;
    updateRect.adjust(0, -updateRect.height() * multipler, 0, updateRect.height() * multipler);
    update(updateRect);
}

void UBDocumentTreeView::adjustSize()
{
    resizeColumnToContents(0);

    int headerSizeHint = width();

    if (verticalScrollBar()->isVisible() && verticalScrollBar()->maximum() > 0)
        headerSizeHint -= verticalScrollBar()->width();

    if (columnWidth(0) < headerSizeHint)
        setColumnWidth(0, headerSizeHint);
}

UBDocumentTreeItemDelegate::UBDocumentTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void UBDocumentTreeItemDelegate::commitAndCloseEditor()
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit*>(sender());
    if (lineEditor) {
        emit commitData(lineEditor);
        emit closeEditor(lineEditor);
    }
}

void UBDocumentTreeItemDelegate::processChangedText(const QString &str) const
{
    QLineEdit *editor = qobject_cast<QLineEdit*>(sender());
    if (!editor) {
        return;
    }

    if (!validateString(str)) {
        editor->setStyleSheet("background-color: #FFB3C8;");
    } else {
        editor->setStyleSheet("background-color: #FFFFFF;");
    }
}

bool UBDocumentTreeItemDelegate::validateString(const QString &str) const
{
    return QRegExp("[^\\/\\:\\?\\*\\|\\<\\>\\\"]{1,}").exactMatch(str)
            && !mExistingFileNames.contains(str);

}

QWidget *UBDocumentTreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    mExistingFileNames.clear();
    const UBDocumentTreeModel *indexModel = qobject_cast<const UBDocumentTreeModel*>(index.model());
    if (indexModel) {
        mExistingFileNames = indexModel->nodeNameList(index.parent());
        mExistingFileNames.removeOne(index.data().toString());
    }

    QLineEdit *nameEditor = new QLineEdit(parent);
    connect(nameEditor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
    connect(nameEditor, SIGNAL(textChanged(QString)), this, SLOT(processChangedText(QString)));
    return nameEditor;
}

void UBDocumentTreeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 0) {
        QLineEdit *lineEditor = qobject_cast<QLineEdit*>(editor);
        lineEditor->setText(index.data().toString());
        lineEditor->selectAll();
    }
}

void UBDocumentTreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit*>(editor);
    if (validateString(lineEditor->text())) {
        model->setData(index, lineEditor->text());
    }
}

void UBDocumentTreeItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
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
   , mCurrentIndexMoved(false)
{

    setupViews();
    setupToolbar();
//    this->selectDocument(UBApplication::boardController->selectedDocument());
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
    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *docModel = pManager->mDocumentTreeStructureModel;
    QModelIndex selectedIndex = firstSelectedTreeIndex();
    if (selectedIndex.isValid()) {
        QString groupName = docModel->isCatalog(selectedIndex)
                ? docModel->virtualPathForIndex(selectedIndex)
                : docModel->virtualDirForIndex(selectedIndex);


        /* disabled model sorting. We cannot use it because of lot os troubles with internal Qt implementation related QPerisstentModelIndex which becomes invalid on index move.
        UBDocumentProxy *document = new UBDocumentProxy(*pManager->createDocument(groupName)); // work around with memory leak.
        mBoardController->setActiveDocumentScene(document);
        UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->sort(0);
        */

        UBDocumentProxy *document = pManager->createDocument(groupName);
        selectDocument(document);

        if (document)
            UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->markDocumentAsNew(document);
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
        mDocumentUI->documentTreeView->setSelectedAndExpanded(indexCurrentDoc, true);
    }

    mSelectionType = Document;
    setDocument(proxy);
}

void UBDocumentController::createNewDocumentGroup()
{
    UBPersistenceManager *pManager = UBPersistenceManager::persistenceManager();
    UBDocumentTreeModel *docModel = pManager->mDocumentTreeStructureModel;
    QModelIndex selectedIndex = firstSelectedTreeIndex();
    if (!selectedIndex.isValid()) {
        return;
    }
    QModelIndex parentIndex = docModel->isCatalog(selectedIndex)
            ? selectedIndex
            : selectedIndex.parent();

    QString newFolderName = docModel->adjustNameForParentIndex(tr("New Folder"), parentIndex);

    docModel->addCatalog(newFolderName, parentIndex);
/*  disabled model sorting. We cannot use it because of lot os troubles with internal Qt implementation related QPerisstentModelIndex which becomes invalid on index move.
    UBDocumentProxy *document = new UBDocumentProxy(*selectedDocument()); // work around with memory leak.
    mBoardController->setActiveDocumentScene(document);
    UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->sort(0);
    selectDocument(document);
*/
}


UBDocumentProxy* UBDocumentController::selectedDocumentProxy()
{
    return UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->proxyForIndex(firstSelectedTreeIndex());
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

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    UBDocumentProxy *currentDocumentProxy = docModel->proxyData(current);
    setDocument(currentDocumentProxy, false);

    if (mCurrentIndexMoved) {
        if (docModel->isDocument(current)) {
            docModel->setCurrentDocument(currentDocumentProxy);
        } else if (docModel->isCatalog(current)) {
            docModel->setCurrentDocument(0);
        }
        mCurrentIndexMoved = false;
    }

    itemSelectionChanged(docModel->isCatalog(current) ? Folder : Document);
}

void UBDocumentController::TreeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    if (selected.indexes().count()) {
        QModelIndex NewSelectedRow = selected.indexes().first();
        TreeViewSelectionChanged(NewSelectedRow, QModelIndex());
    }

}

void UBDocumentController::itemSelectionChanged(LastSelectedElementType newSelection)
{
    mSelectionType = newSelection;
    updateActions();
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
            adaptor->setAssociatedAction(currentExportAction);
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
        mDocumentUI->documentTreeView->setItemDelegate(new UBDocumentTreeItemDelegate(this));
//        mDocumentUI->documentTreeView->setDragDropMode(QAbstractItemView::InternalMove);
        mDocumentUI->documentTreeView->setDragEnabled(true);
        mDocumentUI->documentTreeView->setAcceptDrops(true);
        mDocumentUI->documentTreeView->viewport()->setAcceptDrops(true);
        mDocumentUI->documentTreeView->setDropIndicatorShown(true);
        mDocumentUI->documentTreeView->header()->setStretchLastSection(false);
        mDocumentUI->documentTreeView->header()->setResizeMode(0, QHeaderView::Interactive);

        mDocumentUI->documentTreeView->resizeColumnToContents(0);

        int headerSizeHint = mDocumentUI->documentTreeView->width();

        if (mDocumentUI->documentTreeView->verticalScrollBar()->isVisible())
            headerSizeHint = mDocumentUI->documentTreeView->width() - mDocumentUI->documentTreeView->verticalScrollBar()->width();

        if (mDocumentUI->documentTreeView->columnWidth(0) < headerSizeHint)
            mDocumentUI->documentTreeView->setColumnWidth(0, headerSizeHint);

        connect(mDocumentUI->documentTreeView->itemDelegate(), SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint) ), mDocumentUI->documentTreeView, SLOT(adjustSize()));
//        connect(mDocumentUI->documentTreeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(TreeViewSelectionChanged(QModelIndex,QModelIndex)));
        connect(mDocumentUI->documentTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(TreeViewSelectionChanged(QItemSelection,QItemSelection)));
        connect(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel, SIGNAL(indexChanged(QModelIndex,QModelIndex))
                ,mDocumentUI->documentTreeView, SLOT(onModelIndexChanged(QModelIndex,QModelIndex)));
        connect(UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel, SIGNAL(currentIndexMoved(QModelIndex,QModelIndex))
                ,this, SLOT(currentIndexMoved(QModelIndex,QModelIndex)));

        connect(mDocumentUI->thumbnailWidget, SIGNAL(sceneDropped(UBDocumentProxy*, int, int)), this, SLOT(moveSceneToIndex ( UBDocumentProxy*, int, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(resized()), this, SLOT(thumbnailViewResized()));
//        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseDoubleClick(QGraphicsItem*, int)), this, SLOT(pageDoubleClicked(QGraphicsItem*, int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseDoubleClick(QGraphicsItem*,int)), this, SLOT(thumbnailPageDoubleClicked(QGraphicsItem*,int)));
        connect(mDocumentUI->thumbnailWidget, SIGNAL(mouseClick(QGraphicsItem*, int)), this, SLOT(pageClicked(QGraphicsItem*, int)));

        connect(mDocumentUI->thumbnailWidget->scene(), SIGNAL(selectionChanged()), this, SLOT(pageSelectionChanged()));

//        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentCreated(UBDocumentProxy*)), this, SLOT(addDocumentInTree(UBDocumentProxy*)));
//        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentMetadataChanged(UBDocumentProxy*)), this, SLOT(updateDocumentInTree(UBDocumentProxy*)));

        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneCreated(UBDocumentProxy*, int)), this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));
        connect(UBPersistenceManager::persistenceManager(), SIGNAL(documentSceneWillBeDeleted(UBDocumentProxy*, int)), this, SLOT(documentSceneChanged(UBDocumentProxy*, int)));

        mDocumentUI->thumbnailWidget->setBackgroundBrush(UBSettings::documentViewLightColor);

/*disabled model sorting. We cannot use it because of lot os troubles with internal Qt implementation related QPerisstentModelIndex which becomes invalid on index move.
        selectDocument(new UBDocumentProxy (*mBoardController->selectedDocument())); // work around with memory leak
        UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel->sort(0);
*/
        #ifdef Q_WS_MACX
            mMessageWindow = new UBMessageWindow(NULL);
        #else
            mMessageWindow = new UBMessageWindow(mDocumentUI->thumbnailWidget);
        #endif

        mMessageWindow->hide();
        mDocumentUI->documentTreeWidget->hide();
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

//    selectionChanged();
    updateActions();

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
                mBoardController->setActiveDocumentScene(proxy, thumb->sceneIndex());
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
        UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
        QModelIndex selectedIndex = firstSelectedTreeIndex();

        Q_ASSERT(!docModel->isConstant(selectedIndex) && !docModel->inTrash(selectedIndex));

        //        UBDocumentProxy* source = selectedDocumentProxy();
        //        UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();

//        QString docName = docModel->proxyData(selectedIndex)->metaData(UBSettings::documentName).toString();

        showMessage(tr("Duplicating Document %1").arg(""), true);

        docModel->copyIndexToNewParent(selectedIndex, selectedIndex.parent(), UBDocumentTreeModel::aContentCopy);

        //            UBDocumentProxy* duplicatedDoc = UBPersistenceManager::persistenceManager()->duplicateDocument(source);
        //            duplicatedDoc->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        //            UBMetadataDcSubsetAdaptor::persist(duplicatedDoc);

        //            selectDocument(duplicatedDoc, false);

        showMessage(tr("Document %1 copied").arg(""), false);
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
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    QModelIndex currentIndex = firstSelectedTreeIndex();
    DeletionType deletionForSelection = deletionTypeForSelection(mSelectionType, currentIndex, docModel);

    switch (static_cast<int>(deletionForSelection)) {
    case DeletePage :
        deletePages(mDocumentUI->thumbnailWidget->selectedItems());
        break;

    case MoveToTrash :
        docModel->moveIndex(currentIndex, docModel->trashIndex());
        break;

    case CompleteDelete :
        deleteIndexAndAssociatedData(currentIndex);
        break;

    case EmptyFolder :
        if (currentIndex == docModel->myDocumentsIndex()) { //Emptying "My documents". Keeping Untitled Documents
            int startInd = 0;
            while (docModel->rowCount(currentIndex)) {
                QModelIndex testSubINdecurrentIndex = docModel->index(startInd, 0, currentIndex);
                if (testSubINdecurrentIndex == docModel->untitledDocumentsIndex()) {
                    emptyFolder(testSubINdecurrentIndex, MoveToTrash);
                    startInd++;
                    continue;
                }
                if (!testSubINdecurrentIndex.isValid()) {
                    break;
                }
                docModel->moveIndex(testSubINdecurrentIndex, docModel->trashIndex());
            }
        } else {
            emptyFolder(currentIndex, MoveToTrash); //Empty constant folder
        }
        break;

    case EmptyTrash :
         emptyFolder(currentIndex, CompleteDelete); // Empty trahs folder
        break;
    }



//    if (mSelectionType == Page)
//    {
//        QList<QGraphicsItem*> selectedItems = mDocumentUI->thumbnailWidget->selectedItems();
//        deletePages(selectedItems);

//    } else {

//        UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
//        QModelIndex currentIndex = firstSelectedTreeIndex();

//        if (docModel->isConstant(currentIndex)) { //Constant folder
//            if (currentIndex == docModel->myDocumentsIndex()) { //Emptying "My documents". Keeping Untitled Documents
//                int startInd = 0;
//                while (docModel->rowCount(currentIndex)) {
//                    QModelIndex testSubINdecurrentIndex = docModel->index(startInd, 0, currentIndex);
//                    if (testSubINdecurrentIndex == docModel->untitledDocumentsIndex()) {
//                        emptyFolder(testSubINdecurrentIndex, MoveToTrash);
//                        startInd++;
//                        continue;
//                    }
//                    if (!testSubINdecurrentIndex.isValid()) {
//                        break;
//                    }
//                    docModel->copyIndexToNewParent(testSubINdecurrentIndex, docModel->trashIndex());
//                    docModel->removeRow(startInd, testSubINdecurrentIndex.parent());
//                }
//            } else {
//                emptyFolder(currentIndex, MoveToTrash); //Empty constant folder
//            }

//        } else if (currentIndex == docModel->trashIndex()) { //Trash folder
//            emptyFolder(currentIndex, CompleteDelete); // Empty trahs folder

//        } else if (!docModel->inTrash(currentIndex)) { //Within trash
//            docModel->copyIndexToNewParent(currentIndex, docModel->trashIndex());
//            docModel->removeRow(currentIndex.row(), currentIndex.parent());

//        } else {
//            deleteIndexAndAssociatedData(currentIndex);
//        }


//        if (docModel->isDocument(currentIndex))
//        {
//            UBDocumentProxy *docProxy = docModel->proxyData(currentIndex);
//            if (!docProxy) {
//                return;
//            }

//            if( UBApplication::mainWindow->yesNoQuestion(tr("Remove Document")
//                                                         , tr("Are you sure you want to remove the document '%1'?")
//                                                         .arg(docProxy->metaData(UBSettings::documentName).toString()))) {
//                if (!docModel->inTrash(currentIndex)) {

////                    moveDocumentToTrash(groupTi, proxyTi);

//                } else {
//                    // We have to physically delete document
//                    proxyTi->parent()->removeChild(proxyTi);
//                    UBPersistenceManager::persistenceManager()->deleteDocument(proxyTi->proxy());

//                    if (mTrashTi->childCount()==0)
//                        selectDocument(NULL);
//                    else
//                        selectDocument(((UBDocumentProxyTreeItem*)mTrashTi->child(0))->proxy());
//                    reloadThumbnails();
//                }
//            }

//        } else if (groupTi) {

//            if (groupTi == mTrashTi) {
//                if(UBApplication::mainWindow->yesNoQuestion(tr("Empty Trash"), tr("Are you sure you want to empty trash?"))) {
//                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
//                    QList<UBDocumentProxyTreeItem*> toBeDeleted;

//                    for (int i = 0; i < groupTi->childCount(); i++) {
//                        UBDocumentProxyTreeItem* proxyTi = dynamic_cast<UBDocumentProxyTreeItem*>(groupTi->child(i));
//                        if (proxyTi && proxyTi->proxy())
//                            toBeDeleted << proxyTi;
//                    }

//                    showMessage(tr("Emptying trash"));

//                    for (int i = 0; i < toBeDeleted.count(); i++) {
//                        UBDocumentProxyTreeItem* proxyTi = toBeDeleted.at(i);

//                        proxyTi->parent()->removeChild(proxyTi);
//                        UBPersistenceManager::persistenceManager()->deleteDocument(proxyTi->proxy());
//                    }

//                    showMessage(tr("Emptied trash"));

//                    QApplication::restoreOverrideCursor();
//                    mMainWindow->actionDelete->setEnabled(false);
//                }

//            } else {
//                if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Folder")
//                                                            , tr("Are you sure you want to remove the folder '%1' and all its content?")
//                                                            .arg(groupTi->groupName()))) {
//                    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
//                    moveFolderToTrash(groupTi);
//                    QApplication::restoreOverrideCursor();
//                }
//            }
//        }
//    }
}

void UBDocumentController::emptyFolder(const QModelIndex &index, DeletionType pDeletionType)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    if (!docModel->isCatalog(index)) {
        return;
    }
    while (docModel->rowCount(index)) {
        QModelIndex subIndex = docModel->index(0, 0, index);
        switch (static_cast<int>(pDeletionType)) {
        case MoveToTrash :
            docModel->moveIndex(subIndex, docModel->trashIndex());
            break;

        case CompleteDelete :
            deleteIndexAndAssociatedData(subIndex);
            break;
        }
    }
}

void UBDocumentController::deleteIndexAndAssociatedData(const QModelIndex &pIndex)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    while (docModel->rowCount(pIndex)) {
        QModelIndex subIndex = docModel->index(0, 0, pIndex);
        deleteIndexAndAssociatedData(subIndex);
    }

    if (docModel->isDocument(pIndex)) {
        UBDocumentProxy *proxyData = docModel->proxyData(pIndex);
        if (proxyData) {
            UBPersistenceManager::persistenceManager()->deleteDocument(proxyData);
        }
    }
    docModel->removeRow(pIndex.row(), pIndex.parent());
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
//    UBDocumentGroupTreeItem* group = selectedDocumentGroupTreeItem();
    UBDocumentManager *docManager = UBDocumentManager::documentManager();

    QString defaultPath = UBSettings::settings()->lastImportFilePath->get().toString();
    QString filePath = QFileDialog::getOpenFileName(mParentWidget, tr("Open Supported File"),
                                                    defaultPath, docManager->importFileFilter());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();
    QFileInfo fileInfo(filePath);

    if (fileInfo.suffix().toLower() == "ubx") {
        UBPersistenceManager::persistenceManager()->createDocumentProxiesStructure(docManager->importUbx(filePath, UBSettings::userDocumentDirectory()), true);

    } else {
        UBSettings::settings()->lastImportFilePath->set(QVariant(fileInfo.absolutePath()));

        if (filePath.length() > 0)
        {
            UBDocumentProxy* createdDocument = 0;
            QApplication::processEvents();
            QFile selectedFile(filePath);

            QString groupName = UBPersistenceManager::myDocumentsName;

//            if (groupName == mDefaultDocumentGroupName || fileInfo.suffix() != "ubz")
                groupName = "";

            showMessage(tr("Importing file %1...").arg(fileInfo.baseName()), true);

            createdDocument = docManager->importFile(selectedFile, groupName);


            if (createdDocument) {
                selectDocument(createdDocument, true);

            } else {
                showMessage(tr("Failed to import file ... "));
            }
        }
    }

    QApplication::restoreOverrideCursor();

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
    if (mIsClosing)
        return;

    bool pageSelected = mDocumentUI->thumbnailWidget->selectedItems().count() > 0;

    if (pageSelected)
        itemSelectionChanged(Page);
    else
        itemSelectionChanged(None);

    updateActions();
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


    bool isAModel = dynamic_cast<UBDocumentGroupTreeItem*>(proxyTi->parent())->groupName() == UBPersistenceManager::modelsName;
    mMainWindow->actionOpen->setEnabled((docSelected || pageSelected) && !trashSelected && !isAModel);
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
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    QModelIndex selectedIndex = firstSelectedTreeIndex();

    if (selectedIndex.isValid()) {
        if (docModel->inModel(selectedIndex)) {
            UBApplication::showMessage(tr("The model documents are not editable. Copy it to \"My documents\" to be able to work with"));
            return;
        } else if (docModel->inTrash(selectedIndex)) {
            return;
        }
    }

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

//    selectionChanged();

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

    UBPersistenceManager::persistenceManager()->closing();
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
            mBoardController->addScene(pageInfoList.at(i).first, pageInfoList.at(i).second, false);
        }

        int newActiveSceneIndex = selectedItems.count() == mBoardController->selectedDocument()->pageCount() ? 0 : oldActiveSceneIndex + 1;
        mDocumentUI->thumbnailWidget->selectItemAt(newActiveSceneIndex, false);
        selectDocument(mBoardController->selectedDocument());
        mBoardController->selectedDocument()->setMetaData(UBSettings::documentUpdatedAt, UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime()));
        UBMetadataDcSubsetAdaptor::persist(mBoardController->selectedDocument());
        mBoardController->reloadThumbnails();

        UBApplication::boardController->documentNavigator()->generateThumbnails(this);
        UBApplication::applicationController->showBoard();

        mBoardController->setActiveDocumentScene(newActiveSceneIndex);
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
    QModelIndex selectedIndex = firstSelectedTreeIndex();
    if (selectedIndex.isValid()) {
        mDocumentUI->documentTreeView->edit(selectedIndex);
    }
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
    UBDocumentTreeModel *treeModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    if (current == mDocumentUI->thumbnailWidget)
    {
        if (mDocumentUI->thumbnailWidget->selectedItems().count() > 0)
            mSelectionType = Page;
        else
            mSelectionType = None;
    }
    else if (current == mDocumentUI->documentTreeView)
    {
        if (treeModel->isDocument(firstSelectedTreeIndex()))
            mSelectionType = Document;
        else if (treeModel->isCatalog(firstSelectedTreeIndex()))
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
            old != mDocumentUI->documentTreeView &&
            old != mDocumentUI->documentZoomSlider)
        {
            mSelectionType = None;
        }
    }

//    selectionChanged();
//    updateActions();
}

void UBDocumentController::updateActions()
{
    if (mIsClosing)
        return;

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;

    QModelIndex selectedIndex = firstSelectedTreeIndex();
    UBDocumentProxy *selectedProxy = docModel->proxyData(selectedIndex);
    int pageCount = -1;
    if (selectedProxy) {
        pageCount = selectedProxy->pageCount();
    }

    bool pageSelected = false;
    bool groupSelected = false;
    bool docSelected = false;

    if (mSelectionType == Page) {
        pageSelected = true;
    } else {
        if (docModel->isDocument(firstSelectedTreeIndex())) {
            docSelected = true;
        } else if (docModel->isCatalog(firstSelectedTreeIndex())) {
            groupSelected = true;
        }
    }

    bool trashSelected = docModel->inTrash(selectedIndex) || selectedIndex == docModel->trashIndex()  ? true : false;
    bool modelSelected = docModel->inModel(selectedIndex) || selectedIndex == docModel->modelsIndex() ? true : false;

    mMainWindow->actionNewDocument->setEnabled(docModel->newNodeAllowed(selectedIndex) && !modelSelected);
    mMainWindow->actionNewFolder->setEnabled(docModel->newNodeAllowed(selectedIndex));
    mMainWindow->actionExport->setEnabled((docSelected || pageSelected || groupSelected) && !trashSelected);
    updateExportSubActions(selectedIndex);

    bool firstSceneSelected = false;

    if (docSelected) {
        mMainWindow->actionDuplicate->setEnabled(!trashSelected && !modelSelected);

    } else if (pageSelected) {
        QList<QGraphicsItem*> selection = mDocumentUI->thumbnailWidget->selectedItems();
        if(pageCount == 1) {
            mMainWindow->actionDuplicate->setEnabled(!trashSelected && pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(0)));

        } else {
            for (int i = 0; i < selection.count() && !firstSceneSelected; i += 1) {
                if (qgraphicsitem_cast<UBSceneThumbnailPixmap*>(selection.at(i))->sceneIndex() == 0) {
                    mMainWindow->actionDuplicate->setEnabled(!trashSelected && pageCanBeDuplicated(UBDocumentContainer::pageFromSceneIndex(0)));
                    firstSceneSelected = true;
                    break;
                }
            }
            if (!firstSceneSelected) {
                mMainWindow->actionDuplicate->setEnabled(!trashSelected);
            }
        }

    } else {
        mMainWindow->actionDuplicate->setEnabled(false);
    }

    mMainWindow->actionOpen->setEnabled((docSelected || pageSelected) && !trashSelected && !modelSelected);
    mMainWindow->actionRename->setEnabled(docModel->isOkToRename(selectedIndex));

    mMainWindow->actionAddToWorkingDocument->setEnabled(pageSelected
            && !(selectedProxy == mBoardController->selectedDocument()) && !trashSelected);

    DeletionType deletionForSelection = deletionTypeForSelection(mSelectionType, selectedIndex, docModel);
    mMainWindow->actionDelete->setEnabled(deletionForSelection != NoDeletion);

    switch (static_cast<int>(deletionForSelection)) {
    case MoveToTrash :
    case DeletePage :
        mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
        mMainWindow->actionDelete->setText(tr("Trash"));
        break;
    case CompleteDelete :
        mMainWindow->actionDelete->setIcon(QIcon(":/images/toolbar/deleteDocument.png"));
        mMainWindow->actionDelete->setText(tr("Delete"));
        break;
    case EmptyFolder :
        mMainWindow->actionDelete->setIcon(QIcon(":/images/trash.png"));
        mMainWindow->actionDelete->setText(tr("Empty"));
        break;
    case EmptyTrash :
        mMainWindow->actionDelete->setIcon(QIcon(":/images/toolbar/deleteDocument.png"));
        mMainWindow->actionDelete->setText(tr("Empty"));
        break;
    }

    mMainWindow->actionDocumentAdd->setEnabled((docSelected || pageSelected) && !trashSelected && !modelSelected);
    mMainWindow->actionImport->setEnabled(!trashSelected);

}

void UBDocumentController::updateExportSubActions(const QModelIndex &selectedIndex)
{
    UBDocumentManager *documentManager = UBDocumentManager::documentManager();
    for (int i = 0; i < documentManager->supportedExportAdaptors().length(); i++)
    {
        UBExportAdaptor* adaptor = documentManager->supportedExportAdaptors()[i];
        if (adaptor->associatedAction()) {
            adaptor->associatedAction()->setEnabled(adaptor->associatedActionactionAvailableFor(selectedIndex));
        }
    }
}

void UBDocumentController::currentIndexMoved(const QModelIndex &newIndex, const QModelIndex &PreviousIndex)
{
    Q_UNUSED(newIndex);
    Q_UNUSED(PreviousIndex);

    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    UBDocumentProxy *newProxy = docModel->proxyData(newIndex);
    if (newProxy) {
        UBDocumentProxy *cp = new UBDocumentProxy(*newProxy); // we cannot use newProxy because it will be destroyed later
        pureSetDocument(cp);
        mBoardController->pureSetDocument(cp);
        mBoardController->pureSetDocument(newProxy);
    }
    mCurrentIndexMoved = true;
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

        if(UBApplication::mainWindow->yesNoQuestion(tr("Remove Page"),tr("This is an irreversible action!") +"\n\n" + tr("Are you sure you want to remove %n page(s) from the selected document '%1'?", "", sceneIndexes.count()).arg(proxy->metaData(UBSettings::documentName).toString())))
        {
//            UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
//            if (docModel->proxyData(docModel->currentIndex()) == proxy) {

//                int offset = 0;
//                foreach(int index, sceneIndexes)
//                {
////                    if (UBPersistenceManager::persistenceManager()->isSceneInCached(proxy, index)) {
//                        mBoardController->regenerateThumbnails();
////                    }
////                    mBoardController->deletePages(sceneIndexes);

//                    deleteThumbPage(index - offset);
//                    offset++;
//                }

//                emit UBDocumentContainer::documentThumbnailsUpdated(this);

//            } else {
                UBDocumentContainer::deletePages(sceneIndexes);
                mBoardController->regenerateThumbnails();

//            }


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

QModelIndex UBDocumentController::firstSelectedTreeIndex()
{
    return selectedTreeIndexes().count() ? selectedTreeIndexes().first() : QModelIndex();
}

UBDocumentController::DeletionType
UBDocumentController::deletionTypeForSelection(LastSelectedElementType pTypeSelection
                                               , const QModelIndex &selectedIndex
                                               , UBDocumentTreeModel *docModel) const
{
    if (pTypeSelection == Page) {
        if (!firstSceneSelected()) {
            return DeletePage;
        }
    } else if (docModel->isConstant(selectedIndex)) {
        if (selectedIndex == docModel->trashIndex()) {
            return EmptyTrash;
        }
        return EmptyFolder;
    } else if (pTypeSelection != None) {
        if (docModel->inTrash(selectedIndex)) {
            return CompleteDelete;
        } else {
            return MoveToTrash;
        }
    }

    return NoDeletion;
}

bool UBDocumentController::firstSceneSelected() const
{
    bool firstSceneSelected = false;
    QList<QGraphicsItem*> selection = mDocumentUI->thumbnailWidget->selectedItems();
    for(int i = 0; i < selection.count() && !firstSceneSelected; i += 1){
        if(dynamic_cast<UBSceneThumbnailPixmap*>(selection.at(i))->sceneIndex() == 0){
            firstSceneSelected = true;
        }
    }

    return firstSceneSelected;
}

void UBDocumentController::refreshDocumentThumbnailsView(UBDocumentContainer*)
{
    UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
    UBDocumentProxy *currentDocumentProxy = selectedDocument();

    QModelIndex current = docModel->indexForProxy(currentDocumentProxy);

    if (!current.isValid()) {
        mDocumentUI->thumbnailWidget->setGraphicsItems(QList<QGraphicsItem*>()
                                                       , QList<QUrl>()
                                                       , QStringList()
                                                       , UBApplication::mimeTypeUniboardPage);
        return;
    }

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

    if (docModel->inTrash(current) || docModel->inModel(current)) {
        mDocumentUI->thumbnailWidget->setDragEnabled(false);
    } else {
        mDocumentUI->thumbnailWidget->setDragEnabled(true);
    }

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
