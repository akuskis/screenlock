cmake_minimum_required(VERSION 3.25)

project(screen_lock)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic -Wall -Wextra")

set(SOURCES
        src/auth/Auth.hpp
        src/auth/AuthError.hpp
        src/auth/ShadowAuth.cpp
        src/auth/ShadowAuth.hpp
        src/locker/Locker.hpp
        src/locker/LockerError.hpp
        src/locker/Position.hpp
        src/locker/Size.hpp
        src/locker/X11Locker.cpp
        src/locker/X11Locker.hpp
        src/locker/X11Locker_impl.cpp
        src/locker/X11Locker_impl.hpp
        src/main.cpp
        src/os.cpp
        src/os.hpp
        )

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} crypt X11 pthread)
