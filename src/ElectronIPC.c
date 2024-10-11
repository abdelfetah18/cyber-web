#include "./headers/ElectronIPC.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

int electronSocket;

void createSocket() {
  electronSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (electronSocket == -1) {
    printf("Could not create socket\n");
    return;
  }
  printf("[*] Create ElectronSocket\n");
}

void connectToElectronSocket() {
  struct sockaddr_in server;

  server.sin_family = AF_INET;
  server.sin_port = htons(5555);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(electronSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("Connection error\n");
    return;
  }
  printf("[*] Connect to ElectronSocket\n");
}

void sendDataToElectronSocket(char *data, int length) {
  if (send(electronSocket, data, length, 0) < 0) {
    printf("Send failed\n");
    return;
  }
  printf("[*] Send data to Electron Socket\n");
}

void initializeElectronSocket() {
  printf("[*] Initialize Electron Socket\n");
  createSocket();
  connectToElectronSocket();
}