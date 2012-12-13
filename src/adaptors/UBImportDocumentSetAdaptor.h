#ifndef UBIMPORTDOCUMENTSETADAPTOR_H
#define UBIMPORTDOCUMENTSETADAPTOR_H

#include <QtGui>
#include "UBImportAdaptor.h"

class UBDocumentProxy;

class UBImportDocumentSetAdaptor : public UBImportAdaptor
{
    Q_OBJECT

    public:
        UBImportDocumentSetAdaptor(QObject *parent = 0);
        virtual ~UBImportDocumentSetAdaptor();


        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

//        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);
//        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

    private:
        bool extractFileToDir(const QFile& pZipFile, const QString& pDir, QString& documentRoot);

};

#endif // UBIMPORTDOCUMENTSETADAPTOR_H
