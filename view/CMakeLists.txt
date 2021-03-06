pkg_check_modules(pkgs_view
    REQUIRED
    appcore-efl
    elementary
    dlog
    glib-2.0
    evas
    edje
    aul
    eina
    ail
    appsvc
    icu-i18n
    ui-gadget-1
    capi-appfw-application
    cert-svc-vcore
)

SET(SRCS
    src/certificates/certificates_menu.c
    src/certificates/certificate_util.c
    src/certificates/trusted_root_ca_cert/trusted_root_ca_cert.c
    src/certificates/user_cert/install_certificate.c
    src/certificates/user_cert/user_cert.c
    src/certificates/pfx_cert/pfx_cert.c
    src/certificates/pfx_cert/pfx_cert_install.c
    src/certificates/pfx_cert/pfx_cert_remove.c
    src/certificates/detail_info_cert.c
    src/certificates/put_password_certificate.c
    src/certificates/uninstall_certificate.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/view/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ug/include)
INCLUDE_DIRECTORIES(${pkgs_view_INCLUDE_DIRS})

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
    MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

MESSAGE("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${TARGET_CERT_VIEW} SHARED ${SRCS})

TARGET_LINK_LIBRARIES(${TARGET_CERT_VIEW}
    ${pkgs_view_LIBRARIES}
    ${TARGET_CERT_COMMON}
)

INSTALL(TARGETS
    ${TARGET_CERT_VIEW}
    DESTINATION
    ${LIBDIR}
)
