CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-myfile-efl C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

# subdir
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(src/common)
ADD_SUBDIRECTORY(src/media-db)
ADD_SUBDIRECTORY(src/widget)
ADD_SUBDIRECTORY(res/edje)
ADD_SUBDIRECTORY(res/edje/icons)
ADD_SUBDIRECTORY(res/edje/images)
ADD_SUBDIRECTORY(res/po)
ADD_SUBDIRECTORY(res/edje/sounds)

CONFIGURE_FILE(ug-myfile-efl.manifest-3.0.in ${PKGNAME}.manifest)



# EDJ
ADD_CUSTOM_COMMAND(OUTPUT ug_navibar_layout.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/images
				-sd  ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/sounds
                ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/ug_navibar_layout.edc
				${CMAKE_BINARY_DIR}/ug_navibar_layout.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/ug_navibar_layout.edc
)

ADD_CUSTOM_COMMAND(OUTPUT ug_edc_image_macro.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/icons
		${CMAKE_CURRENT_SOURCE_DIR}/res/edje/ug_edc_image_macro.edc ${CMAKE_BINARY_DIR}/ug_edc_image_macro.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/ug_edc_image_macro.edc
)

ADD_CUSTOM_TARGET(myfile_efl_edj_build DEPENDS ug_navibar_layout.edj ug_edc_image_macro.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} myfile_efl_edj_build)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/res/edje)

INSTALL(FILES ${CMAKE_BINARY_DIR}/ug_navibar_layout.edj DESTINATION ${RESDIR}/edje)
INSTALL(FILES ${CMAKE_BINARY_DIR}/ug_edc_image_macro.edj DESTINATION ${RESDIR}/edje)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/images DESTINATION ${RESDIR}/edje)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/icons DESTINATION ${RESDIR}/edje)

INSTALL(FILES ug-myfile-efl.manifest.xml DESTINATION ${MANIFESTDIR} RENAME ${PKGNAME}.xml)
INSTALL(FILES shared/res/ug-myfile-efl.png DESTINATION ${ICONDIR})
#INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/edje/images/ DESTINATION ${IMGDIR})
INSTALL(FILES ${PROJECT_NAME}.xml DESTINATION ${MANIFESTDIR})


# Install appcontrol icon
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/shared/res/ug-myfile-efl.png DESTINATION ${ICONDIR})
