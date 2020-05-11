#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {

  struct sockaddr_rc loc_addr = {0}, rem_addr = {0};
  char buf[1024] = {0};
  int s, client, bytes_read;
  socklen_t opt = sizeof(rem_addr);

  // allocate socket
  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // bind socket to first available
  loc_addr.rc_family = AF_BLUETOOTH;
  loc_addr.rc_bdaddr = *BDADDR_ANY;
  loc_addr.rc_channel = (uint8_t)1;
  bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

  // put the socket into listening mode
  listen(s, 1);

  // accept one connection
  client = accept(s, (struct sockaddr *)&rem_addr, &opt);

  ba2str(&rem_addr.rc_bdaddr, buf);
  fprintf(stderr, "accepted connection from %s:\n", buf);
  memset(buf, 0, sizeof(buf));

  // Read data from client
  bytes_read = recv(client, buf, sizeof(buf), 0);
  if (bytes_read > 0) {
    printf("received [%s]\n", buf);
  }

  // close connection
  close(client);
  close(s);
  return 0;
}