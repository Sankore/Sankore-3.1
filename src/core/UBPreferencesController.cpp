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



#include "UBPreferencesController.h"

#include "UBSettings.h"
#include "UBApplication.h"

#include "gui/UBCircleFrame.h"

#include "core/UBSetting.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "podcast/UBPodcastController.h"

#include "ui_preferences.h"

#include "core/memcheck.h"

qreal UBPreferencesController::sSliderRatio = 10.0;
qreal UBPreferencesController::sMinPenWidth = 0.5;
qreal UBPreferencesController::sMaxPenWidth = 50.0;


UBPreferencesDialog::UBPreferencesDialog(UBPreferencesController* prefController, QWidget* parent,Qt::WindowFlags f)
    :QDialog(parent,f)
    ,mPreferencesController(prefController)
{
}

UBPreferencesDialog::~UBPreferencesDialog()
{
}

void UBPreferencesDialog::closeEvent(QCloseEvent* e)
{
    if(mPreferencesController->inputValuesConsistence())
        e->accept();
    else
        e->ignore();
}



UBPreferencesController::UBPreferencesController(QWidget *parent)
    : QObject(parent)
    , mPreferencesWindow(0)
    , mPreferencesUI(0)
    , mPenProperties(0)
    , mMarkerProperties(0)
{
    mDesktop = qApp->desktop();
    mPreferencesWindow = new UBPreferencesDialog(this,parent, Qt::Dialog);
    mPreferencesUI = new Ui::preferencesDialog();  // deleted in
    mPreferencesUI->setupUi(mPreferencesWindow);
    adjustScreens(1);
    connect(mDesktop, SIGNAL(screenCountChanged(int)), this, SLOT(adjustScreens(int)));

    connect(mPreferencesUI->languageComboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(onLanguageChanged(QString)));

    wire();
}


UBPreferencesController::~UBPreferencesController()
{
    delete mPreferencesWindow;

    delete mPreferencesUI;

    delete mPenProperties;

    delete mMarkerProperties;
}

void UBPreferencesController::adjustScreens(int screen)
{
    Q_UNUSED(screen);
    UBDisplayManager displayManager;
    mPreferencesUI->multiDisplayGroupBox->setEnabled(displayManager.numScreens() > 1);
}

void UBPreferencesController::show()
{
    init();

    mPreferencesWindow->exec();
}

