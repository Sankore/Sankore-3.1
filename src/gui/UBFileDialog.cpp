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

#include "UBFileDialog.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "gui/UBKeyboardPalette.h"
#include "gui/UBMainWindow.h"

UBFileDialog::UBFileDialog(QWidget *parent,
             const QString &caption,
             const QString &dir,
             const QString &filter,
             Options options,
             FileMode fileMode,
             AcceptMode acceptMode):
    QFileDialog(parent, caption, dir, filter)
{

    hide();
    setWindowModality(Qt::WindowModal);

    setOptions(options);
    setAcceptMode(acceptMode);
    setFileMode(fileMode);
}

QString UBFileDialog::getSaveFileName(QWidget *parent,
                                      const QString &caption,
                                      const QString &dir,
                                      const QString &filter,
                                      QString *selectedFilter,
                                      Options options)
{
    UBFileDialog dialog(parent, caption,dir, filter, options);

    UBKeyboardPalette* keyboard  = UBApplication::boardController->paletteManager()->mKeyboardPalette;

    keyboard->detachFromParent();


    if (selectedFilter)
        dialog.selectNameFilter(*selectedFilter);

    QString filename;
    if (dialog.exec() == QDialog::Accepted) {
         if (selectedFilter)
             *selectedFilter = dialog.selectedFilter();

         filename = dialog.selectedFiles().value(0);
    }

    keyboard->attachToParent();
    return filename;
}

QStringList UBFileDialog::getOpenFileNames(QWidget * parent,
                                   const QString & caption,
                                    const QString & dir,
                                    const QString & filter,
                                    QString * selectedFilter,
                                    Options options)
{
    UBKeyboardPalette* keyboard  = UBApplication::boardController->paletteManager()->mKeyboardPalette;

    keyboard->detachFromParent();

    UBFileDialog dialog(parent, caption, dir, filter, options, ExistingFiles);

    QStringList filenames;

    if (selectedFilter)
        dialog.selectNameFilter(*selectedFilter);
    if (dialog.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog.selectedFilter();
        filenames = dialog.selectedFiles();
    }

    keyboard->attachToParent();

    return filenames;
}

QString UBFileDialog::getExistingDirectory(QWidget * parent,
                                    const QString & caption,
                                    const QString & dir,
                                    Options options)
{
    UBKeyboardPalette* keyboard  = UBApplication::boardController->paletteManager()->mKeyboardPalette;

    keyboard->detachFromParent();

   FileMode mode = (options & ShowDirsOnly ? DirectoryOnly : Directory);
   UBFileDialog dialog(parent, caption, dir, QString(), options, mode);

   QString directory;

   if (dialog.exec() == QDialog::Accepted) {
       directory = dialog.selectedFiles().value(0);
   }

   keyboard->attachToParent();

   return directory;
}

QString UBFileDialog::getOpenFileName(QWidget *parent,
                               const QString &caption,
                               const QString &dir,
                               const QString &filter,
                               QString *selectedFilter,
                               Options options)
{
    UBKeyboardPalette* keyboard  = UBApplication::boardController->paletteManager()->mKeyboardPalette;
    keyboard->detachFromParent();

    UBFileDialog dialog(parent, caption, dir, filter, options, ExistingFiles);

    QString filename;

    if (selectedFilter)
        dialog.selectNameFilter(*selectedFilter);
    if (dialog.exec() == QDialog::Accepted) {
        if (selectedFilter)
            *selectedFilter = dialog.selectedFilter();
        filename = dialog.selectedFiles().value(0);
    }

    keyboard->attachToParent();

    return filename;
}

bool UBFileDialog::event(QEvent *event)
{
    UBKeyboardPalette* keyboard  = UBApplication::boardController->paletteManager()->mKeyboardPalette;

    switch(event->type()){
        case QEvent::WindowActivate :
            keyboard->onWindowFocusGain();
            break ;

        case QEvent::WindowDeactivate :
            keyboard->onWindowFocusLost();
            break ;
        default:
            break;
    };

    return QFileDialog::event(event);
}
