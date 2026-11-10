QT += widgets svg multimedia
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
    countdown/countdowntimer.cpp \
    countdown/countdowndisplay.cpp \
    countdown/countdownsetupdialog.cpp \
    floating/floatinglabel.cpp \
    floating/floatingdisplaywindow.cpp \
    floating/floatingdisplaymanager.cpp \
    RTC/chimerenderer.cpp \
    RTC/chime.cpp

HEADERS += \
    app.h \
    awake/awake.h \
    countdown/countdowntimer.h \
    countdown/countdowndisplay.h \
    countdown/countdownsetupdialog.h \
    floating/floatinglabel.h \
    floating/floatingdisplaywindow.h \
    floating/floatingdisplaymanager.h \
    RTC/macos_window.h \
    RTC/chimerenderer.h \
    RTC/chime.h

# Deployment rules
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target