cmake_minimum_required(VERSION 3.25)

project(CyberWeb LANGUAGES C)

set(HEADERS
    "src/headers/BufferStorage.h"
    "src/headers/CertificateManager.h"
    "src/headers/HttpParser.h"
    "src/headers/RequestsManager.h"
    "src/headers/ProxyServer.h"
    "src/headers/Json.h"
    "src/headers/Helpers.h"
    "src/headers/ElectronIPC.h"
    "src/headers/ServerManager.h")
    
set(SOURCES
    "src/BufferStorage.c"
    "src/CertificateManager.c"
    "src/HttpParser.c"
    "src/main.c"
    "src/RequestsManager.c"
    "src/ProxyServer.c"
    "src/Json.c"
    "src/Helpers.c"
    "src/ElectronIPC.c"
    "src/ServerManager.c")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(CyberWeb ${SOURCES} ${HEADERS})
target_link_libraries(CyberWeb ssl crypto)
