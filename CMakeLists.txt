cmake_minimum_required(VERSION 3.21)
project(zstrzeszow-scraper VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_FLAGS_DEBUG "-g")

set(SOURCE_FILES
  src/main.c
  src/utils/error.c
  src/utils/array.c
  src/utils/cellmap.c
  src/utils/logger.c
  src/utils/str_replace.c
  src/scraper/list.c
  src/scraper/timetable.c
  src/database.c
  src/server/listener.c
  src/server/handler.c
  src/server/router.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC curl xml2 sqlite3)
