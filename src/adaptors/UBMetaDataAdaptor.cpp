#include <QString>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>

#include "UBMetaDataAdaptor.h"

const QString metadataFilename = "metadata2.rdf";
const QString nsRdf = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";

UBMetaDataAdaptor::UBMetaDataAdaptor(){

}

UBMetaDataAdaptor::~UBMetaDataAdaptor(){

}

void UBMetaDataAdaptor::persist(const QString& path, QList<sNamespace> ns, QList<sMetaData> md, const QString& desc){
    if(!QDir(path).exists()){
        //In this case the a document is an empty document so we do not persist it
        return;
    }
    QString fileName = path + "/" + metadataFilename;
    qWarning() << fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "cannot open " << fileName << " for writing ...";
        qCritical() << "error : "  << file.errorString();
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    xmlWriter.writeDefaultNamespace(nsRdf);

    // Write the namespaces
    QList<sNamespace>::const_iterator it;
    for(it = ns.begin(); it != ns.end(); it++)
        xmlWriter.writeNamespace(it->url, it->name);

    xmlWriter.writeStartElement("RDF");

    // Write the description
    xmlWriter.writeStartElement("Description");
    xmlWriter.writeAttribute("about", desc);

    // Write the metadata
    QList<sMetaData>::const_iterator mIt;
    for(mIt = md.begin(); mIt != md.end(); mIt++){
        xmlWriter.writeTextElement(QString("%0:%1").arg(mIt->ns).arg(mIt->key), mIt->value);
    }

    xmlWriter.writeEndElement(); //dc:Description
    xmlWriter.writeEndElement(); //RDF

    xmlWriter.writeEndDocument();

    file.flush();
    file.close();
}

void UBMetaDataAdaptor::load(){

}
