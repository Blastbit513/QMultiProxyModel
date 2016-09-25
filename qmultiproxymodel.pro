contains(QT_VERSION, ^5\\..*) {
    QT  -= gui
}

TARGET = qmultiproxymodel
TEMPLATE = lib
CONFIG += staticlib

DEFINES += QMULTIPROXYMODEL_LIBRARY

SOURCES += qmultiproxymodel.cpp
HEADERS += qmultiproxymodel.h

unix {
    target.headers = /usr/include/qmultiproxymodel
    target.path = /usr/lib
    INSTALLS += target
}
