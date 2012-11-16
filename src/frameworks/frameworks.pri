
HEADERS      += $$CORE_SRC/frameworks/UBGeometryUtils.h \
                $$CORE_SRC/frameworks/UBPlatformUtils.h \
                $$CORE_SRC/frameworks/UBFileSystemUtils.h \
                $$CORE_SRC/frameworks/UBStringUtils.h \
                $$CORE_SRC/frameworks/UBVersion.h \
                $$CORE_SRC/frameworks/UBCryptoUtils.h \
                $$CORE_SRC/frameworks/UBBase32.h

SOURCES      += $$CORE_SRC/frameworks/UBGeometryUtils.cpp \
                $$CORE_SRC/frameworks/UBPlatformUtils.cpp \
                $$CORE_SRC/frameworks/UBFileSystemUtils.cpp \
                $$CORE_SRC/frameworks/UBStringUtils.cpp \
                $$CORE_SRC/frameworks/UBVersion.cpp \
                $$CORE_SRC/frameworks/UBCryptoUtils.cpp \
                $$CORE_SRC/frameworks/UBBase32.cpp


win32 {

    SOURCES  += $$CORE_SRC/frameworks/UBPlatformUtils_win.cpp
}            


macx {                

    OBJECTIVE_SOURCES  += $$CORE_SRC/frameworks/UBPlatformUtils_mac.mm
                
}       


linux-g++ {

    SOURCES  += $$CORE_SRC/frameworks/UBPlatformUtils_linux.cpp
}         
linux-g++-32 {

    SOURCES  += $$CORE_SRC/frameworks/UBPlatformUtils_linux.cpp
}
linux-g++-64 {

    SOURCES  += $$CORE_SRC/frameworks/UBPlatformUtils_linux.cpp
}
