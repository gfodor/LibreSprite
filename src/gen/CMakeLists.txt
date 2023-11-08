# Aseprite Code Generator
# Copyright (C) 2014, 2015  David Capello

add_executable(gen
  gen.cpp
  pref_types.cpp
  skin_class.cpp
  ui_class.cpp)

if(MSVC)
  # Fix problem compiling gen from a Visual Studio solution
  set_target_properties(gen
    PROPERTIES LINK_FLAGS -ENTRY:"mainCRTStartup")
endif()


if (NOT EMSCRIPTEN)
  target_link_libraries(gen
    base-lib
    tinyxml)
else()
  # Define a list of folder and project name pairs
  set(PROJECT_PAIRS
    base	base-lib
    observable	obs
    # folder project-name
    # You can add more pairs below
  )

# Iterate over the pairs
  while(PROJECT_PAIRS)
    list(POP_FRONT PROJECT_PAIRS FOLDER PROJECT)

    # Add the subdirectory
    #add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../${FOLDER} ${PROJECT})

    ## Get the sources from the project
    get_target_property(PROJECT_SOURCES ${PROJECT} SOURCES)

    # Provide the paths to the source files
    set(PROJECT_SOURCE_PATHS "")
    foreach(SOURCE_FILE IN LISTS PROJECT_SOURCES)
      list(APPEND PROJECT_SOURCE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../${FOLDER}/${SOURCE_FILE}")
    endforeach()

    ## Include the sources directly into the executable
    target_sources(gen PRIVATE ${PROJECT_SOURCE_PATHS})

    ## Add include directories if needed
    target_include_directories(gen PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/../${FOLDER}
    )
  endwhile()

  # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../base base-lib)

  # get_target_property(BASE_LIB_SOURCES base-lib SOURCES)

  # # Provide the paths to the source files relative to the current CMakeLists.txt file
  # set(BASE_LIB_SOURCE_PATHS "")
  # foreach(SOURCE_FILE IN LISTS BASE_LIB_SOURCES)
  #   list(APPEND BASE_LIB_SOURCE_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../base/${SOURCE_FILE}")
  # endforeach()

  # # Include the sources directly into the executable
  # target_sources(gen PRIVATE ${BASE_LIB_SOURCE_PATHS} ${TINYXML_SOURCES})

  # # Add include directories if needed
  # target_include_directories(gen PRIVATE
  #   ${CMAKE_CURRENT_SOURCE_DIR}/../base/include
  # )
endif()
