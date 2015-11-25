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
    src/cert-selection.c
    src/cert-selection-uigadget.c
)

FOREACH(flag ${ug-cert-selection-ug-efl_pkgs_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/view/include)

ADD_DEFINITIONS(${ug-cert-selection-ug-efl_pkgs_DEPS_CFLAGS})
ADD_DEFINITIONS(${ug-cert-selection-ug-efl_pkgs_DEPS_OTHER})

INCLUDE_DIRECTORIES(${ug-cert-selection-ug-efl_pkgs_INCLUDE_DIRS})

ADD_LIBRARY(${TARGET_CERT_SELECTION_UG} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${TARGET_CERT_SELECTION_UG} PROPERTIES SOVERSION 0.1.0)

TARGET_LINK_LIBRARIES(${TARGET_CERT_SELECTION_UG}
    ${ug-cert-selection-ug-efl_pkgs_LIBRARIES}
)

INSTALL(TARGETS
    ${TARGET_CERT_SELECTION_UG}
    DESTINATION
    ${LIBDIR}
)
