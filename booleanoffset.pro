QT += widgets
CONFIG += debug

TARGET = BooleanOffset

RESOURCES = booleanoffset.qrc

HEADERS += \
	src/engine/global.h \
	src/engine/vertex.h \
	src/engine/terminal.h \
	src/engine/primitive.h \
	src/engine/line.h \
	src/engine/arc.h \
	src/engine/shape.h \
	src/engine/core.h \
	src/Actions.h \
	src/GeometryPlot.h \
	src/MainWindow.h

SOURCES += \
	src/engine/vertex.cpp \
	src/engine/terminal.cpp \
	src/engine/primitive.cpp \
	src/engine/line.cpp \
	src/engine/arc.cpp \
	src/engine/shape.cpp \
	src/Actions.cpp \
	src/GeometryPlot.cpp \
	src/MainWindow.cpp \
	src/main.cpp
