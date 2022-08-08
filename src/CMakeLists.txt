add_executable(${PROJECT_NAME}
    main.cpp
    application.cpp
    window.cpp
    preferences.cpp
    alexaclient.cpp
    gtkwebview.cpp
    ${GENERATED_DATA_DIR}/resources.c
    )

link_directories(
    ${GTKMM_LIBRARY_DIRS}
    )

include_directories(
    ${GTKMM_INCLUDE_DIRS}
    )

target_link_libraries(${PROJECT_NAME}
    ${GTKMM_LIBRARIES}
    )

target_include_directories(${PROJECT_NAME} PRIVATE ${GENERATED_SOURCES_DIR})

compile_resources(
    ${GENERATED_DATA_DIR}/resources.c
    window.ui
    headerbar.ui
    menu.ui
    preferences.ui
    )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
