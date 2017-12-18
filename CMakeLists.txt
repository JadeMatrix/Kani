cmake_minimum_required( VERSION 3.9 )


SET(    ORG_NAME      "JadeMatrix"            )
SET( AUTHOR_NAME      "Joseph Durel"          )
SET( BUNDLE_NAME      "Kani"                  )
SET( BUNDLE_VER_NUM   "0.0"                   )
SET( BUNDLE_VER_NAME  "alpha"                 )
SET( BUNDLE_COPYRIGHT "© 2017 ${AUTHOR_NAME}" )


project( ${BUNDLE_NAME} )

set( CMAKE_CXX_STANDARD          11               )
set( CMAKE_CXX_STANDARD_REQUIRED ON               )
set( CMAKE_C_FLAGS               "-x objective-c" )


add_executable( ${BUNDLE_NAME} MACOSX_BUNDLE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/main.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/AppDelegate.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/KaniMainView.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/KaniMainToolbarDelegate.mm"
    "Resources/${BUNDLE_NAME}.icns"
)
set_target_properties( ${BUNDLE_NAME} PROPERTIES
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

target_link_libraries( ${BUNDLE_NAME}
    "-framework Cocoa"
)


find_program(
    INKSCAPE_PROGRAM
    inkscape
    PATHS $ENV{HOME}/Applications/Inkscape.app/Contents/Resources/bin /Applications/Inkscape.app/Contents/Resources/bin
    # CMake wants to prefer Inkscape.app/Contents/MacOS/Inkscape, which won't
    # work on OS X -- need to use Inkscape.app/Contents/Resources/bin/inkscape
    NO_DEFAULT_PATH
)
find_program(
    ICONUTIL_PROGRAM
    iconutil
)


add_custom_command(
    OUTPUT "Resources/${BUNDLE_NAME}.iconset"
    COMMAND ${CMAKE_COMMAND} -E make_directory "Resources/${BUNDLE_NAME}.iconset/"
)
add_custom_command(
    OUTPUT "${BUNDLE_NAME}.iconset/icon_512x512@2x.png"
    COMMAND ${INKSCAPE_PROGRAM} --export-png="${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset/icon_512x512@2x.png" --export-area-page "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${BUNDLE_NAME}.svg"
    DEPENDS "Resources/${BUNDLE_NAME}.iconset"
)
add_custom_command(
    OUTPUT "Resources/${BUNDLE_NAME}.icns"
    COMMAND ${ICONUTIL_PROGRAM} -c icns -o "${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.icns" "${CMAKE_CURRENT_BINARY_DIR}/Resources/${BUNDLE_NAME}.iconset"
    DEPENDS "${BUNDLE_NAME}.iconset/icon_512x512@2x.png"
)

set_source_files_properties(
    "Resources/${BUNDLE_NAME}.icns"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)


configure_file(
    "Resources/Info.plist"
    "Info.plist.in"
    COPYONLY
)
