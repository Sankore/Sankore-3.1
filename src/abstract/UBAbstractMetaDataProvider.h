#ifndef UBABSTRACTMETADATAPROVIDER_H
#define UBABSTRACTMETADATAPROVIDER_H

#include <QString>
#include <QVector>
#include <QList>

#include "interfaces/IMetaDataProvider.h"

class UBAbstractMetaDataProvider : public IMetaDataProvider{
public:
    UBAbstractMetaDataProvider();
    ~UBAbstractMetaDataProvider();
    virtual void save(QList<sNamespace> &ns, QList<sMetaData> &md)=0;
    virtual QString nameSpace()=0;
    virtual QString nameSpaceUrl()=0;

protected:
    void addMetaData(QString ns, QString key, QString value="");
    void updateMetaData(QString key, QString value);

    QVector<sMetaData> mMetaDatas;
};

#endif // UBABSTRACTMETADATAPROVIDER_H