void UBPreferencesController::wire()
{
    UBSettings* settings = UBSettings::settings();

    // main tab
    mPreferencesUI->mainTabWidget->setCurrentWidget(mPreferencesUI->displayTab);
    mPreferencesUI->versionLabel->setText(tr("version: ") + UBApplication::applicationVersion());

    connect(mPreferencesUI->closeButton, SIGNAL(released()), this, SLOT(close()));
    connect(mPreferencesUI->defaultSettingsButton, SIGNAL(released()), this, SLOT(defaultSettings()));

    connect(mPreferencesUI->startupTipsCheckBox,SIGNAL(clicked(bool)),this,SLOT(onStartupTipsClicked(bool)));


    // OSK preferences

    for(int i = 0; i < settings->supportedKeyboardSizes->size(); i++)
        mPreferencesUI->keyboardPaletteKeyButtonSize->addItem(settings->supportedKeyboardSizes->at(i));


    connect(mPreferencesUI->keyboardPaletteKeyButtonSize, SIGNAL(currentIndexChanged(const QString &)), settings->boardKeyboardPaletteKeyBtnSize, SLOT(setString(const QString &)));
    connect(mPreferencesUI->startModeComboBox, SIGNAL(currentIndexChanged(int)), settings->appStartMode, SLOT(setInt(int)));


    connect(mPreferencesUI->useExternalBrowserCheckBox, SIGNAL(clicked(bool)), settings->webUseExternalBrowser, SLOT(setBool(bool)));
    connect(mPreferencesUI->displayBrowserPageCheckBox, SIGNAL(clicked(bool)), settings->webShowPageImmediatelyOnMirroredScreen, SLOT(setBool(bool)));
    connect(mPreferencesUI->swapControlAndDisplayScreensCheckBox, SIGNAL(clicked(bool)), settings->swapControlAndDisplayScreens, SLOT(setBool(bool)));
    connect(mPreferencesUI->swapControlAndDisplayScreensCheckBox, SIGNAL(clicked(bool)), UBApplication::applicationController->displayManager(), SLOT(reinitScreens(bool)));

    connect(mPreferencesUI->toolbarAtTopRadioButton, SIGNAL(clicked(bool)), this, SLOT(toolbarPositionChanged(bool)));
    connect(mPreferencesUI->toolbarAtBottomRadioButton, SIGNAL(clicked(bool)), this, SLOT(toolbarPositionChanged(bool)));
    connect(mPreferencesUI->horizontalChoice, SIGNAL(clicked(bool)), this, SLOT(toolbarOrientationHorizontal(bool)));
    connect(mPreferencesUI->verticalChoice, SIGNAL(clicked(bool)), this, SLOT(toolbarOrientationVertical(bool)));
    connect(mPreferencesUI->toolbarDisplayTextCheckBox, SIGNAL(clicked(bool)), settings->appToolBarDisplayText, SLOT(setBool(bool)));

    // pen
    QList<QColor> penLightBackgroundColors = settings->boardPenLightBackgroundColors->colors();
    QList<QColor> penDarkBackgroundColors = settings->boardPenDarkBackgroundColors->colors();
    QList<QColor> penLightBackgroundSelectedColors = settings->boardPenLightBackgroundSelectedColors->colors();
    QList<QColor> penDarkBackgroundSelectedColors = settings->boardPenDarkBackgroundSelectedColors->colors();

    mPenProperties = new UBBrushPropertiesFrame(mPreferencesUI->penFrame,
                                                penLightBackgroundColors, penDarkBackgroundColors, penLightBackgroundSelectedColors,
                                                penDarkBackgroundSelectedColors, this);

    mPenProperties->opacityFrame->hide();

    connect(mPenProperties->fineSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->mediumSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->strongSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->pressureSensitiveCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setPenPressureSensitive(bool)));

    // marker
    QList<QColor> markerLightBackgroundColors = settings->boardMarkerLightBackgroundColors->colors();
    QList<QColor> markerDarkBackgroundColors = settings->boardMarkerDarkBackgroundColors->colors();
    QList<QColor> markerLightBackgroundSelectedColors = settings->boardMarkerLightBackgroundSelectedColors->colors();
    QList<QColor> markerDarkBackgroundSelectedColors = settings->boardMarkerDarkBackgroundSelectedColors->colors();

    mMarkerProperties = new UBBrushPropertiesFrame(mPreferencesUI->markerFrame, markerLightBackgroundColors,
                                                   markerDarkBackgroundColors, markerLightBackgroundSelectedColors,
                                                   markerDarkBackgroundSelectedColors, this);

    mMarkerProperties->pressureSensitiveCheckBox->setText(tr("Marker is pressure sensitive"));

    connect(mMarkerProperties->fineSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->mediumSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->strongSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->pressureSensitiveCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setMarkerPressureSensitive(bool)));
    connect(mMarkerProperties->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));


    //network
    connect(mPreferencesUI->Username_textBox, SIGNAL(editingFinished()), this, SLOT(onCommunityUsernameChanged()));
    connect(mPreferencesUI->Password_textEdit, SIGNAL(editingFinished()), this, SLOT(onCommunityPasswordChanged()));
    connect(mPreferencesUI->PSCredentialsPersistenceCheckBox,SIGNAL(clicked()),this, SLOT(onCommunityPersistenceChanged()));

    // about tab
    connect(mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox, SIGNAL(clicked(bool)), settings->appEnableAutomaticSoftwareUpdates, SLOT(setBool(bool)));
}

