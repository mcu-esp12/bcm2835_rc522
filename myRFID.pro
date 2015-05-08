TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    rc522.c \
    rfid.c \
    MFRC522.cpp

HEADERS += \
    rc522.h \
    rfid.h \
    MFRC522.h

LIBS +=-L/usr/local/lib -lbcm2835

