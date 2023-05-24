project(JsEngine LANGUAGES C)

set(HEADERS
    "src/headers/CertificateManager.h"
    "src/headers/HttpParser.h"
    "src/headers/RequestsManager.h"
    "src/headers/ServerManager.h")

set(SOURCES
    "src/CertificateManager.c"
    "src/HttpParser.c"
    "src/main.c"
    "src/RequestsManager.c"
    "src/ServerManager.c")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(CyberWeb ${SOURCES} ${HEADERS})
target_link_libraries(CyberWeb ssl crypto)