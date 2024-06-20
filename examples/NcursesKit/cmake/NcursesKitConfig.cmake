include(CMakeFindDependencyMacro)

set(CURSES_NEED_NCURSES TRUE)
find_dependency(Curses)

include("${CMAKE_CURRENT_LIST_DIR}/NcursesKitTargets.cmake")