void UBPreferencesController::init()
{
    UBSettings* settings = UBSettings::settings();

    // about tab
    mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox->setChecked(settings->appEnableAutomaticSoftwareUpdates->get().toBool());

    // display tab
    for(int i=0; i<mPreferencesUI->keyboardPaletteKeyButtonSize->count(); i++)
        if (mPreferencesUI->keyboardPaletteKeyButtonSize->itemText(i) == settings->boardKeyboardPaletteKeyBtnSize->get().toString()) {
            mPreferencesUI->keyboardPaletteKeyButtonSize->setCurrentIndex(i);
            break;
        }

    mPreferencesUI->startupTipsCheckBox->setChecked(settings->appStartupHintsEnabled->get().toBool());

    mPreferencesUI->startModeComboBox->setCurrentIndex(settings->appStartMode->get().toInt());

    mPreferencesUI->useExternalBrowserCheckBox->setChecked(settings->webUseExternalBrowser->get().toBool());
    mPreferencesUI->displayBrowserPageCheckBox->setChecked(settings->webShowPageImmediatelyOnMirroredScreen->get().toBool());
    mPreferencesUI->webHomePage->setText(settings->webHomePage->get().toString());

    mPreferencesUI->proxyUsername->setText(settings->proxyUsername());
    mPreferencesUI->proxyPassword->setText(settings->proxyPassword());

    mPreferencesUI->toolbarAtTopRadioButton->setChecked(settings->appToolBarPositionedAtTop->get().toBool());
    mPreferencesUI->toolbarAtBottomRadioButton->setChecked(!settings->appToolBarPositionedAtTop->get().toBool());
    mPreferencesUI->toolbarDisplayTextCheckBox->setChecked(settings->appToolBarDisplayText->get().toBool());
    mPreferencesUI->verticalChoice->setChecked(settings->appToolBarOrientationVertical->get().toBool());
    mPreferencesUI->horizontalChoice->setChecked(!settings->appToolBarOrientationVertical->get().toBool());

    mPreferencesUI->Username_textBox->setText(settings->communityUsername());
    mPreferencesUI->Password_textEdit->setText(settings->communityPassword());
    mPreferencesUI->swapControlAndDisplayScreensCheckBox->setChecked(settings->swapControlAndDisplayScreens->get().toBool());

    // pen tab
    mPenProperties->fineSlider->setValue(settings->boardPenFineWidth->get().toDouble() * sSliderRatio);
    mPenProperties->mediumSlider->setValue(settings->boardPenMediumWidth->get().toDouble() * sSliderRatio);
    mPenProperties->strongSlider->setValue(settings->boardPenStrongWidth->get().toDouble() * sSliderRatio);
    mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->get().toBool());

    // marker tab
    mMarkerProperties->fineSlider->setValue(settings->boardMarkerFineWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->mediumSlider->setValue(settings->boardMarkerMediumWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->strongSlider->setValue(settings->boardMarkerStrongWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->get().toBool());

    mMarkerProperties->opacitySlider->setValue(settings->boardMarkerAlpha->get().toDouble() * 100);

    //network
    mPreferencesUI->PSCredentialsPersistenceCheckBox->setChecked(settings->getCommunityDataPersistence());
    persistanceCheckboxUpdate();

    //Issue NC - CFA - 20140520 : clear list, to prevent duplication of the list
    mIsoCodeAndLanguage.clear();
    mIsoCodeAndLanguage.insert(tr("Default"),"NO_VALUE");
    mIsoCodeAndLanguage.insert(tr("Arabic"),"ar");
    mIsoCodeAndLanguage.insert(tr("Basque"),"eu");
    mIsoCodeAndLanguage.insert(tr("Bambara"),"bm");
    mIsoCodeAndLanguage.insert(tr("Bulgarian"),"bg");
    mIsoCodeAndLanguage.insert(tr("Catalan"),"ca");
    mIsoCodeAndLanguage.insert(tr("Corsican"),"co");
    mIsoCodeAndLanguage.insert(tr("Czech"),"cs");
    mIsoCodeAndLanguage.insert(tr("Danish"),"da");
    mIsoCodeAndLanguage.insert(tr("German"),"de");
    mIsoCodeAndLanguage.insert(tr("Greek"),"el");
    mIsoCodeAndLanguage.insert(tr("English"),"en");
    mIsoCodeAndLanguage.insert(tr("English UK"),"en_UK");
    mIsoCodeAndLanguage.insert(tr("Spanish"),"es");
    mIsoCodeAndLanguage.insert(tr("Finnish"),"fi");
    mIsoCodeAndLanguage.insert(tr("French"),"fr");
    mIsoCodeAndLanguage.insert(tr("Swiss French"),"fr_CH");
    mIsoCodeAndLanguage.insert(tr("Hindi"),"hi");
    mIsoCodeAndLanguage.insert(tr("Hungarian"),"hu");
    mIsoCodeAndLanguage.insert(tr("Italian"),"it");
    mIsoCodeAndLanguage.insert(tr("Hebrew"),"iw");
    mIsoCodeAndLanguage.insert(tr("Japanese"),"ja");
    mIsoCodeAndLanguage.insert(tr("Korean"),"ko");
    mIsoCodeAndLanguage.insert(tr("Malagasy"),"mg");
    mIsoCodeAndLanguage.insert(tr("Norwegian"),"nb");
    mIsoCodeAndLanguage.insert(tr("Dutch"),"nl");
    mIsoCodeAndLanguage.insert(tr("Occitan"), "oc");
    mIsoCodeAndLanguage.insert(tr("Polish"),"pl");
    mIsoCodeAndLanguage.insert(tr("Portuguese"),"pt");
    mIsoCodeAndLanguage.insert(tr("Brazilian Portuguese"),"pt_BR");
    mIsoCodeAndLanguage.insert(tr("Romansh"),"rm");
    mIsoCodeAndLanguage.insert(tr("Romanian"),"ro");
    mIsoCodeAndLanguage.insert(tr("Russian"),"ru");
    mIsoCodeAndLanguage.insert(tr("Slovak"),"sk");
    mIsoCodeAndLanguage.insert(tr("Swedish"),"sv");
    mIsoCodeAndLanguage.insert(tr("Turkish"),"tr");
    mIsoCodeAndLanguage.insert(tr("Chinese"),"zh");
    mIsoCodeAndLanguage.insert(tr("Chinese Simplified"),"zh_CN");
    mIsoCodeAndLanguage.insert(tr("Chinese Traditional"),"zh_TW");
    mIsoCodeAndLanguage.insert(tr("Galician"),"gl");

    QStringList list;
    list << mIsoCodeAndLanguage.keys();
    list.sort();
    //Issue NC - CFA - 20140520 : clear list, to prevent duplication of the list
    QString currentIsoLanguage = UBSettings::settings()->appPreferredLanguage->get().toString();
    mPreferencesUI->languageComboBox->clear();
    mPreferencesUI->languageComboBox->addItems(list);
    if(currentIsoLanguage.length()){
        QString language;
        foreach(QString eachKey, mIsoCodeAndLanguage.keys())
            if(mIsoCodeAndLanguage[eachKey] == currentIsoLanguage){
                language = eachKey;
                break;
            }
        mPreferencesUI->languageComboBox->setCurrentIndex(list.indexOf(language));
    }
    else
        mPreferencesUI->languageComboBox->setCurrentIndex(list.indexOf("Default"));

    connect(mPreferencesUI->quitOpenSankorePushButton,SIGNAL(clicked()),UBApplication::app(),SLOT(closing()));
    mPreferencesUI->quitOpenSankorePushButton->setDisabled(true);

}

void UBPreferencesController::onStartupTipsClicked(bool clicked)
{
    UBSettings::settings()->appStartupHintsEnabled->setBool(clicked);
}

void UBPreferencesController::onLanguageChanged(QString currentItem)
{
    QString isoCode = mIsoCodeAndLanguage[currentItem] == "NO_VALUE" ? "" : mIsoCodeAndLanguage[currentItem];
    UBSettings::settings()->appPreferredLanguage->setString(isoCode);
    mPreferencesUI->quitOpenSankorePushButton->setEnabled(true);
}

void UBPreferencesController::onCommunityUsernameChanged()
{
    UBSettings* settings = UBSettings::settings();
    settings->setCommunityUsername(mPreferencesUI->Username_textBox->text());
    persistanceCheckboxUpdate();
}

void UBPreferencesController::onCommunityPasswordChanged()
{
    UBSettings* settings = UBSettings::settings();
    settings->setCommunityPassword(mPreferencesUI->Password_textEdit->text());
    persistanceCheckboxUpdate();
}

void UBPreferencesController::onCommunityPersistenceChanged()
{
    UBSettings::settings()->setCommunityPersistence(mPreferencesUI->PSCredentialsPersistenceCheckBox->isChecked());
}

void UBPreferencesController::persistanceCheckboxUpdate()
{
    bool checkBoxEnabled = mPreferencesUI->Username_textBox->text().length() || mPreferencesUI->Password_textEdit->text().length();
    mPreferencesUI->PSCredentialsPersistenceCheckBox->setEnabled(checkBoxEnabled);
    mPreferencesUI->PSCredentialsPersistenceCheckBox->setStyleSheet(checkBoxEnabled ? "color:black;" : "color:lightgray;");
}


bool UBPreferencesController::inputValuesConsistence()
{
    QString backgroundStyle = "QWidget {background-color: white}";
    mPreferencesUI->Username_textBox->setStyleSheet(backgroundStyle);
    mPreferencesUI->Password_textEdit->setStyleSheet(backgroundStyle);

    QString username = mPreferencesUI->Username_textBox->text();
    QString password = mPreferencesUI->Password_textEdit->text();
    bool isConsistent = true;
    if (username.length() + password.length()){
        backgroundStyle = "QWidget {background-color: magenta}";
        if(username.isEmpty()){
            isConsistent = false;
            mPreferencesUI->mainTabWidget->setCurrentWidget(mPreferencesUI->networkTab);
            mPreferencesUI->Username_textBox->setStyleSheet(backgroundStyle);
            mPreferencesUI->Username_textBox->setFocus();
            mPreferencesUI->Username_textBox->setCursorPosition(0);
        }
        else if(password.isEmpty()){
            isConsistent = false;
            mPreferencesUI->mainTabWidget->setCurrentWidget(mPreferencesUI->networkTab);
            mPreferencesUI->Password_textEdit->setStyleSheet(backgroundStyle);
            mPreferencesUI->Password_textEdit->setFocus();
            mPreferencesUI->Password_textEdit->setCursorPosition(0);
        }
    }
    return isConsistent;
}

void UBPreferencesController::close()
{
    //web
    QString homePage = mPreferencesUI->webHomePage->text();

    UBSettings::settings()->webHomePage->set(homePage);
    UBSettings::settings()->setProxyUsername(mPreferencesUI->proxyUsername->text());
    UBSettings::settings()->setProxyPassword(mPreferencesUI->proxyPassword->text());

    if (!inputValuesConsistence())
        return;

    mPreferencesWindow->accept();
}


void UBPreferencesController::defaultSettings()
{
    UBSettings* settings = UBSettings::settings();

    if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->displayTab)
    {
        bool defaultValue = settings->appToolBarPositionedAtTop->reset().toBool();
        mPreferencesUI->toolbarAtTopRadioButton->setChecked(defaultValue);
        mPreferencesUI->toolbarAtBottomRadioButton->setChecked(!defaultValue);

        defaultValue = settings->appToolBarDisplayText->reset().toBool();
        mPreferencesUI->toolbarDisplayTextCheckBox->setChecked(defaultValue);
        mPreferencesUI->verticalChoice->setChecked(settings->appToolBarOrientationVertical->reset().toBool());
        mPreferencesUI->horizontalChoice->setChecked(!settings->appToolBarOrientationVertical->reset().toBool());
        mPreferencesUI->startModeComboBox->setCurrentIndex(0);
        onLanguageChanged("Default");
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->penTab)
    {

        mPenProperties->fineSlider->setValue(settings->boardPenFineWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->mediumSlider->setValue(settings->boardPenMediumWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->strongSlider->setValue(settings->boardPenStrongWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->reset().toBool());

        settings->boardPenLightBackgroundSelectedColors->reset();
        QList<QColor> lightBackgroundSelectedColors = settings->boardPenLightBackgroundSelectedColors->colors();

        settings->boardPenDarkBackgroundSelectedColors->reset();
        QList<QColor> darkBackgroundSelectedColors = settings->boardPenDarkBackgroundSelectedColors->colors();

        for (int i = 0 ; i < settings->colorPaletteSize ; i++)
        {
            mPenProperties->lightBackgroundColorPickers[i]->setSelectedColorIndex(lightBackgroundSelectedColors.indexOf(settings->penColors(false).at(i)));
            mPenProperties->darkBackgroundColorPickers[i]->setSelectedColorIndex(darkBackgroundSelectedColors.indexOf(settings->penColors(true).at(i)));
        }
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->markerTab)
    {
        mMarkerProperties->fineSlider->setValue(settings->boardMarkerFineWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->mediumSlider->setValue(settings->boardMarkerMediumWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->strongSlider->setValue(settings->boardMarkerStrongWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->reset().toBool());

        mMarkerProperties->opacitySlider->setValue(settings->boardMarkerAlpha->reset().toDouble() * 100);

        settings->boardMarkerLightBackgroundSelectedColors->reset();
        QList<QColor> lightBackgroundSelectedColors = settings->boardMarkerLightBackgroundSelectedColors->colors();

        settings->boardMarkerDarkBackgroundSelectedColors->reset();
        QList<QColor> darkBackgroundSelectedColors = settings->boardMarkerDarkBackgroundSelectedColors->colors();

        for (int i = 0 ; i < settings->colorPaletteSize ; i++)
        {
            mMarkerProperties->lightBackgroundColorPickers[i]->setSelectedColorIndex(lightBackgroundSelectedColors.indexOf(settings->markerColors(false).at(i)));
            mMarkerProperties->darkBackgroundColorPickers[i]->setSelectedColorIndex(darkBackgroundSelectedColors.indexOf(settings->markerColors(true).at(i)));
        }
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->aboutTab)
    {
        bool defaultValue = settings->appEnableAutomaticSoftwareUpdates->reset().toBool();
        mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox->setChecked(defaultValue);
    }
    else if(mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->networkTab){
        bool defaultValue = settings->webUseExternalBrowser->reset().toBool();
        mPreferencesUI->useExternalBrowserCheckBox->setChecked(defaultValue);
        defaultValue = settings->webShowPageImmediatelyOnMirroredScreen->reset().toBool();
        mPreferencesUI->displayBrowserPageCheckBox->setChecked(defaultValue);

        mPreferencesUI->webHomePage->setText(settings->webHomePage->reset().toString());
    }

}


void UBPreferencesController::widthSliderChanged(int value)
{
    UBSettings* settings = UBSettings::settings();

    qreal width = value / sSliderRatio;

    QObject *slider = sender();
    UBCircleFrame* display = 0;

    if (slider == mPenProperties->fineSlider)
    {
        settings->boardPenFineWidth->set(width);
        display = mPenProperties->fineDisplayFrame;
    }
    else if (slider == mPenProperties->mediumSlider)
    {
        settings->boardPenMediumWidth->set(width);
        display = mPenProperties->mediumDisplayFrame;
    }
    else if (slider == mPenProperties->strongSlider)
    {
        settings->boardPenStrongWidth->set(width);
        display = mPenProperties->strongDisplayFrame;
    }
    else if (slider == mMarkerProperties->fineSlider)
    {
        settings->boardMarkerFineWidth->set(width);
        display = mMarkerProperties->fineDisplayFrame;
    }
    else if (slider == mMarkerProperties->mediumSlider)
    {
        settings->boardMarkerMediumWidth->set(width);
        display = mMarkerProperties->mediumDisplayFrame;
    }
    else if (slider == mMarkerProperties->strongSlider)
    {
        settings->boardMarkerStrongWidth->set(width);
        display = mMarkerProperties->strongDisplayFrame;
    }
    else
    {
        qDebug() << "widthSliderChanged : unknown sender ... ignoring event";
    }

    if (display)
    {
        display->currentPenWidth = width;
        display->maxPenWidth = sMaxPenWidth;
        display->repaint();
    }
}


void UBPreferencesController::opacitySliderChanged(int value)
{
    qreal opacity = ((qreal)value) / 100;

    QObject *slider = sender();

    if (slider == mMarkerProperties->opacitySlider)
    {
        UBDrawingController::drawingController()->setMarkerAlpha(opacity);
    }
}


void UBPreferencesController::colorSelected(const QColor& color)
{
    UBColorPicker *colorPicker = qobject_cast<UBColorPicker*>(sender());

    int index = mPenProperties->lightBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setPenColor(false, color, index);
        return;
    }

    index = mPenProperties->darkBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setPenColor(true, color, index);
    }

    index = mMarkerProperties->lightBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setMarkerColor(false, color, index);
    }

    index = mMarkerProperties->darkBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setMarkerColor(true, color, index);
    }

}


