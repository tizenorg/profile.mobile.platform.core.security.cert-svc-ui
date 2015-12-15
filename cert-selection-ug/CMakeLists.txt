ADD_DEFINITIONS("-DBUILD_CERT_SELECTION_UG")

pkg_check_modules(ug-cert-selection-ug-efl_pkgs
    REQUIRED
    glib-2.0
    dlog
    evas
    elementary
    efl-extension
    ui-gadget-1
    cert-svc-vcore
)

SET(SRCS
    ${CMAKE_SOURCE_DIR}/common/src/common-utils.c
    src/cert-selection.c
    src/cert-selection-uigadget.c
)

FOREACH(flag ${ug-cert-selection-ug-efl_pkgs_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/cert-selection-ug/include
    ${CMAKE_SOURCE_DIR}/view/include
    ${ug-cert-selection-ug-efl_pkgs_INCLUDE_DIRS}
)

ADD_LIBRARY(${TARGET_CERT_SELECTION_UG} SHARED ${SRCS})
SET_TARGET_PROPERTIES(
    ${TARGET_CERT_SELECTION_UG}
    PROPERTIES
        COMPILE_FLAGS "-D_GNU_SOURCE -fPIC -fvisibility=default"
        SOVERSION ${SO_VERSION}
        VERSION ${VERSION}
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_SELECTION_UG}
    ${ug-cert-selection-ug-efl_pkgs_LIBRARIES}
    ${ug-cert-selection-ug-efl_pkgs_LDFLAGS}
)

INSTALL(TARGETS
    ${TARGET_CERT_SELECTION_UG}
    DESTINATION
    ${LIBDIR}
)
