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
#ifndef UBABSTRACTDOCUMENTUSER_H
#define UBABSTRACTDOCUMENTUSER_H

#include <QObject>
#include "interfaces/IDocument.h"
#include "interfaces/IDocumentUser.h"

class UBAbstractDocumentUser : public QObject,  IDocumentUser{
    Q_OBJECT
    Q_INTERFACES(IDocumentUser)
public:
    UBAbstractDocumentUser();
    virtual ~UBAbstractDocumentUser();
    virtual void setDocument(IDocument* doc);
    virtual IDocument* document();

private:
    IDocument* mpDocument;
};

#endif // UBABSTRACTDOCUMENTUSER_H
