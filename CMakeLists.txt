cmake_minimum_required(VERSION 3.20)
project(chess_engine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

# Source files
set(COMMON_SOURCES
    src/board.cpp
    src/movegen.cpp
    src/search.cpp
)

set(GAME_SOURCES
    src/game.cpp
)

# Executable: main (UCI engine)
add_executable(main
    src/main.cpp
    ${COMMON_SOURCES}
)

# Executable: runtest
add_executable(runtest
    src/runtest.cpp
    ${COMMON_SOURCES}
)

# Find GTK4 for GUI (optional)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(GTK4 gtk4)
endif()

# Executable: gui (GTK4 application) - only build if GTK4 is available
if(GTK4_FOUND)
    add_executable(gui
        src/gui.cpp
        ${COMMON_SOURCES}
        ${GAME_SOURCES}
    )
    
    # Link GTK4 to GUI executable
    target_include_directories(gui PRIVATE ${GTK4_INCLUDE_DIRS})
    target_link_directories(gui PRIVATE ${GTK4_LIBRARY_DIRS})
    target_link_libraries(gui PRIVATE ${GTK4_LIBRARIES})
    target_compile_options(gui PRIVATE ${GTK4_CFLAGS_OTHER})
    
    message(STATUS "GTK4 found - GUI executable will be built")
else()
    message(WARNING "GTK4 not found - GUI executable will NOT be built. Install GTK4 and pkg-config to build GUI.")
endif()

# Set output directory to src/ to match Makefile behavior
set_target_properties(main runtest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src
)

if(GTK4_FOUND)
    set_target_properties(gui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src
    )
endif()

# Set working directory for running executables
set_target_properties(main runtest PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
)

if(GTK4_FOUND)
    set_target_properties(gui PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
    )
endif()
