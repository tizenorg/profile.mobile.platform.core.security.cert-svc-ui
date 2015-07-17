pkg_check_modules(pkgs_view
    REQUIRED
    dlog
    capi-system-system-settings
    ui-gadget-1
    elementary
    evas
    edje
    eina
    efl-extension
    icu-i18n
    cert-svc-vcore
    vconf
)

SET(SRCS
    src/certificates/certificates_menu.c
    src/certificates/certificate_util.c
    src/certificates/trusted_root_ca_cert/trusted_root_ca_cert.c
    src/certificates/pfx_cert/pfx_cert.c
    src/certificates/pfx_cert/pfx_cert_install.c
    src/certificates/pfx_cert/pfx_cert_remove.c
    src/certificates/detail_info_cert.c
    src/certificates/put_password_certificate.c
#   src/certificates/uninstall_certificate.c
)

FOREACH(flag ${pkgs_view_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_INSTALL_RPATH "${LIBDIR}")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/view/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ug/include)
INCLUDE_DIRECTORIES(${pkgs_view_INCLUDE_DIRS})

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
    MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

MESSAGE("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${TARGET_CERT_VIEW} SHARED ${SRCS})

TARGET_LINK_LIBRARIES(${TARGET_CERT_VIEW}
    ${pkgs_view_LIBRARIES}
)

INSTALL(TARGETS
    ${TARGET_CERT_VIEW}
    DESTINATION
    ${LIBDIR}
)
