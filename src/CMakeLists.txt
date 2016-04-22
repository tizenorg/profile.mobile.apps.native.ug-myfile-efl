CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-myfile-efl C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(LIBDIR "${PREFIX}/lib")

SET(VERSION 0.1.0)

SET(SRCS
	mf-ug-main.c
	common/mf-ug-util.c
	common/mf-ug-file-util.c
	common/mf-ug-cb.c
	common/mf-ug-music.c
	common/mf-ug-fm-svc-wapper.c
	common/mf-ug-inotify-handle.c
	common/mf-ug-list-play.c
	common/mf-ug-search-internal.c
	common/mf-ug-search.c
	common/mf-ug-db-handle.c
	widget/mf-ug-navi-bar.c
	widget/mf-ug-genlist.c
	widget/mf-ug-popup.c
	widget/mf-ug-widget.c
	widget/mf-ug-search-view.c
	widget/mf-ug-ringtone-view.c
	common/file-system/mf-ug-file-attr.c
	common/file-system/mf-ug-fs-oper.c
	media-db/mf-ug-media.c
        media-db/mf-ug-media-db.c
)


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc/)



INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	ecore-evas
	elementary
	glib-2.0
	capi-appfw-application
	capi-media-metadata-extractor
	capi-system-system-settings
	capi-content-media-content
	dlog
	capi-media-sound-manager
	capi-media-player
	capi-system-device
	capi-system-runtime-info
	notification
	appsvc
	sqlite3
	storage
	capi-content-mime-type
	efl-extension
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -fvisibility=hidden -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DUG_OPERATION_SELECT_MODE")

#Set executable
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

# install binary
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
