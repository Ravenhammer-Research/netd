# CMakeLists.txt for FreeBSD route module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Route module source files
set(ROUTE_SOURCES
    delete.c
    set.c
    show.c
)

# Route module headers
set(ROUTE_HEADERS
    route.h
)

# Add route sources to parent scope
set(FREEBSD_SOURCES ${FREEBSD_SOURCES} ${ROUTE_SOURCES} PARENT_SCOPE)
