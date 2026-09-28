cmake_minimum_required(VERSION 3.25)

project(faststore-api
    VERSION 0.1.0
    LANGUAGES CXX
)

# ---------------------------------------------------------
# Configuração da linguagem
# ---------------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Define um padrão apenas para geradores de configuração única.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(
        CMAKE_BUILD_TYPE
        Release
        CACHE STRING "Build type"
        FORCE
    )

    set_property(
        CACHE CMAKE_BUILD_TYPE
        PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel
    )
endif()

# ---------------------------------------------------------
# Dependências
# ---------------------------------------------------------

find_package(Drogon CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Gather all our source files. As the project grows across chapters,
# this glob keeps us from hand-editing this list every time we add a file.
file(GLOB_RECURSE FASTSTORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/controllers/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/services/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/repositories/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/models/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/dto/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/middleware/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/auth/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/database/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/config/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/routes/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/validation/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/logging/*.cc"
)

# ---------------------------------------------------------
# Executável
# ---------------------------------------------------------

add_executable(faststore-api
    cmd/main.cpp
    ${FASTSTORE_SOURCES}
)

target_include_directories(faststore-api
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

find_library(ARGON2_LIB argon2 REQUIRED)



target_link_libraries(faststore-api
    PRIVATE
        Drogon::Drogon
        jsoncpp_lib
        ${ARGON2_LIB}
        OpenSSL::Crypto
)

# ---------------------------------------------------------
# Avisos do compilador
# ---------------------------------------------------------

if(MSVC)
    target_compile_options(faststore-api PRIVATE /W4)
else()
    target_compile_options(faststore-api PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ---------------------------------------------------------
# Testes
# ---------------------------------------------------------

include(CTest)

if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()