MESSAGE(CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR})
MESSAGE(CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR})
MESSAGE(CMAKE_EDJ_DIR: ${EDJDIR})
MESSAGE(CMAKE_ICON_DIR: ${ICONDIR})

ADD_CUSTOM_TARGET(ug-setting-manage-certificates-efl.edj
    COMMAND edje_cc ${CMAKE_CURRENT_SOURCE_DIR}/edc/ug-setting-manage-certificates-efl.edc ${CMAKE_BINARY_DIR}/data/ug-setting-manage-certificates-efl.edj
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/edc/ug-setting-manage-certificates-efl.edc
)

ADD_DEPENDENCIES(${PROJECT_NAME} ug-setting-manage-certificates-efl.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/data/ug-setting-manage-certificates-efl.edj DESTINATION ${EDJDIR}/${PROJECT_NAME})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/ManageApplications.png DESTINATION ${ICONDIR}/${PROJECT_NAME})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icons/ManageApplications_default.png DESTINATION ${ICONDIR}/${PROJECT_NAME})
