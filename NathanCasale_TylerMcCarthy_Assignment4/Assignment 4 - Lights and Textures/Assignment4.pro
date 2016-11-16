QT += core
QT += gui widgets xml

CONFIG += c++11

TARGET = Assignment4
CONFIG += console
CONFIG -= app_bundle

INCLUDEPATH += ../headers

TEMPLATE = app

SOURCES += main.cpp \
    OpenGLWindow.cpp \
    View.cpp

HEADERS += \
    OpenGLWindow.h \
    VertexAttrib.h \
    View.h \
    ../headers/sgraph/AbstractNode.h \
    ../headers/sgraph/GLScenegraphRenderer.h \
    ../headers/sgraph/GroupNode.h \
    ../headers/sgraph/INode.h \
    ../headers/sgraph/IScenegraph.h \
    ../headers/sgraph/LeafNode.h \
    ../headers/sgraph/Scenegraph.h \
    ../headers/sgraph/scenegraphinfo.h \
    ../headers/sgraph/SceneXMLReader.h \
    ../headers/sgraph/TransformNode.h \
    lightlocation.h
