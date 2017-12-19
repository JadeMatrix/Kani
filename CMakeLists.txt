CMAKE_MINIMUM_REQUIRED( VERSION 3.9 )


SET(    ORG_NAME      "JadeMatrix"            )
SET( AUTHOR_NAME      "Joseph Durel"          )
SET( BUNDLE_NAME      "Kani"                  )
SET( BUNDLE_VER_NUM   "0.0"                   )
SET( BUNDLE_VER_NAME  "alpha"                 )
SET( BUNDLE_COPYRIGHT "© 2017 ${AUTHOR_NAME}" )


PROJECT( ${BUNDLE_NAME} )

SET( CMAKE_CXX_STANDARD          11               )
SET( CMAKE_CXX_STANDARD_REQUIRED ON               )
SET( CMAKE_C_FLAGS               "-x objective-c" )


ADD_EXECUTABLE( ${BUNDLE_NAME} MACOSX_BUNDLE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/main.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/AppDelegate.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/KaniMainView.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/KaniMainToolbarDelegate.mm"
    "Resources/${BUNDLE_NAME}.icns"
)
SET_TARGET_PROPERTIES( ${BUNDLE_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST           "Info.plist.in"
    MACOSX_BUNDLE_INFO_STRING          "${BUNDLE_NAME} v${BUNDLE_VER_NUM} (${BUNDLE_VER_NAME}) ${BUNDLE_COPYRIGHT}"
    MACOSX_BUNDLE_ICON_FILE            "${BUNDLE_NAME}.icns"
    MACOSX_BUNDLE_GUI_IDENTIFIER       "com.${ORG_NAME}.${BUNDLE_NAME}"
    MACOSX_BUNDLE_LONG_VERSION_STRING  "${BUNDLE_NAME} v${BUNDLE_VER_NUM} (${BUNDLE_VER_NAME})"
    MACOSX_BUNDLE_BUNDLE_NAME          "${BUNDLE_NAME}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${BUNDLE_VER_NAME}"
    MACOSX_BUNDLE_BUNDLE_VERSION       "${BUNDLE_VER_NUM}"
    MACOSX_BUNDLE_COPYRIGHT            "${BUNDLE_COPYRIGHT}"
)

TARGET_LINK_LIBRARIES( ${BUNDLE_NAME}
    "-framework Cocoa"
)


FIND_PROGRAM(
    INKSCAPE_PROGRAM
    inkscape
    PATHS $ENV{HOME}/Applications/Inkscape.app/Contents/Resources/bin /Applications/Inkscape.app/Contents/Resources/bin
    # CMake wants to prefer Inkscape.app/Contents/MacOS/Inkscape, which won't
    # work on OS X -- need to use Inkscape.app/Contents/Resources/bin/inkscape
    NO_DEFAULT_PATH
)
FIND_PROGRAM(
    ICONUTIL_PROGRAM
    iconutil
)


ADD_CUSTOM_COMMAND(
    OUTPUT "Resources/${BUNDLE_NAME}.icns"
    COMMAND ${CMAKE_COMMAND} -E make_directory "Resources/${BUNDLE_NAME}.iconset/"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_16x16@1x.png" --export-dpi 96 --export-area=512:480:528:496 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_16x16@2x.png" --export-dpi 192 --export-area=512:480:528:496 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_32x32@1x.png" --export-dpi 96 --export-area=512:448:544:480 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_32x32@2x.png" --export-dpi 192 --export-area=512:448:544:480 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_128x128@1x.png" --export-dpi 96 --export-area=512:256:640:384 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_128x128@2x.png" --export-dpi 192 --export-area=512:256:640:384 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_256x256@1x.png" --export-dpi 96 --export-area=512:0:768:256 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_256x256@2x.png" --export-dpi 192 --export-area=512:0:768:256 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_512x512@1x.png" --export-dpi 96 --export-area=0:0:512:512 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${INKSCAPE_PROGRAM} --without-gui --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_512x512@2x.png" --export-dpi 192 --export-area=0:0:512:512 "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    COMMAND ${ICONUTIL_PROGRAM} -c icns -o "${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.icns" "${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset"
    DEPENDS "Resources/Kani.svg"
)

SET_SOURCE_FILES_PROPERTIES(
    "Resources/${BUNDLE_NAME}.icns"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)


CONFIGURE_FILE(
    "Resources/Info.plist"
    "Info.plist.in"
    COPYONLY
)
