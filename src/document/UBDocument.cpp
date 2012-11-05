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
#include "UBDocument.h"
#include "frameworks/UBStringUtils.h"
#include "adaptors/UBMetaDataAdaptor.h"

const QString qsNamespace = "dc";
const QString qsNameSpaceUrl = "http://purl.org/dc/elements/1.1/";

// Extra namespace especially done for this software
const QString qsUbNamespace = "ub";
const QString qsUbNamespaceUrl = "http://uniboard.mnemis.com/document";
// --

const QString qsMetaTitle = "title";
const QString qsMetaType = "type";
const QString qsMetaDate = "date";
const QString qsMetaFormat = "format";
const QString qsMetaIdentifier = "identifier";
const QString qsMetaUbVersion = "version";
const QString qsMetaUbSize = "size";
const QString qsMetaUbUpdate = "updated-at";

/**
 * \brief Constructor
 */
UBDocument::UBDocument(){
    init();
    registerMetaDataProvider(this);
}

/**
 * \brief Destructor
 */
UBDocument::~UBDocument(){
    QVector<IMetaDataProvider*>::const_iterator itMetaData;
    for(itMetaData = mlMetaDataProviders.constBegin(); itMetaData != mlMetaDataProviders.constEnd(); itMetaData++)
        unRegisterMetaDataProvider(*itMetaData);
}

/**
 * \brief Save the document information
 */
void UBDocument::save(QList<sNamespace> &ns, QList<sMetaData> &md){
    QVector<sMetaData>::const_iterator itMetaData;
    for(itMetaData = mMetaDatas.constBegin(); itMetaData != mMetaDatas.constEnd(); itMetaData++)
        md.append(*itMetaData);

    sNamespace docNs;
    docNs.name = qsNamespace;
    docNs.url = qsNameSpaceUrl;

    sNamespace ubNs;
    ubNs.name = qsUbNamespace;
    ubNs.url = qsUbNamespaceUrl;

    ns.append(docNs);
    ns.append(ubNs);
}

/**
 * \brief Get the namespace
 * @return the namespace
 */
QString UBDocument::nameSpace(){
    return qsNamespace;
}

/**
 * \brief Get the namespace url
 * @return the namespace url
 */
QString UBDocument::nameSpaceUrl(){
    return qsNameSpaceUrl;
}

/**
 * \brief Registers the given metadata provider for further saving
 * @param provider as the metadata provider to register
 */
void UBDocument::registerMetaDataProvider(IMetaDataProvider* provider){
    Q_ASSERT(NULL != provider);
    if(NULL != provider && !mlMetaDataProviders.contains(provider)){
        mlMetaDataProviders.append(provider);
    }
}

/**
 * \brief Unregisters the given metadata provider for further saving
 * @param provider as the metadata provider to unregister
 */
void UBDocument::unRegisterMetaDataProvider(IMetaDataProvider* provider){
    Q_ASSERT(NULL != provider);
    if(NULL != provider && mlMetaDataProviders.contains(provider)){
        mlMetaDataProviders.remove(mlMetaDataProviders.indexOf(provider));
    }
}

/**
 * \brief Append the given page to the page list
 * @param page as the given page
 */
void UBDocument::appendPage(UBDocumentPage *&page){
    Q_ASSERT(NULL != page);
    if(NULL != page && !mlPages.contains(page)){
        mlPages.append(page);
    }
}

/**
 * \brief Insert the given page to the given position
 * @param page as the given page
 * @param index as the given position
 */
void UBDocument::addPageAt(UBDocumentPage *&page, int index){
    Q_ASSERT(NULL != page);
    Q_ASSERT(0 <= index);
    if(NULL != page && !mlPages.contains(page) && 0 <= index){
        mlPages.insert(index, page);
    }
}

/**
 * \brief Remove the given page
 * @param page as the given page
 */
void UBDocument::removePage(UBDocumentPage *&page){
    Q_ASSERT(NULL != page);
    if(NULL != page && mlPages.contains(page)){
        mlPages.remove(mlPages.indexOf(page));
    }
}

/**
 * \brief Save the document
 */
void UBDocument::persist(const QString& path){
    //  -------------------------
    //  -- Save the meta datas
    //  -------------------------
    QVector<IMetaDataProvider*>::const_iterator itMetaData;

    // Here we build the metadata structure
    QList<sNamespace> qlNs;
    QList<sMetaData> qlMd;
    for(itMetaData = mlMetaDataProviders.constBegin(); itMetaData != mlMetaDataProviders.constEnd(); itMetaData++)
        (*itMetaData)->save(qlNs, qlMd);

    // Then we persist the datas in a file
    UBMetaDataAdaptor* adaptor = new UBMetaDataAdaptor();
    adaptor->persist(path, qlNs, qlMd, mIdentifier.toAscii().constData());
}

