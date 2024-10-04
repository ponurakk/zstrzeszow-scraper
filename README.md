# ZSTRzeszow website scraper

# Features
- Classes timetable
- Abbreviated lessons
- Old timetables
- Improved UI

# Instalation
Packages needed:
libcurl-dev
libxml2-dev
libsqlite3-dev

To fix libxml issue:
```sh
ln -s /usr/include/libxml2/libxml /usr/include/libxml
```

## Build
```sh
cmake -Bbuild && make -Cbuild
```

## Run
```sh
./build/zstrzeszow-scraper
```
