# Configuration ################################################################


AUTHOR_NAME=JadeMatrix
BUNDLE_NAME=Kani

CWD := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))


# Source Dependencies ##########################################################


# Source/main.mm: Source/AppDelegate.hh

# Source/AppDelegate.mm: Source/AppDelegate.hh Source/KaniToolbarDelegate.hh Source/KaniMainView.hh

# Source/KaniMainView.mm: Source/KaniMainView.hh

# Source/KaniMainToolbarDelegate.mm: Source/KaniMainToolbarDelegate.hh


# Object Files #################################################################


MAIN_OBJECTS = \
	Make/Object/main.o \
	Make/Object/AppDelegate.o \
	Make/Object/KaniMainView.o \
	Make/Object/KaniMainToolbarDelegate.o

Make/Object/%.o: Source/%.mm
	@mkdir -p "Make/Object"
	@clang++ -std=c++11 -Wall -o $@ -c $<


# Executables ##################################################################


Make/${BUNDLE_NAME}: ${MAIN_OBJECTS}
	@mkdir -p "Make"
	@clang++ -std=c++11 -Wall -framework Cocoa -lobjc -o "Make/${BUNDLE_NAME}" $^


# Resources ####################################################################


Make/Resources/${BUNDLE_NAME}.iconset/icon_512x512@2x.png: Resources/${BUNDLE_NAME}.svg
	@mkdir -p "Make/Resources/${BUNDLE_NAME}.iconset"
	@~/Applications/Inkscape.app/Contents/Resources/bin/inkscape --export-png="${CWD}/Make/Resources/${BUNDLE_NAME}.iconset/icon_512x512@2x.png" --export-area-page "${CWD}/Resources/${BUNDLE_NAME}.svg"

Make/Resources/${BUNDLE_NAME}.icns: Make/Resources/${BUNDLE_NAME}.iconset/icon_512x512@2x.png
	@mkdir -p "Make/Resources"
	@iconutil -c icns -o Make/Resources/${BUNDLE_NAME}.icns Make/Resources/${BUNDLE_NAME}.iconset


# Packages #####################################################################


Make/Info.plist: Resources/Info.plist
	@mkdir -p "Make"
	@sed 's/AUTHOR_NAME/${AUTHOR_NAME}/' "Resources/Info.plist" | sed 's/BUNDLE_NAME/${BUNDLE_NAME}/' > "Make/Info.plist"

Make/${BUNDLE_NAME}.app: Make/${BUNDLE_NAME} Make/Resources/${BUNDLE_NAME}.icns Make/Info.plist
	@mkdir -p "Make/${BUNDLE_NAME}.app/Contents/MacOS"
	@mkdir -p "Make/${BUNDLE_NAME}.app/Contents/Resources"
	@cp "Make/Info.plist" "Make/${BUNDLE_NAME}.app/Contents/Info.plist"
	@cp "Make/Resources/${BUNDLE_NAME}.icns" "Make/${BUNDLE_NAME}.app/Contents/Resources/${BUNDLE_NAME}.icns"
	@cp "Make/${BUNDLE_NAME}" "Make/${BUNDLE_NAME}.app/Contents/MacOS/${BUNDLE_NAME}"


# Utility ######################################################################


all: Make/${BUNDLE_NAME}.app

clean:
	@rm -rf "Make"

.PHONY: all clean
