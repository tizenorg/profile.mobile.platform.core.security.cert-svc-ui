pkg_check_modules(pkgs_common
    REQUIRED
    appcore-efl
    elementary
    dlog
    glib-2.0
    evas
    edje
)

SET(SRCS
    src/mgr-app-common-util.c
    src/mgr-app-view-manager.c
    src/mgr-app-widget.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/include)
INCLUDE_DIRECTORIES(${pkgs_common_INCLUDE_DIRS})

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")

IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")

ADD_LIBRARY(${TARGET_CERT_COMMON} SHARED ${SRCS})

TARGET_LINK_LIBRARIES(${TARGET_CERT_COMMON} ${pkgs_common_LDFLAGS})

INSTALL(TARGETS
    ${TARGET_CERT_COMMON}
    DESTINATION
    ${LIBDIR}
)
