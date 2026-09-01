cmake_minimum_required(VERSION 3.18)
project(zstrzeszow-scraper VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")

set(SOURCE_FILES
  src/main.c
  src/utils/error.c
  src/utils/array.c
  src/utils/cellmap.c
  src/utils/logger.c
  src/utils/str_replace.c
  src/utils/hour_util.c
  src/utils/content_type.c
  src/scraper/list.c
  src/scraper/timetable.c
  src/database.c
  src/server/listener.c
  src/server/handler.c
  src/server/router.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(CMAKE_BUILD_TYPE Release)

target_link_libraries(${PROJECT_NAME} PUBLIC curl xml2 sqlite3)
