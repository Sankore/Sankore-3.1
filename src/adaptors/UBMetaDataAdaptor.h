#ifndef UBMETADATAADAPTOR_H
#define UBMETADATAADAPTOR_H

#include <QVector>
#include "interfaces/IMetaDataProvider.h"

class UBMetaDataAdaptor{
public:
    UBMetaDataAdaptor();
    ~UBMetaDataAdaptor();

    void persist(const QString& path, QList<sNamespace> ns, QList<sMetaData> md, const QString& desc);
    void load();

};

#endif // UBMETADATAADAPTOR_H
