HEADERS       = \
    chromelistener.h
SOURCES       = \
                main.cpp \
    chromelistener.cpp
QT           += network widgets

INCLUDEPATH += /usr/local/Cellar/boost/1.64.0_1/include

LIBS += -L/usr/local/Cellar/boost/1.64.0_1/lib
LIBS += -lboost_system-mt

INSTALLS += target

DISTFILES +=
