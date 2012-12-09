/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef UBDOCUMENTCONTROLLER_H_
#define UBDOCUMENTCONTROLLER_H_

#include <QtGui>
#include "document/UBDocumentContainer.h"

namespace Ui
{
    class documents;
}

#include "gui/UBMessageWindow.h"

class UBGraphicsScene;
class QDialog;
class UBDocumentProxy;
class UBBoardController;
class UBThumbnailsScene;
class UBDocumentGroupTreeItem;
class UBDocumentProxyTreeItem;
class UBMainWindow;
class UBDocumentToolsPalette;


class UBDocumentTreeNode
{
public:
    enum Type {
        Catalog
        , Document
    };

    UBDocumentTreeNode(Type pType, const QString &pName, const QString &pDisplayName = QString(), UBDocumentProxy *pProxy = 0);
    UBDocumentTreeNode() : mType(Catalog), mParent(0), mProxy(0) {;}
    ~UBDocumentTreeNode();

    QList<UBDocumentTreeNode*> children() const {return mChildren;}
    UBDocumentTreeNode *parentNode() {return mParent;}
    Type nodeType() const {return mType;}
    QString nodeName() const {return mName;}
    QString displayName() const {return mDisplayName;}
    void addChild(UBDocumentTreeNode *pChild);
    void removeChild(int index);
    UBDocumentTreeNode *moveTo(const QString &pPath);
    UBDocumentProxy *proxyData() const {return mProxy;}
    bool isRoot() {return !mParent;}
    bool isTopLevel() {return mParent && !mParent->mParent;}


private:
    Type mType;
    QString mName;
    QString mDisplayName;
    UBDocumentTreeNode *mParent;
    QList<UBDocumentTreeNode*> mChildren;
    QPointer<UBDocumentProxy> mProxy;
};
Q_DECLARE_METATYPE(UBDocumentTreeNode*)

class UBDocumentTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    UBDocumentTreeModel(QObject *parent = 0);
    ~UBDocumentTreeModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    Qt::DropActions supportedDropActions() const {return Qt::MoveAction | Qt::CopyAction;}
//    QMimeData *mimeData( const QModelIndexList &indexes ) const;
    QStringList mimeTypes() const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool removeRow(int row, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

    QModelIndex indexForNode(UBDocumentTreeNode *pNode) const;
    QPersistentModelIndex persistentIndexForNode(UBDocumentTreeNode *pNode);
//    bool insertRow(int row, const QModelIndex &parent);
    void addNode(UBDocumentTreeNode *pFreeNode, UBDocumentTreeNode *pParent);
    void addNode(UBDocumentTreeNode *pFreeNode, const QModelIndex &pParent);
    void setCurrentNode(UBDocumentTreeNode *pNode) {mCurrentNode = pNode;}
    QModelIndex indexForProxy(UBDocumentProxy *pSearch) const;
    void setCurrentDocument(UBDocumentProxy *pDocument);
    void setRootNode(UBDocumentTreeNode *pRoot);
    UBDocumentProxy *proxyForIndex(const QModelIndex &pIndex) const;
    QString virtualDirForIndex(const QModelIndex &pIndex) const;
    QStringList nodeNameList(const QModelIndex &pIndex) const;
    bool newFolderAllowed(const QModelIndex &pIndex)  const;

private:
    UBDocumentTreeNode *mRootNode;
    UBDocumentTreeNode *mCurrentNode;
    UBDocumentTreeNode *nodeFromIndex(const QModelIndex &pIndex) const;
    UBDocumentTreeNode *findProxy(UBDocumentProxy *pSearch, UBDocumentTreeNode *pParent) const;
    QModelIndex pIndexForNode(const QModelIndex &parent, UBDocumentTreeNode *pNode) const;
    bool removeChildFromModel(UBDocumentTreeNode *child, UBDocumentTreeModel *parent);
};

class UBDocumentTreeView : public QTreeView
{
    Q_OBJECT

public:
    UBDocumentTreeView (QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
};

class UBDocumentController : public UBDocumentContainer
{
    Q_OBJECT

    public:
        UBDocumentController(UBMainWindow* mainWindow);
        virtual ~UBDocumentController();

