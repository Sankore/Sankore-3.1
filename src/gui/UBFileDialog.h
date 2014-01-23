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

#ifndef UBFILEDIALOG_H
#define UBFILEDIALOG_H

#include <qfiledialog.h>

class UBFileDialog : public QFileDialog
{
public:
    UBFileDialog(QWidget *parent = 0):
        QFileDialog(parent)
    {
        hide();
        setWindowModality(Qt::WindowModal);
        show();
    }

    UBFileDialog(QWidget *parent = 0,
                 const QString &caption = QString(),
                 const QString &dir = QString(),
                 const QString &filter = QString(),
                 Options options = 0,
                 FileMode fileMode = AnyFile,
                 AcceptMode acceptMode = AcceptOpen
                 );


    bool event(QEvent *event);

    static QString getSaveFileName(QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   Options options = 0);

    static QStringList getOpenFileNames(QWidget * parent = 0,
                                        const QString & caption = QString(),
                                        const QString & dir = QString(),
                                        const QString & filter = QString(),
                                        QString * selectedFilter = 0,
                                        Options options = 0);

    static QString getExistingDirectory(QWidget * parent = 0,
                                        const QString & caption = QString(),
                                        const QString & dir = QString(),
                                        Options options = ShowDirsOnly);

    static QString getOpenFileName(QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   Options options = 0);
};

#endif // UBFILEDIALOG_H
