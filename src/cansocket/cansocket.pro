TEMPLATE = lib
TARGET = $$qtLibraryTarget(QtCanSocket)
QT += core
    
PUBLIC_HEADERS += qcanmessage.h \
    qcansocket.h
    
HEADERS += $$PUBLIC_HEADERS \
    qcansocket_p.h 
SOURCES += qcansocket.cpp
OBJECTS_DIR = ../../obj
DESTDIR = ../../lib
MOC_DIR = ../../moc
headers.files = $$PUBLIC_HEADERS
headers.path = $$[QT_INSTALL_PREFIX]/include/QtAddOn
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target \
    headers
CONFIG += create_pc \
    create_prl \
    no_install_prl
QMAKE_PKGCONFIG_NAME = QtCanSocket
QMAKE_PKGCONFIG_DESCRIPTION = SocketCan implementation for Qt
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$target.path
QMAKE_PKGCONFIG_DESTDIR = ../pkgconfig

QMAKE_CLEAN += -r $$DESTDIR $$OBJECTS_DIR $$MOC_DIR $$QMAKE_PKGCONFIG_DESTDIR ../$$QMAKE_PKGCONFIG_DESTDIR
