pkg_check_modules(ug-setting-manage-certificates-efl_pkgs
    REQUIRED
    dlog
    glib-2.0
    ui-gadget-1
    evas
    elementary
    efl-extension
    cert-svc-vcore
)

SET(SRCS
    src/mgr-app-uigadget.c
)

FOREACH(flag ${ug-setting-manage-certificates-efl_pkgs_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ug/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/view/include)
INCLUDE_DIRECTORIES(${ug-setting-manage-certificates-efl_pkgs_INCLUDE_DIRS})

ADD_DEFINITIONS(${ug-setting-manage-certificates-efl_pkgs_DEPS_CFLAGS})
ADD_DEFINITIONS(${ug-setting-manage-certificates-efl_pkgs_DEPS_OTHER})

ADD_LIBRARY(${TARGET_CERT_MANAGER_UG} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${TARGET_CERT_MANAGER_UG}
    PROPERTIES
        SOVERSION ${SO_VERSION}
        VERSION ${VERSION}
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_MANAGER_UG}
    ${ug-setting-manage-certificates-efl_pkgs_LIBRARIES}
    ${ug-setting-manage-certificates-efl_pkgs_LDFLAGS}
    ${TARGET_CERT_VIEW}
)

INSTALL(TARGETS
    ${TARGET_CERT_MANAGER_UG}
    DESTINATION
    ${LIBDIR}
)
