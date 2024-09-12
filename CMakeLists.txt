cmake_minimum_required(VERSION 3.21)
project(zstrzeszow-scraper VERSION 0.1.0 LANGUAGES C)

set(SOURCE_FILES
  src/main.c
  src/utils/error.c
  src/utils/array.c
  src/scraper/list.c
  src/scraper/timetable.c
  src/database.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PUBLIC curl xml2 sqlite3)
