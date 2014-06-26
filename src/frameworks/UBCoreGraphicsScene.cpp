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



#include "UBCoreGraphicsScene.h"

#include "domain/UBGraphicsMediaItem.h"
#include "domain/UBGraphicsProxyWidget.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsGroupContainerItem.h"

#include "core/memcheck.h"

UBCoreGraphicsScene::UBCoreGraphicsScene(QObject * parent)
    : QGraphicsScene ( parent  )
    , mIsModified(true)
{
    //NOOP
}

UBCoreGraphicsScene::~UBCoreGraphicsScene()
{
    //we must delete removed items that are no more in any scene
    //at groups deleting some items can be added to mItemsToDelete, so we need to use iterators.    

    foreach(QGraphicsItem* item, mItemsToDelete){
        if (item && item->type() != UBGraphicsItemType::PolygonItemType && item->type() != QGraphicsItem::UserType && item->type() != UBGraphicsItemType::groupContainerType && (item->scene() == NULL || item->scene() == this))
            delete item;
    }
    mItemsToDelete.clear();
}

void UBCoreGraphicsScene::addItem(QGraphicsItem* item)
{
    addItemToDeletion(item);

    if (item->type() == UBGraphicsGroupContainerItem::Type && item->childItems().count()) {
        foreach (QGraphicsItem *curItem, item->childItems()) {
            removeItemFromDeletion(curItem);
        }
    }

    if (item->scene() != this)
        QGraphicsScene::addItem(item);

    setModified(true);
}


void UBCoreGraphicsScene::removeItem(QGraphicsItem* item, bool forceDelete)
{
    QGraphicsScene::removeItem(item);
    if (forceDelete)
    {
//        qDebug() << "force delete is " << forceDelete;
        deleteItem(item);
    }
    setModified(true);
}

bool UBCoreGraphicsScene::deleteItem(QGraphicsItem* item)
{
    if(mItemsToDelete.contains(item))
    {

        UBGraphicsItem *item_casted = dynamic_cast<UBGraphicsItem *>(item);
        if (item_casted != NULL)
            item_casted->clearSource();

        mItemsToDelete.remove(item);

        //Issue NC - CFA - 20140417 : [QT-BUG18021] -> https://bugreports.qt-project.org/browse/QTBUG-18021
        //it seems that in some cases, remove item from scene then delete item (which also tries to remove item from scene...)
        //makes application crash. The entire code to clear undoStack was commented for that reason.
        //Now, we have to clear sources to make documents lighter, even if we're not able for the moment
        //to prevent these memory leaks (only for UBGraphicsW3CWidgetItems ?)

        //delete item
        //item = NULL;
        return true;
    }
    else
        return false;
}

void UBCoreGraphicsScene::removeItemFromDeletion(QGraphicsItem *item)
{
    if(NULL != item){
        mItemsToDelete.remove(item);
    }
}

void UBCoreGraphicsScene::addItemToDeletion(QGraphicsItem *item)
{
    if (item) {
        mItemsToDelete.insert(item);
    }
}
