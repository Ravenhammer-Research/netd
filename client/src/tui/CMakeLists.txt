# TUI module sources
set(TUI_SOURCES
  command.cpp
  message.cpp
  prompt.cpp
  curses.cpp
  cursor.cpp
  screen.cpp
  logger.cpp
  text.cpp
)

# Create TUI library
add_library(netd_tui STATIC ${TUI_SOURCES})

# Link curses library
target_link_libraries(netd_tui curses)

# Set include directories
target_include_directories(netd_tui PRIVATE ${GLOBAL_INCLUDE_DIRS})

