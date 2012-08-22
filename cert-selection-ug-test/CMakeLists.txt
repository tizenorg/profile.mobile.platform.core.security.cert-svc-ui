pkg_check_modules(cert-selection-ug-test_pkgs
    REQUIRED
    elementary
    evas
    ecore-x
    ui-gadget-1
    dlog
    capi-appfw-application
    cert-svc-vcore
)

SET(SRCS
    cert-selection-ug-test.c
)

INCLUDE_DIRECTORIES(${cert-selection-ug-test_pkgs_INCLUDE_DIRS})

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
    MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

MESSAGE("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_EXECUTABLE(${TARGET_CERT_SELECTION_TEST} ${SRCS})

SET_TARGET_PROPERTIES(${TARGET_CERT_SELECTION_TEST} PROPERTIES SOVERSION 0.1.0)

TARGET_LINK_LIBRARIES(${TARGET_CERT_SELECTION_TEST}
    ${cert-selection-ug-test_pkgs_LIBRARIES}
)

INSTALL(TARGETS
    ${TARGET_CERT_SELECTION_TEST}
    DESTINATION
    /usr/bin
)
