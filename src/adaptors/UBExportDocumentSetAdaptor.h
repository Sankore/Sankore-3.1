#ifndef UBEXPORTDOCUMENTSETADAPTOR_H
#define UBEXPORTDOCUMENTSETADAPTOR_H

#include <QtCore>
#include "UBExportAdaptor.h"
#include "frameworks/UBFileSystemUtils.h"
#include "globals/UBGlobals.h"

THIRD_PARTY_WARNINGS_DISABLE
#include "quazip.h"
#include "quazipfile.h"
THIRD_PARTY_WARNINGS_ENABLE

class UBDocumentProxy;
class UBDocumentTreeModel;


class UBExportDocumentSetAdaptor : public UBExportAdaptor
{
    Q_OBJECT

    public:
        UBExportDocumentSetAdaptor(QObject *parent = 0);
        virtual ~UBExportDocumentSetAdaptor();

        virtual QString exportName();
        virtual QString exportExtention();

        virtual void persist(UBDocumentProxy* pDocument);
        bool persistData(const QModelIndex &pRootIndex, QString filename);
        bool addDocumentToZip(const QModelIndex &pIndex, UBDocumentTreeModel *model, QuaZip &zip);
};

#endif // UBEXPORTDOCUMENTSETADAPTOR_H
