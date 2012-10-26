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
#ifndef IMETADATAPROVIDER_H
#define IMETADATAPROVIDER_H

#include <QString>
#include <QVector>

typedef struct{
    QString key;
    QString value;
}sMetaData;

class IMetaDataProvider{
public:
    virtual ~IMetaDataProvider(){}
    virtual void save()=0;
    virtual QString nameSpace()=0;
    virtual QString nameSpaceUrl()=0;

protected:
    QVector<sMetaData> mMetaDatas;
};

#endif // IMETADATAPROVIDER_H
