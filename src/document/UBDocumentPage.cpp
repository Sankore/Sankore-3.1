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
#include "UBDocumentPage.h"

/**
 * \brief Constructor
 */
UBDocumentPage::UBDocumentPage(){

}

/**
 * \brief Destructor
 */
UBDocumentPage::~UBDocumentPage(){

}

/**
 * \brief Save the document
 */
void UBDocumentPage::save(){
    QVector<ISavable*>::const_iterator itSavable;

    for(itSavable = mlSavables.constBegin(); itSavable != mlSavables.constEnd(); itSavable++)
        (*itSavable)->save();
}

/**
 * \brief Registers the given savable for further saving
 * @param savable as the savable to register
 */
void UBDocumentPage::registerSavable(ISavable* savable){
    Q_ASSERT(NULL != savable);
    if(NULL != savable && !mlSavables.contains(savable)){
        mlSavables.append(savable);
    }
}

/**
 * \brief Unregisters the given savable for further saving
 * @param savable as the savable to unregister
 */
void UBDocumentPage::unRegisterSavable(ISavable* savable){
    Q_ASSERT(NULL != savable);
    if(NULL != savable && mlSavables.contains(savable)){
        mlSavables.remove(mlSavables.indexOf(savable));
    }
}

/**
 * \brief Set the page number
 * @param number as the given number
 */
void UBDocumentPage::setPageNumber(int number){
    Q_ASSERT(0 <= number);
    if(0 <= number){
        mPageNumber = number;
    }
}

int UBDocumentPage::pageNumber(){
    return mPageNumber;
}
