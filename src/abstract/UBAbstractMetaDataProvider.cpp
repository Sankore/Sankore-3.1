#include "UBAbstractMetaDataProvider.h"

/**
 * \brief Constructor
 */
UBAbstractMetaDataProvider::UBAbstractMetaDataProvider(){

}

/**
 * \brief Destructor
 */
UBAbstractMetaDataProvider::~UBAbstractMetaDataProvider(){

}

/**
 * \brief Add the given metadata to the metadata list
 * @param key as the metadata key
 * @param value as the metadata value
 */
void UBAbstractMetaDataProvider::addMetaData(QString ns, QString key, QString value){
    Q_ASSERT(!key.isEmpty());
    if(!key.isEmpty()){
        sMetaData metaData;
        metaData.ns = ns;
        metaData.key = key;
        metaData.value = value;
        mMetaDatas.append(metaData);
    }
}

/**
 * \brief Update the given metadata
 * @param key as the metadata key
 * @param value as the metadata value
 */
void UBAbstractMetaDataProvider::updateMetaData(QString key, QString value){
    Q_ASSERT(!key.isEmpty());
    if(!key.isEmpty()){
        for(int i=0; i<mMetaDatas.size(); i++){
            if(mMetaDatas.at(i).key == key){
                sMetaData meta;
                meta.ns = mMetaDatas.at(i).ns;
                meta.key = key;
                meta.value = value;
                mMetaDatas.replace(i, meta);
                break;
            }
        }
    }
}
