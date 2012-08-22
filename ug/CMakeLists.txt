pkg_check_modules(ug-setting-manage-certificates-efl_pkgs
    REQUIRED
    elementary
    ui-gadget-1
    evas
    dlog
    dpl-dbus-efl
    cert-svc-vcore
)

SET(SRCS
    src/mgr-app-uigadget.c
)

SET(CMAKE_INSTALL_RPATH "${PREFIX}/lib")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ug/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/include)
INCLUDE_DIRECTORIES(${ug-setting-manage-certificates-efl_pkgs_INCLUDE_DIRS})

ADD_DEFINITIONS(${ug-setting-manage-certificates-efl_pkgs_DEPS_CFLAGS})
ADD_DEFINITIONS(${ug-setting-manage-certificates-efl_pkgs_DEPS_OTHER})

ADD_LIBRARY(${TARGET_CERT_MANAGER_UG} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${TARGET_CERT_MANAGER_UG} PROPERTIES SOVERSION 0.1.0)

TARGET_LINK_LIBRARIES(${TARGET_CERT_MANAGER_UG}
    ${ug-setting-manage-certificates-efl_pkgs_LIBRARIES}
    ${ug-setting-manage-certificates-efl_pkgs_LDFLAGS}
    ${TARGET_CERT_COMMON}
    ${TARGET_CERT_VIEW}
)

INSTALL(TARGETS
    ${TARGET_CERT_MANAGER_UG}
    DESTINATION
    /opt/ug/lib
)
