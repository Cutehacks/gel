TEMPLATE = app
TARGET = tst_gel
CONFIG += warn_on qmltestcase
SOURCES += tst_gel.cpp
OTHER_FILES += *.qml

include($$PWD/../com_cutehacks_gel.pri)
