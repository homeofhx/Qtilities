QT += widgets svg
unix:!macx: QT += dbus
CONFIG += c++17
TARGET = Qtilities
TEMPLATE = app
RESOURCES += resources.qrc

# OS specific
win32: LIBS += -luser32

macx {
    LIBS += -framework CoreFoundation -framework IOKit -framework AppKit -framework CoreGraphics
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_CXXFLAGS += -include arm_acle.h
    OBJECTIVE_SOURCES += RTC/macos_window.mm
}

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # Disables all APIs deprecated before Qt6. Use if needed.

SOURCES += \
    main.cpp \
    app.cpp \
    awake/awake.cpp \
    RTC/clockwindow.cpp

HEADERS += \
    app.h \
    awake/awake.h \
    RTC/clockwindow.h \
    RTC/macos_window.h

# Deployment rules
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=