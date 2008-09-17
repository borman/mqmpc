qtopia_project(qtopia app)
TARGET=mqmpc
CONFIG+=qtopia_main no_quicklaunch

# Input
HEADERS += collection.h libmpdclient.h mainwidget.h ui_general.h qslidelabel.h
SOURCES += collection.cpp libmpdclient.c main.cpp mainwidget.cpp ui_general.cpp qslidelabel.cpp
TRANSLATIONS += mqmpc-ru_RU.ts

# Set this to trusted for full privileges
target.hint=sxe
target.domain=trusted

desktop.files=mqmpc.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/mqmpc
pics.hint=pics
INSTALLS+=pics

appicon.files=pics/mqmpc*
appicon.path=/pics/mqmpc
appicon.hint=pics
INSTALLS+=appicon

pkg.name=mqmpc
pkg.desc=Mobile Qt Media Player Control
pkg.version=1.1
pkg.maintainer=borman
pkg.license=GPL
pkg.domain=trusted
