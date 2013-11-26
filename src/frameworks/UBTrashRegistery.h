#ifndef UBTRASHREGISTERY_H_H
#define UBTRASHREGISTERY_H_H

#include <QString>
#include <QStringList>
#include <QMap>

class UBFeaturesController;

/*!
 *  Represents an entry of the register. An entry hold the following data :
 *  the filename, the original real path and the originale virtual path.
 */
struct RegisteryEntry
{
    QString filename;
    QString originalRealPath;
    QString originalVirtualPath;

    RegisteryEntry(){ }

    RegisteryEntry(QString filename, QString realPath, QString virtualPath)
    {
        this->filename = filename;
        this->originalRealPath = realPath;
        this->originalVirtualPath = virtualPath;
    }
};

/*!
 * This class represents the register of the trash. The goal of the register is to store
 * the real and virtual path to allow retrive to the original place a file which is placed in the trash.
 */
class UBTrashRegistery
{
public:
    /*!
     * Default constructor. Read the file wich contains the register and load it.
     */
    UBTrashRegistery(UBFeaturesController *featureController);

    /*!
     * Add an new entry to the register. If the file exists on the disk, it is added in the registery otherwise
     * it not added.
     * @param filename The name of the file
     * @param realPath The real path of the file (ie : on the disk)
     * @param virtualPath The virtual path of the file (ie : the path in sankore)
     */
    void addEntry(const QString &filename, const QString &realPath, const QString &virtualPath);

    /*!
     * Get a entry according the filename.
     * @param filename The name of the file.
     * @return Return the registrer entry according the filename.
     */
    RegisteryEntry getEntry(const QString& filename) const;

    /*!
     * Remove an entry in the register.
     * @param filename The filename which is contained in the entry to remove.
     */
    void removeEnty(const QString &filename);

    /*!
     * Add a new entry in the regiser according a string which contains the filename, the real path and the
     * virtual path separate by ';' character.
     * @param entry The data of the new entry.
     */
    void addEntry(QString entry);

    /*!
     * Save to the disk the actual state of the register.
     */
    void saveToDisk();

    void synchronizeWith(const QString& path);

private:
    QMap<QString, RegisteryEntry> mRegister; ///< This is the register.
    QString filePath; ///< The file path where to save the register.
    UBFeaturesController *mFeatureController; ///< A reference to the feature controller
};

#endif // UBTRASHREGISTERY_H
