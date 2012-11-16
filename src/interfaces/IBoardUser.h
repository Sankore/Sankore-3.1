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
#ifndef IBOARDUSER_H
#define IBOARDUSER_H

#include <QObject>

class IBoardUser{
public:
    ~IBoardUser(){}
    virtual QObject* boardUser() = 0;

public slots:
    virtual void onActiveSceneChanged() = 0; // connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
    virtual void onActiveDocumentChanged() = 0; // connect(UBApplication::boardController, SIGNAL(documentSet(UBDocumentProxy*)), this, SLOT(onActiveDocumentChanged()));
};

Q_DECLARE_INTERFACE(IBoardUser, "org.open-sankore.interfaces.IBoardUser")

#endif // IBOARDUSER_H
