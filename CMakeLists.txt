project(JsEngine LANGUAGES C)

set(HEADERS
    "src/headers/BufferStorage.h"
    "src/headers/CertificateManager.h"
    "src/headers/HttpParser.h"
    "src/headers/RequestsManager.h"
    "src/headers/ProxyServer.h"
    "src/headers/AppUI.h"
    "src/headers/ServerManager.h")
    
    set(SOURCES
    "src/BufferStorage.c"
    "src/CertificateManager.c"
    "src/HttpParser.c"
    "src/main.c"
    "src/RequestsManager.c"
    "src/ProxyServer.c"
    "src/AppUI.c"
    "src/ServerManager.c")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(CyberWeb ${SOURCES} ${HEADERS})
target_link_libraries(CyberWeb ssl crypto PkgConfig::GTK4 X11)