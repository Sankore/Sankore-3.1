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
#ifndef UBDOCUMENT_H
#define UBDOCUMENT_H

#include <QVector>
#include <QString>
#include <QDateTime>
#include "UBDocumentPage.h"
#include "abstract/UBAbstractMetaDataProvider.h"

class UBDocument : public UBAbstractMetaDataProvider{
public:
    UBDocument();
    ~UBDocument();

    virtual void save(QList<sNamespace> &ns, QList<sMetaData> &md);
    virtual QString nameSpace();
    virtual QString nameSpaceUrl();

    void registerMetaDataProvider(IMetaDataProvider* provider);
    void unRegisterMetaDataProvider(IMetaDataProvider* provider);
    void appendPage(UBDocumentPage* &page);
    void addPageAt(UBDocumentPage* &page, int index);
    void removePage(UBDocumentPage* &page);
    void persist(const QString& path);
    QString title();
    void setTitle(const QString& title);
    QString type();
    void setType(const QString& type);
    QDateTime creationDate();
    void setCreationDate(const QDateTime& date);
    QString format();
    void setFormat(const QString& format);
    QString id();
    void setId(const QString& id);
    QString version();
    void setVersion(const QString& version);
    QDateTime updatedDate();
    void setUpdatedDate(const QDateTime& date);
    QString size();
    void setSize(const QString& size);
    QString uuid();
    void setUuid(const QString& uuid);

private:
    void init();

    /** Pages */
    QVector<UBDocumentPage*> mlPages;
    /** Registered data providers */
    QVector<IMetaDataProvider*> mlMetaDataProviders;
    /** Page Title*/
    QString mTitle;
    /** Type */
    QString mType;
    /** Date */
    QDateTime mDate;
    /** Format */
    QString mFormat;
    /** Identifier */
    QString mIdentifier;
    /** Document version */
    QString mVersion;
    /** Updated date */
    QDateTime mUpdatedDate;
    /** Size */
    QString mSize;  // Nobody knows what's this...
    /** Uuid */
    QString mUuid;
};

#endif // UBDOCUMENT_H