void UBPreferencesController::toolbarPositionChanged(bool checked)
{
    Q_UNUSED(checked);

    UBSettings* settings = UBSettings::settings();

    settings->appToolBarPositionedAtTop->set(mPreferencesUI->toolbarAtTopRadioButton->isChecked());

}

void UBPreferencesController::toolbarOrientationVertical(bool checked)
{
    UBSettings* settings = UBSettings::settings();
    settings->appToolBarOrientationVertical->set(checked);
}

void UBPreferencesController::toolbarOrientationHorizontal(bool checked)
{
    UBSettings* settings = UBSettings::settings();
    settings->appToolBarOrientationVertical->set(!checked);
}

UBBrushPropertiesFrame::UBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,
                                               const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,
                                               const QList<QColor>& darkBackgroundSelectedColors, UBPreferencesController* controller)
{
    setupUi(owner);

    QPalette lightBackgroundPalette = QApplication::palette();
    lightBackgroundPalette.setColor(QPalette::Window, Qt::white);

    lightBackgroundFrame->setAutoFillBackground(true);
    lightBackgroundFrame->setPalette(lightBackgroundPalette);

    QPalette darkBackgroundPalette = QApplication::palette();
    darkBackgroundPalette.setColor(QPalette::Window, Qt::black);
    darkBackgroundPalette.setColor(QPalette::ButtonText, Qt::white);
    darkBackgroundPalette.setColor(QPalette::WindowText, Qt::white);

    darkBackgroundFrame->setAutoFillBackground(true);
    darkBackgroundFrame->setPalette(darkBackgroundPalette);
    darkBackgroundLabel->setPalette(darkBackgroundPalette);

    QList<QColor> firstLightBackgroundColor;
    firstLightBackgroundColor.append(lightBackgroundColors[0]);

    lightBackgroundColorPicker0->setColors(firstLightBackgroundColor);
    lightBackgroundColorPicker0->setSelectedColorIndex(0);
    lightBackgroundColorPickers.append(lightBackgroundColorPicker0);

    for (int i = 1 ; i < UBSettings::settings()->colorPaletteSize ; i++)
    {
        UBColorPicker *picker = new UBColorPicker(lightBackgroundFrame);
        picker->setObjectName(QString::fromUtf8("penLightBackgroundColor") + i);
        picker->setMinimumSize(QSize(32, 32));
        picker->setFrameShape(QFrame::StyledPanel);
        picker->setFrameShadow(QFrame::Raised);

        lightBackgroundLayout->addWidget(picker);

        picker->setColors(lightBackgroundColors);

        picker->setSelectedColorIndex(lightBackgroundColors.indexOf(lightBackgroundSelectedColors.at(i)));

        lightBackgroundColorPickers.append(picker);

        QObject::connect(picker, SIGNAL(colorSelected(const QColor&)), controller, SLOT(colorSelected(const QColor&)));

    }

    QList<QColor> firstDarkBackgroundColor;
    firstDarkBackgroundColor.append(darkBackgroundColors[0]);

    darkBackgroundColorPicker0->setColors(firstDarkBackgroundColor);
    darkBackgroundColorPicker0->setSelectedColorIndex(0);
    darkBackgroundColorPickers.append(darkBackgroundColorPicker0);

    for (int i = 1 ; i < UBSettings::settings()->colorPaletteSize ; i++)
    {
        UBColorPicker *picker = new UBColorPicker(darkBackgroundFrame);
        picker->setObjectName(QString::fromUtf8("penDarkBackgroundColor") + i);
        picker->setMinimumSize(QSize(32, 32));
        picker->setFrameShape(QFrame::StyledPanel);
        picker->setFrameShadow(QFrame::Raised);

        darkBackgroundLayout->addWidget(picker);

        picker->setColors(darkBackgroundColors);
        picker->setSelectedColorIndex(darkBackgroundColors.indexOf(darkBackgroundSelectedColors.at(i)));

        darkBackgroundColorPickers.append(picker);

        QObject::connect(picker, SIGNAL(colorSelected(const QColor&)), controller, SLOT(colorSelected(const QColor&)));

    }
}
