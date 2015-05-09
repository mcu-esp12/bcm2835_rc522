TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    MFRC522.cpp

HEADERS += \
    MFRC522.h

LIBS +=-L/usr/local/lib -lbcm2835

