/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBTrashRegistery.h"
#include "core/UBSettings.h"
#include "board/UBFeaturesController.h"

UBTrashRegistery::UBTrashRegistery(UBFeaturesController* featureController)
{
    filePath = UBSettings::userDataDirectory()+"/trashRegister";
    mFeatureController = featureController;

    QFile fileRegister(filePath);

    if (!fileRegister.open(QIODevice::ReadOnly | QIODevice::Text))
         return;

    //load from the file
    QTextStream in(&fileRegister);
    while (!in.atEnd()) {
        QString entry = in.readLine();

        if(!entry.isEmpty()){
            addEntry(entry);
        }
    }

    fileRegister.close();
}

void UBTrashRegistery::addEntry(QString entry)
{
    QStringList data = entry.split(';');

    //Check if the file is in the trash
    QFile file(UBSettings::userTrashDirPath()+"/"+data[0]);

    if(file.exists()){
        addEntry(data[0], data[1], data[2]);
    }

}

void UBTrashRegistery::addEntry(const QString& filename, const QString& realPath, const QString& virtualPath)
{
    mRegister[filename] = RegisteryEntry(filename, realPath, virtualPath);
}

RegisteryEntry UBTrashRegistery::getEntry(const QString& filename) const
{
    return mRegister[filename];
}

void UBTrashRegistery::removeEnty(const QString& filename)
{
    mRegister.remove(filename);
}

void  UBTrashRegistery::saveToDisk()
{
    QFile fileRegister(filePath);

    if(!fileRegister.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&fileRegister);

    QMap<QString, RegisteryEntry>::iterator it = mRegister.begin();
    for(; it != mRegister.end(); ++it){
        RegisteryEntry entry = it.value();
        out << entry.filename << ';' << entry.originalRealPath << ';' << entry.originalVirtualPath << "\n";
    }

    fileRegister.close();
}

void UBTrashRegistery::synchronizeWith(const QString& path)
{
    QFileInfo file(path);

    //the given path must be a folder
    if(file.isDir()){
        QDir dir(path);
        QStringList allFiles = dir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);

        for(int i = 0; i < allFiles.size(); i++){
            QString filename = allFiles[i];
            if(mRegister.find(filename) == mRegister.end()){
                //If the file isn't register
                //Add it to the registery, and set the real and virtual to their default value
                CategoryData categoryData =  mFeatureController->getDestinationCategoryForUrl(QUrl(filename));
                UBFeature categoryFeature = categoryData.categoryFeature();

                QString realPath = categoryFeature.getFullPath().toLocalFile();
                QString virtualPath = categoryFeature.getFullVirtualPath();

                addEntry(filename, realPath, virtualPath);
            }
        }
    }
}
