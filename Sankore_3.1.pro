TARGET = "Open-Sankore"
TEMPLATE = app

THIRD_PARTY_PATH=../Sankore-ThirdParty

CONFIG -= flat
CONFIG += debug_and_release \
          no_include_pwd


VERSION_MAJ = 2
VERSION_MIN = 5
VERSION_TYPE = r # a = alpha, b = beta, r = release, other => error
VERSION_PATCH = 1

VERSION = "$${VERSION_MAJ}.$${VERSION_MIN}.$${VERSION_TYPE}.$${VERSION_PATCH}"
VERSION = $$replace(VERSION, "\\.r", "")

LONG_VERSION = "$${VERSION}.$${SVN_VERSION}"
macx:OSX_VERSION = "$${VERSION} (r$${SVN_VERSION})"

VERSION_RC = $$VERSION_MAJ,$$VERSION_MIN,$$VERSION_TYPE,$$VERSION_PATCH
VERSION_RC = $$replace(VERSION_RC, "a", "160") # 0xA0
VERSION_RC = $$replace(VERSION_RC, "b", "176") # 0xB0
VERSION_RC = $$replace(VERSION_RC, "r", "240") # 0xF0

QT += webkit
QT += svg
QT += network
QT += phonon
QT += xml
QT += script
QT += xmlpatterns

INCLUDEPATH += src

include($$THIRD_PARTY_PATH/libs.pri)
include(src/adaptors/adaptors.pri)
include(src/api/api.pri)
include(src/board/board.pri)
include(src/core/core.pri)
include(src/document/document.pri)
include(src/domain/domain.pri)
include(src/frameworks/frameworks.pri)
include(src/gui/gui.pri)
include(src/network/network.pri)
include(src/pdf/pdf.pri)
include(src/podcast/podcast.pri)
include(src/tools/tools.pri)
include(src/desktop/desktop.pri)
include(src/web/web.pri)
include(src/transition/transition.pri)
include(src/interfaces/interfaces.pri)
include(src/customWidgets/customWidgets.pri)

DEPENDPATH += src/pdf-merger
INCLUDEPATH += src/pdf-merger
include(src/pdf-merger/pdfMerger.pri)

#plugins
include(plugins/plugins.pri)
INCLUDEPATH += plugins/cffadaptor/src


#ThirdParty
DEPENDPATH += $$THIRD_PARTY_PATH/quazip/
INCLUDEPATH += $$THIRD_PARTY_PATH/quazip/
include($$THIRD_PARTY_PATH/quazip/quazip.pri)
DEPENDPATH += $$THIRD_PARTY_PATH/trolltech/singleapplication
INCLUDEPATH += $$THIRD_PARTY_PATH/trolltech/singleapplication
include($$THIRD_PARTY_PATH/trolltech/singleapplication/qtsingleapplication.pri)

FORMS += resources/forms/mainWindow.ui \
   resources/forms/preferences.ui \
   resources/forms/brushProperties.ui \
   resources/forms/documents.ui \
   resources/forms/blackoutWidget.ui \
   resources/forms/trapFlash.ui \
   resources/forms/youTubePublishingDialog.ui \
   resources/forms/webPublishing.ui \
   resources/forms/capturePublishing.ui \
   resources/forms/intranetPodcastPublishingDialog.ui

