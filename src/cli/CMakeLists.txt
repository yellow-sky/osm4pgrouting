FILE(GLOB CLI_SRC "${CMAKE_SOURCE_DIR}/src/cli/*.cpp")
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/src/core")

ADD_EXECUTABLE(osm4pgrouting-cli ${CLI_SRC})

TARGET_LINK_LIBRARIES(osm4pgrouting-cli osm4pgrouting)

INSTALL(TARGETS osm4pgrouting-cli
  RUNTIME DESTINATION bin
)
