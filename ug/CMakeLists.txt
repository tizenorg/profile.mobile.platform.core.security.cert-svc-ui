ADD_DEFINITIONS("-DBUILD_MGR_APP")

pkg_check_modules(ug-setting-manage-certificates-efl_pkgs
    REQUIRED
    dlog
    glib-2.0
    ui-gadget-1
    evas
    elementary
    edje
    eina
    efl-extension
    cert-svc-vcore
    capi-system-system-settings
    icu-i18n
    vconf
    libtzplatform-config
)

SET(SRCS
    ${CMAKE_SOURCE_DIR}/common/src/common-utils.c
    src/mgr-app-uigadget.c
    src/certificates_menu.c
    src/certificate_util.c
    src/detail_info_cert.c
    src/put_password_certificate.c
    src/trusted_root_ca_cert/trusted_root_ca_cert.c
    src/pfx_cert/pfx_cert.c
    src/pfx_cert/pfx_cert_install.c
    src/pfx_cert/pfx_cert_remove.c
)

FOREACH(flag ${ug-setting-manage-certificates-efl_pkgs_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/ug/include
    ${ug-setting-manage-certificates-efl_pkgs_INCLUDE_DIRS}
)

ADD_LIBRARY(${TARGET_CERT_MANAGER_UG} SHARED ${SRCS})
SET_TARGET_PROPERTIES(
    ${TARGET_CERT_MANAGER_UG}
    PROPERTIES
        COMPILE_FLAGS "-D_GNU_SOURCE -fPIC -fvisibility=default"
        SOVERSION ${SO_VERSION}
        VERSION ${VERSION}
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_MANAGER_UG}
    ${ug-setting-manage-certificates-efl_pkgs_LIBRARIES}
    ${ug-setting-manage-certificates-efl_pkgs_LDFLAGS}
)

INSTALL(TARGETS
    ${TARGET_CERT_MANAGER_UG}
    DESTINATION
    ${LIBDIR}
)