UB_ETC.files = resources/etc
UB_I18N.files = resources/i18n/*.qm
UB_LIBRARY.files = resources/library
UB_FONTS.files = resources/fonts
UB_THIRDPARTY_INTERACTIVE.files = thirdparty/interactive

DEFINES += NO_THIRD_PARTY_WARNINGS
DEFINES += VERSION_TYPE=\"\\\"$${VERSION_TYPE}\"\\\"
DEFINES += VERSION_PATCH=\"\\\"$${VERSION_PATCH}\"\\\"
DEFINES += UBVERSION=\"\\\"$${LONG_VERSION}\"\\\" \
   UBVERSION_RC=$$VERSION_RC
ALPHA_BETA_STR = $$find(VERSION, "[ab]")
count(ALPHA_BETA_STR, 1):DEFINES += PRE_RELEASE
BUILD_DIR = build

macx:BUILD_DIR = $$BUILD_DIR/macx
win32:BUILD_DIR = $$BUILD_DIR/win32
linux-*:BUILD_DIR = $$BUILD_DIR/linux

CONFIG(debug, debug|release):BUILD_DIR = $$BUILD_DIR/debug
CONFIG(release, debug|release) {
   BUILD_DIR = $$BUILD_DIR/release
   CONFIG += warn_off
}

DESTDIR = $$BUILD_DIR/product
OBJECTS_DIR = $$BUILD_DIR/objects
MOC_DIR = $$BUILD_DIR/moc
RCC_DIR = $$BUILD_DIR/rcc
UI_DIR = $$BUILD_DIR/ui

win32 {
   RC_FILE = resources/win/sankore.rc
   CONFIG += qaxcontainer
   exists(console):CONFIG += console
   QMAKE_CXXFLAGS += /MP
   QMAKE_CXXFLAGS_RELEASE += /Od /Zi
   QMAKE_LFLAGS_RELEASE += /DEBUG
   UB_LIBRARY.path = $$DESTDIR
   UB_I18N.path = $$DESTDIR/i18n
   UB_ETC.path = $$DESTDIR
   UB_THIRDPARTY_INTERACTIVE.path = $$DESTDIR/library
   system(md $$replace(BUILD_DIR, /, \\))
   system(echo "$$VERSION" > $$BUILD_DIR/version)
   system(echo "$$LONG_VERSION" > $$BUILD_DIR/longversion)
   system(echo "$$SVN_VERSION" > $$BUILD_DIR/svnversion)

}

macx {
   LIBS += -framework Foundation
   LIBS += -lcrypto
   #commented because Sankore crashes on Java Script. It seems to backends dependencies.
   #LIBS += -framework AppKit
   #LIBS += -framework WebKit

   CONFIG(release, debug|release):CONFIG += x86

   # [03-02-2011] We must use the 32bit version for the moment
   # because the Quicktime components used by this application
   # are not yet available in 64bits.
   CONFIG(debug, debug|release):CONFIG += x86

   QMAKE_MAC_SDK = "/Developer/SDKs/MacOSX10.6.sdk"
   QMAKE_MACOSX_DEPLOYMENT_TARGET = "10.5"

   VERSION_RC_PATH = "$$BUILD_DIR/version_rc"

   # Embed version into executable for breakpad
   QMAKE_LFLAGS += -sectcreate \
       __DATA \
       __version \
       $$VERSION_RC_PATH

   QMAKE_CXXFLAGS_RELEASE += -gdwarf-2 \
       -mdynamic-no-pic

   QMAKE_CFLAGS += -fopenmp
   QMAKE_CXXFLAGS += -fopenmp
   QMAKE_LFLAGS += -fopenmp

   CONTENTS_DIR = "Contents"
   RESOURCES_DIR = "Contents/Resources"
   FRAMEWORKS_DIR = "Contents/Frameworks"

   UB_ETC.files = "resources/etc"
   UB_ETC.path = "$$RESOURCES_DIR"
   UB_LIBRARY.files = "resources/library"
   UB_LIBRARY.path = "$$RESOURCES_DIR"
   UB_FONTS.files = "resources/fonts"
   UB_FONTS.path = "$$RESOURCES_DIR"
   UB_THIRDPARTY_INTERACTIVE.files = $$files($$THIRD_PARTY_PATH/interactive/*)
   UB_THIRDPARTY_INTERACTIVE.path = "$$RESOURCES_DIR/library/interactive"
   UB_MACX_ICNS.files = $$files(resources/macx/*.icns)
   UB_MACX_ICNS.path = "$$RESOURCES_DIR"
   UB_MACX_EXTRAS.files = "resources/macx/Save PDF to Open-Sankore.workflow"
   UB_MACX_EXTRAS.path = "$$RESOURCES_DIR"
   UB_I18N.path = $$DESTDIR/i18n # not used

   exists(resources/i18n/sankore_en.qm) {
       TRANSLATION_en.files = resources/i18n/sankore_en.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_en.path = "$$RESOURCES_DIR/en.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_en
   }
   exists(resources/i18n/sankore_en_UK.qm) {
       TRANSLATION_en_UK.files = resources/i18n/sankore_en_UK.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_en_UK.path = "$$RESOURCES_DIR/en_UK.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_en_UK
   }
   exists(resources/i18n/sankore_fr.qm) {
       TRANSLATION_fr.files = resources/i18n/sankore_fr.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_fr.path = "$$RESOURCES_DIR/fr.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_fr
   }
   exists(resources/i18n/sankore_fr_CH.qm) {
       TRANSLATION_fr_CH.files = resources/i18n/sankore_fr_CH.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_fr_CH.path = "$$RESOURCES_DIR/fr_CH.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_fr_CH
   }
   exists(resources/i18n/sankore_de.qm) {
       TRANSLATION_de.files = resources/i18n/sankore_de.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_de.path = "$$RESOURCES_DIR/de.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_de
   }
   exists(resources/i18n/sankore_nl.qm) {
       TRANSLATION_nl.files = resources/i18n/sankore_nl.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_nl.path = "$$RESOURCES_DIR/nl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_nl
   }
   exists(resources/i18n/sankore_es.qm) {
       TRANSLATION_es.files = resources/i18n/sankore_es.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_es.path = "$$RESOURCES_DIR/es.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_es
   }
   exists(resources/i18n/sankore_it.qm) {
       TRANSLATION_it.files = resources/i18n/sankore_it.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_it.path = "$$RESOURCES_DIR/it.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_it
   }
   exists(resources/i18n/sankore_pl.qm) {
       TRANSLATION_pl.files = resources/i18n/sankore_pl.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pl.path = "$$RESOURCES_DIR/pl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pl
   }
   exists(resources/i18n/sankore_ru.qm) {
       TRANSLATION_ru.files = resources/i18n/sankore_ru.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ru.path = "$$RESOURCES_DIR/ru.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ru
   }
   exists(resources/i18n/sankore_da.qm) {
       TRANSLATION_da.files = resources/i18n/sankore_da.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_da.path = "$$RESOURCES_DIR/da.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_da
   }
   exists(resources/i18n/sankore_nb.qm) {
       TRANSLATION_nb.files = resources/i18n/sankore_nb.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_nb.path = "$$RESOURCES_DIR/nb.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_nb
   }
   exists(resources/i18n/sankore_sv.qm) {
       TRANSLATION_sv.files = resources/i18n/sankore_sv.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_sv.path = "$$RESOURCES_DIR/sv.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_sv
   }
   exists(resources/i18n/sankore_ja.qm) {
       TRANSLATION_ja.files = resources/i18n/sankore_ja.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ja.path = "$$RESOURCES_DIR/ja.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ja
   }
   exists(resources/i18n/sankore_ko.qm) {
       TRANSLATION_ko.files = resources/i18n/sankore_ko.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ko.path = "$$RESOURCES_DIR/ko.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ko
   }
   exists(resources/i18n/sankore_zh.qm) {
       TRANSLATION_zh.files = resources/i18n/sankore_zh.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_zh.path = "$$RESOURCES_DIR/zh.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_zh
   }
   exists(resources/i18n/sankore_zh_CN.qm) {
       TRANSLATION_zh_CN.files = resources/i18n/sankore_zh_CN.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_zh_CN.path = "$$RESOURCES_DIR/zh_CN.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_zh_CN
   }
   exists(resources/i18n/sankore_zh_TW.qm) {
       TRANSLATION_zh_TW.files = resources/i18n/sankore_zh_TW.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_zh_TW.path = "$$RESOURCES_DIR/zh_TW.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_zh_TW
   }
   exists(resources/i18n/sankore_ro.qm) {
       TRANSLATION_ro.files = resources/i18n/sankore_ro.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ro.path = "$$RESOURCES_DIR/ro.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ro
   }
   exists(resources/i18n/sankore_ar.qm) {
       TRANSLATION_ar.files = resources/i18n/sankore_ar.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ar.path = "$$RESOURCES_DIR/ar.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ar
   }
   exists(resources/i18n/sankore_iw.qm) {
       TRANSLATION_iw.files = resources/i18n/sankore_iw.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_iw.path = "$$RESOURCES_DIR/iw.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_iw
   }
   exists(resources/i18n/sankore_pt.qm) {
       TRANSLATION_pt.files = resources/i18n/sankore_pt.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pt.path = "$$RESOURCES_DIR/pt.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pt
   }
   exists(resources/i18n/sankore_pt_BR.qm) {
       TRANSLATION_pt_BR.files = resources/i18n/sankore_pt_BR.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_pt_BR.path = "$$RESOURCES_DIR/pt_BR.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_pt_BR
   }
   exists(resources/i18n/sankore_sk.qm) {
       TRANSLATION_sk.files = resources/i18n/sankore_sk.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_sk.path = "$$RESOURCES_DIR/sk.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_sk
   }
   exists(resources/i18n/sankore_bg.qm) {
       TRANSLATION_bg.files = resources/i18n/sankore_bg.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_bg.path = "$$RESOURCES_DIR/bg.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_bg
   }
   exists(resources/i18n/sankore_ca.qm) {
       TRANSLATION_ca.files = resources/i18n/sankore_ca.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_ca.path = "$$RESOURCES_DIR/ca.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_ca
   }
   exists(resources/i18n/sankore_el.qm) {
       TRANSLATION_el.files = resources/i18n/sankore_el.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_el.path = "$$RESOURCES_DIR/el.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_el
   }
   exists(resources/i18n/sankore_tr.qm) {
       TRANSLATION_tr.files = resources/i18n/sankore_tr.qm \
           resources/i18n/Localizable.strings
       TRANSLATION_tr.path = "$$RESOURCES_DIR/tr.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_tr
   }
   exists(resources/i18n/sankore_cs.qm) {
       TRANSLATION_cs.files = resources/i18n/sankore_cs.qm \
           resources/i18n/localizable.strings
       TRANSLATION_cs.path = "$$RESOURCES_DIR/cs.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_cs
   }
   exists(resources/i18n/sankore_mg.qm) {
       TRANSLATION_mg.files = resources/i18n/sankore_mg.qm \
           resources/i18n/localizable.strings
       TRANSLATION_mg.path = "$$RESOURCES_DIR/mg.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_mg
   }
   exists(resources/i18n/sankore_hi.qm) {
       TRANSLATION_hi.files = resources/i18n/sankore_hi.qm \
           resources/i18n/localizable.strings
       TRANSLATION_hi.path = "$$RESOURCES_DIR/hi.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_hi
   }
   exists(resources/i18n/sankore_co.qm) {
       TRANSLATION_co.files = resources/i18n/sankore_co.qm \
           resources/i18n/localizable.strings
       TRANSLATION_co.path = "$$RESOURCES_DIR/co.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_co
   }
   exists(resources/i18n/sankore_eu.qm) {
       TRANSLATION_eu.files = resources/i18n/sankore_eu.qm \
           resources/i18n/localizable.strings
       TRANSLATION_eu.path = "$$RESOURCES_DIR/eu.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_eu
   }
   exists(resources/i18n/sankore_bm.qm) {
       TRANSLATION_bm.files = resources/i18n/sankore_bm.qm \
           resources/i18n/localizable.strings
       TRANSLATION_bm.path = "$$RESOURCES_DIR/bm.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_bm
   }
   exists(resources/i18n/sankore_gl.qm) {
       TRANSLATION_gl.files = resources/i18n/sankore_gl.qm \
           resources/i18n/localizable.strings
       TRANSLATION_gl.path = "$$RESOURCES_DIR/gl.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_gl
   }
   exists(resources/i18n/sankore_oc.qm) {
       TRANSLATION_oc.files = resources/i18n/sankore_oc.qm \
           resources/i18n/localizable.strings
       TRANSLATION_oc.path = "$$RESOURCES_DIR/oc.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_oc
   }
   exists(resources/i18n/sankore_fi.qm) {
       TRANSLATION_fi.files = resources/i18n/sankore_fi.qm \
           resources/i18n/localizable.strings
       TRANSLATION_fi.path = "$$RESOURCES_DIR/fi.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_fi
   }
   exists(resources/i18n/sankore_hu.qm) {
       TRANSLATION_hu.files = resources/i18n/sankore_hu.qm \
           resources/i18n/localizable.strings
       TRANSLATION_hu.path = "$$RESOURCES_DIR/hu.lproj"
       QMAKE_BUNDLE_DATA += TRANSLATION_hu
   }

   QMAKE_BUNDLE_DATA += UB_ETC \
       UB_LIBRARY \
       UB_FONTS \
       UB_THIRDPARTY_INTERACTIVE \
       UB_MACX_ICNS \
       UB_MACX_EXTRAS \
       SPARKLE_KEY \
       FRAMEWORKS

   QMAKE_PKGINFO_TYPEINFO = "UniB"
   QMAKE_INFO_PLIST = "resources/macx/Info.plist"

   # For packger (release.macx.sh script) to know Uniboard version
   system(mkdir -p $$BUILD_DIR)
   system(printf \""$$OSX_VERSION"\" > $$BUILD_DIR/osx_version)
   system(printf \""$$VERSION"\" > $$BUILD_DIR/version)
   system(printf "%02x%02x%02x%02x" `printf $$VERSION_RC | cut -d ',' -f 1` `printf $$VERSION_RC | cut -d ',' -f 2` `printf $$VERSION_RC | cut -d ',' -f 3` `printf $$VERSION_RC | cut -d ',' -f 4` | xxd -r -p > "$$VERSION_RC_PATH")
}

linux-* {
    CONFIG += link_prl
    LIBS += -lcrypto
    LIBS += -lX11
    QMAKE_CFLAGS += -fopenmp
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
    UB_LIBRARY.path = $$DESTDIR
    UB_I18N.path = $$DESTDIR/i18n
    UB_ETC.path = $$DESTDIR
    UB_THIRDPARTY_INTERACTIVE.path = $$DESTDIR/library
    system(mkdir -p $$BUILD_DIR)
    system(echo "$$VERSION" > $$BUILD_DIR/version)
    system(echo "$$LONG_VERSION" > $$BUILD_DIR/longversion)
    system(echo "$$SVN_VERSION" > $$BUILD_DIR/svnversion)

    linux-clang {
        QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas
        QMAKE_CXXFLAGS_WARN_ON += -Wno-overloaded-virtual
    }
}

RESOURCES += resources/sankore.qrc
RESOURCES += plugins/cffadaptor/resources/resources.qrc

# When adding a translation here, also add it in the macx part
TRANSLATIONS = resources/i18n/sankore_en.ts \
   resources/i18n/sankore_en_UK.ts \
   resources/i18n/sankore_fr.ts \
   resources/i18n/sankore_fr_CH.ts \
   resources/i18n/sankore_de.ts \
   resources/i18n/sankore_nl.ts \
   resources/i18n/sankore_es.ts \
   resources/i18n/sankore_it.ts \
   resources/i18n/sankore_pl.ts \
   resources/i18n/sankore_ru.ts \
   resources/i18n/sankore_da.ts \
   resources/i18n/sankore_nb.ts \
   resources/i18n/sankore_sv.ts \
   resources/i18n/sankore_ja.ts \
   resources/i18n/sankore_ko.ts \
   resources/i18n/sankore_zh.ts \
   resources/i18n/sankore_zh_CN.ts \
   resources/i18n/sankore_zh_TW.ts \
   resources/i18n/sankore_ro.ts \
   resources/i18n/sankore_ar.ts \
   resources/i18n/sankore_iw.ts \
   resources/i18n/sankore_pt.ts \
   resources/i18n/sankore_pt_BR.ts \
   resources/i18n/sankore_sk.ts \
   resources/i18n/sankore_bg.ts \
   resources/i18n/sankore_ca.ts \
   resources/i18n/sankore_el.ts \
   resources/i18n/sankore_tr.ts \
   resources/i18n/sankore_cs.ts \
   resources/i18n/sankore_mg.ts \
   resources/i18n/sankore_hi.ts \
   resources/i18n/sankore_co.ts \
   resources/i18n/sankore_eu.ts \
   resources/i18n/sankore_bm.ts \
   resources/i18n/sankore_oc.ts \
   resources/i18n/sankore_gl.ts \
   resources/i18n/sankore_fi.ts \
   resources/i18n/sankore_hu.ts


INSTALLS = UB_ETC \
   UB_I18N \
   UB_LIBRARY \
   UB_THIRDPARTY_INTERACTIVE

OTHER_FILES += \
    resources/library/shape/fleche blanche.svg \
    resources/library/shape/fleche bleue.svg \
    resources/library/shape/fleche grise.svg \
    resources/library/shape/fleche rouge.svg \
    resources/library/shape/fleche vide.svg