        void closing();
        QWidget* controlView();
        UBDocumentProxyTreeItem* findDocument(UBDocumentProxy* proxy);
        bool addFileToDocument(UBDocumentProxy* document);
        void deletePages(QList<QGraphicsItem*> itemsToDelete);
        int getSelectedItemIndex();

        bool pageCanBeMovedUp(int page);
        bool pageCanBeMovedDown(int page);
        bool pageCanBeDuplicated(int page);
        bool pageCanBeDeleted(int page);
        QString documentTrashGroupName(){ return mDocumentTrashGroupName;}
        QString defaultDocumentGroupName(){ return mDefaultDocumentGroupName;}

        void setDocument(UBDocumentProxy *document, bool forceReload = false);

    signals:
        void exportDone();

    public slots:
        void createNewDocument();
        void createNewDocumentGroup();
        void deleteSelectedItem();
        void renameSelectedItem();
        void openSelectedItem();
        void duplicateSelectedItem();
        void importFile();
        void moveSceneToIndex(UBDocumentProxy* proxy, int source, int target);
        void selectDocument(UBDocumentProxy* proxy, bool setAsCurrentDocument = true);
        void setSelectedAndExpanded(const QModelIndex &pIndex, bool pExpand = true);
        void show();
        void hide();
        void showMessage(const QString& message, bool showSpinningWheel = false);
        void hideMessage();
        void toggleDocumentToolsPalette();
        void cut();
        void copy();
        void paste();
        void focusChanged(QWidget *old, QWidget *current);

    protected:
        virtual void setupViews();
        virtual void setupToolbar();
        void setupPalettes();
        bool isOKToOpenDocument(UBDocumentProxy* proxy);
        UBDocumentProxy* selectedDocumentProxy();
        UBDocumentProxy *firstSelectedTreeProxy();
        QList<UBDocumentProxy*> selectedProxies();
        QModelIndexList selectedTreeIndexes();
        QModelIndex firstSelectedTreeIndex() {return selectedTreeIndexes().first();}
        UBDocumentProxyTreeItem* selectedDocumentProxyTreeItem();
        UBDocumentGroupTreeItem* selectedDocumentGroupTreeItem();
        QStringList allGroupNames();

        enum LastSelectedElementType
        {
            None = 0, Folder, Document, Page
        };

        LastSelectedElementType mSelectionType;

    private:
        QWidget *mParentWidget;
        UBBoardController *mBoardController;
        Ui::documents* mDocumentUI;
        UBMainWindow* mMainWindow;
        QWidget *mDocumentWidget;
        QPointer<UBMessageWindow> mMessageWindow;
        QAction* mAddFolderOfImagesAction;
        QAction* mAddFileToDocumentAction;
        QAction* mAddImagesAction;
        bool mIsClosing;
        UBDocumentToolsPalette *mToolsPalette;
        bool mToolsPalettePositionned;
        UBDocumentGroupTreeItem* mTrashTi;

        void moveDocumentToTrash(UBDocumentGroupTreeItem* groupTi, UBDocumentProxyTreeItem *proxyTi);
        void moveFolderToTrash(UBDocumentGroupTreeItem* groupTi);
        QString mDocumentTrashGroupName;
        QString mDefaultDocumentGroupName;

        UBDocumentProxy *mCurrentTreeDocument;

    private slots:
        void documentZoomSliderValueChanged (int value);
        void loadDocumentProxies();
        void TreeViewSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
        void itemSelectionChanged();
        void exportDocument();
        void itemChanged(QTreeWidgetItem * item, int column);
        void thumbnailViewResized();
        void pageSelectionChanged();
        void selectionChanged();
        void documentSceneChanged(UBDocumentProxy* proxy, int pSceneIndex);
        void pageDoubleClicked(QGraphicsItem* item, int index);
        void thumbnailPageDoubleClicked(QGraphicsItem* item, int index);
        void pageClicked(QGraphicsItem* item, int index);
        void itemClicked(QTreeWidgetItem * item, int column );
        void addToDocument();
        void addDocumentInTree(UBDocumentProxy* pDocument);
        void updateDocumentInTree(UBDocumentProxy* pDocument);
        void addFolderOfImages();
        void addFileToDocument();
        void addImages();

        void refreshDocumentThumbnailsView(UBDocumentContainer* source);
};



#endif /* UBDOCUMENTCONTROLLER_H_ */
