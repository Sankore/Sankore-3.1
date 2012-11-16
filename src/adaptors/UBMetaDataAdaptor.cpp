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
#include <QString>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>

#include "UBMetaDataAdaptor.h"

const QString metadataFilename = "metadata2.rdf";
const QString nsRdf = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";

/**
 * \brief Constructor
 */
UBMetaDataAdaptor::UBMetaDataAdaptor(){

}

/**
 * \brief Destructor
 */
UBMetaDataAdaptor::~UBMetaDataAdaptor(){

}

/**
 * \brief Persist the metadatas in a file on the file system
 * @param path as the file path
 * @param ns as the namespaces list
 * @param md as the metadatas list
 * @param desc as the description, for backward compatibility
 */
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

/**
 * \brief Load the metadatas
 */
void UBMetaDataAdaptor::load(){
    // TODO: Implement me!
}
