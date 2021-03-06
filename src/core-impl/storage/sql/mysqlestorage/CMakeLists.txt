add_definitions(${MYSQL_EMBEDDED_CFLAGS})

########### mysqle ###############

set( amarok_storage-mysqlestorage_PART_SRCS
     ../mysql-shared/MySqlStorage.cpp
     MySqlEmbeddedStorage.cpp
     MySqlEmbeddedStorageFactory.cpp
)

kde4_add_plugin(amarok_storage-mysqlestorage ${amarok_storage-mysqlestorage_PART_SRCS})

STRING(REPLACE "-Wl,--fatal-warnings" "" CMAKE_SHARED_LINKER_FLAGS_NOFATALWARN "${CMAKE_SHARED_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS_NOFATALWARN} )

STRING(REPLACE "-Wl,--fatal-warnings" "" CMAKE_MODULE_LINKER_FLAGS_NOFATALWARN "${CMAKE_MODULE_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS_NOFATALWARN} )

target_link_libraries(amarok_storage-mysqlestorage
    # amarok-sqlstorage
    amarokcore
    amaroklib
    ${KDE4_KDECORE_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${MYSQL_EMBEDDED_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${ZLIB_LIBRARIES}
)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries( amarok_storage-mysqlestorage crypt pthread )
endif(NOT WIN32 AND NOT APPLE)

if(APPLE)
	SET_TARGET_PROPERTIES(amarok_storage-mysqlestorage PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

install(TARGETS amarok_storage-mysqlestorage DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES amarok_storage-mysqlestorage.desktop DESTINATION ${SERVICES_INSTALL_DIR})
