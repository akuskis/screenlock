project(screen_lock)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic -Wall -Wextra")

set(SOURCES
        src/main.cpp
        src/os.cpp
        src/os.hpp
        )

add_executable(${PROJECT_NAME} ${SOURCES})