/**
 * \brief Init the document
 */
void UBDocument::init(){
    addMetaData(nameSpace(), qsMetaTitle);
    addMetaData(nameSpace(), qsMetaType);
    addMetaData(nameSpace(), qsMetaDate);
    addMetaData(nameSpace(), qsMetaFormat);
    addMetaData(nameSpace(), qsMetaIdentifier);
    addMetaData(qsUbNamespace, qsMetaUbVersion);
    addMetaData(qsUbNamespace, qsMetaUbSize);
    addMetaData(qsUbNamespace, qsMetaUbUpdate);
}

/**
 * \brief Get the document title
 * @return the document title
 */
QString UBDocument::title(){
    return mTitle;
}

/**
 * \brief Set the document title
 * @param title as the document title
 */
void UBDocument::setTitle(const QString& title){
    mTitle = title;
    updateMetaData(QString("%0:%1").arg(nameSpace()).arg(qsMetaTitle), mTitle);
}

/**
 * \brief Get the document type
 * @return the document type
 */
QString UBDocument::type(){
    return mType;
}

/**
 * \brief Set the document type
 * @param type as the document type
 */
void UBDocument::setType(const QString& type){
    mType = type;
    updateMetaData(QString("%0:%1").arg(nameSpace()).arg(qsMetaType), mType);
}

/**
 * \brief Get the document creation date
 * @return the document creation date
 */
QDateTime UBDocument::creationDate(){
    return mDate;
}

/**
 * \brief Set the document date
 * @param date as the document date
 */
void UBDocument::setCreationDate(const QDateTime& date){
    mDate = date;
    updateMetaData(QString("%0:%1").arg(nameSpace()).arg(qsMetaDate), UBStringUtils::toUtcIsoDateTime(mDate));
}

/**
 * \brief Get the document fomat
 * @return the document format
 */
QString UBDocument::format(){
    return mFormat;
}

/**
 * \brief Set the document format
 * @param format as the document format
 */
void UBDocument::setFormat(const QString& format){
    mFormat = format;
    updateMetaData(QString("%0:%1").arg(nameSpace()).arg(qsMetaFormat), mFormat);
}

/**
 * \brief Get the document identifier
 * @return the document identifier
 */
QString UBDocument::id(){
    return mIdentifier;
}

/**
 * \brief Set the document identifier
 * @param id as the document identifier
 */
void UBDocument::setId(const QString& id){
    mIdentifier = id;
    updateMetaData(QString("%0:%1").arg(nameSpace()).arg(qsMetaIdentifier), mIdentifier);
}

/**
 * \brief Get the document version
 * @return the document version
 */
QString UBDocument::version(){
    return mVersion;
}

/**
 * \brief Set the document version
 * @param version as the document version
 */
void UBDocument::setVersion(const QString& version){
    mVersion = version;
    updateMetaData(QString("%0:%1").arg(qsUbNamespace).arg(qsMetaUbVersion), mVersion);
}

/**
 * \brief Get the document update date
 * @return the document update date
 */
QDateTime UBDocument::updatedDate(){
    return mUpdatedDate;
}

/**
 * \brief Set the document update date
 * @param date as the document update date
 */
void UBDocument::setUpdatedDate(const QDateTime& date){
    mUpdatedDate = date;
    updateMetaData(QString("%0:%1").arg(qsUbNamespace).arg(qsMetaUbUpdate), UBStringUtils::toUtcIsoDateTime(mUpdatedDate));
}

/**
 * \brief Get the document size
 * @return the document size
 */
QString UBDocument::size(){
    return mSize;
}

/**
 * \brief Set the document size
 * @param size as the document size
 */
void UBDocument::setSize(const QString& size){
    mSize = size;
    updateMetaData(QString("%0:%1").arg(qsUbNamespace).arg(qsMetaUbSize), mSize);
}

/**
 * \brief Get the document uuid
 * @return the document uuid
 */
QString UBDocument::uuid(){
    return mUuid;
}

/**
 * \brief Set the document uuid
 * @param uuid as the document uuid
 */
void UBDocument::setUuid(const QString &uuid){
    mUuid = uuid;
}
