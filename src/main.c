#include "headers/ProxyServer.h"
#include <time.h>

int main(int argc,char** argv){
    // Seed the random number generator
    srand(time(NULL));
    startProxyServer(NULL);
    return 0;
}