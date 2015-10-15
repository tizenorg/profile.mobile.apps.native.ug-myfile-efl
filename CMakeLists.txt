IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
        SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-myfile-efl C)


####################################
########## SET Directory ###########
####################################
SET(MFUGNAME ug-myfile-efl)
SET(UGINCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
SET(UGLIBINCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/include)

SET(UG-SRC-DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(UG-UTILITY-DIR ${UG-SRC-DIR}/common)
SET(UG-FS-DIR ${UG-UTILITY-DIR}/file-system)
SET(UG-WIDGET-DIR ${UG-SRC-DIR}/widget)
SET(UG-MEDIADB-DIR ${UG-SRC-DIR}/media-db)

SET(PREFIX "/usr/ug")
SET(LIBDIR "${PREFIX}/lib")
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/${MFUGNAME}/locale")
SET(IMAGEDIR "${RESDIR}/images/${MFUGNAME}")
SET(EDJDIR "${RESDIR}/edje/${MFUGNAME}")


SET(SRCS
	${UG-SRC-DIR}/mf-ug-main.c
	${UG-UTILITY-DIR}/mf-ug-util.c
	${UG-UTILITY-DIR}/mf-ug-file-util.c
	${UG-UTILITY-DIR}/mf-ug-cb.c
	${UG-UTILITY-DIR}/mf-ug-music.c
	${UG-UTILITY-DIR}/mf-ug-fm-svc-wapper.c
	${UG-UTILITY-DIR}/mf-ug-inotify-handle.c
	${UG-UTILITY-DIR}/mf-ug-list-play.c
	${UG-UTILITY-DIR}/mf-ug-search-internal.c
	${UG-UTILITY-DIR}/mf-ug-search.c
	${UG-UTILITY-DIR}/mf-ug-db-handle.c
###########	winset wrap	##########
	${UG-WIDGET-DIR}/mf-ug-navi-bar.c
	${UG-WIDGET-DIR}/mf-ug-genlist.c
	${UG-WIDGET-DIR}/mf-ug-popup.c
	${UG-WIDGET-DIR}/mf-ug-widget.c
	${UG-WIDGET-DIR}/mf-ug-search-view.c
	${UG-WIDGET-DIR}/mf-ug-ringtone-view.c
################ file system #########
	${UG-FS-DIR}/mf-ug-file-attr.c
	${UG-FS-DIR}/mf-ug-fs-oper.c
#################### media db ######################
	${UG-MEDIADB-DIR}/mf-ug-media.c
        ${UG-MEDIADB-DIR}/mf-ug-media-db.c
	)
#START_PUBLIC_REMOVED_FILE
# Debian
#END_START_PUBLIC_REMOVED_FILE

INCLUDE_DIRECTORIES(${UGINCLUDE})
INCLUDE_DIRECTORIES(${UGLIBINCLUDE})
INCLUDE(FindPkgConfig)
pkg_check_modules(ug_pkgs REQUIRED
ecore-evas
elementary
glib-2.0
ui-gadget-1
capi-appfw-application
capi-media-metadata-extractor
capi-system-system-settings
capi-content-media-content
dlog
capi-media-sound-manager
capi-media-player
capi-system-device
#START_PUBLIC_REMOVED_STRING
#END_START_PUBLIC_REMOVED_STRING
notification
appsvc
sqlite3
storage
capi-content-mime-type
efl-extension
)

FOREACH(flag ${ug_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")


ADD_DEFINITIONS("-DLIST_PLAY_SUPPORT")
ADD_DEFINITIONS("-DUG_THUMB_REAL_TIME_UPDATE")
ADD_DEFINITIONS("-DMF_UG_FEATURE_NEW_UX")
ADD_DEFINITIONS("-DUG_OPERATION_SELECT_MODE")


ADD_LIBRARY(${MFUGNAME} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${MFUGNAME} ${ug_pkgs_LDFLAGS} dl)

message("MFUGNAME="${MFUGNAME} "LIBDIR"=${LIBDIR})
INSTALL(TARGETS ${MFUGNAME} LIBRARY DESTINATION ${LIBDIR})


# edje
ADD_CUSTOM_TARGET(ug_navibar_layout.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/image
		${EDJE_CFLAGS} -sd  ${CMAKE_CURRENT_SOURCE_DIR}/sounds
		${CMAKE_CURRENT_SOURCE_DIR}/ug_navibar_layout.edc
		${CMAKE_BINARY_DIR}/ug_navibar_layout.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ug_navibar_layout.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} ug_navibar_layout.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/ug_navibar_layout.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(ug_edc_image_macro.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/icons -id ${CMAKE_CURRENT_SOURCE_DIR}/image
		${CMAKE_CURRENT_SOURCE_DIR}/ug_edc_image_macro.edc
		${CMAKE_BINARY_DIR}/ug_edc_image_macro.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ug_edc_image_macro.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} ug_edc_image_macro.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/ug_edc_image_macro.edj DESTINATION ${EDJDIR})

# smat lib 608

# Install file

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ug-myfile-efl.xml DESTINATION /usr/share/packages)
# Appcontrl icon
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ug-myfile-efl.png DESTINATION /usr/share/icons/default/small)

# i18n
ADD_SUBDIRECTORY(res/po)

