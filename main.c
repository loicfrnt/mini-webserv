#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

int serv_fd, nfds;
int allFds[1024];
fd_set readfds, writefds;

void fatal(){
  write(2, "Fatal error\n", 12);
  exit(1);
}

void strout(char *str){
  write(1, str, strlen(str) + 1);
}

void printfds(){
  for (int i = 0; i < 1024; i++)
    printf("%d", allFds[i]);
  printf("\n");
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void addCli(){
  struct sockaddr_in cli;
  int len, connfd;
	len = sizeof(cli);
	connfd = accept(serv_fd, (struct sockaddr *)&cli, &len);
	if (connfd < 0)
    fatal();
  else
    strout("server acccept the client...\n");
  if (connfd > nfds)
    nfds = connfd;
  FD_SET(connfd, &readfds);
  FD_SET(connfd, &writefds);
}

void loop(){
  fd_set sel_read, sel_write;
  FD_COPY(&readfds, &sel_read);
  FD_COPY(&writefds, &sel_write);
  printf("nfds %d\n", nfds);
  select(nfds + 1, &sel_read, &sel_write, NULL, NULL);
  printf("select returns\n");
  if (FD_ISSET(serv_fd, &sel_read))
    addCli();
}

int main(int ac, char** av) {
	struct sockaddr_in servaddr;
  if (ac == 1){
    write(2, "Wrong number of arguments\n", 26);
    exit(1);
  }
  const int port = atoi(av[1]);
  printf("port: %d\n", port);

	// socket create and verification
	serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_fd == -1)
    fatal();
	else
		strout("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification
	if ((bind(serv_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
     fatal();
	else
		strout("Socket successfully binded..\n");
	if (listen(serv_fd, 10) != 0) {
		strout("cannot listen\n");
		exit(0);
	}
  memset(allFds, -1, sizeof(allFds));
  nfds = serv_fd;
  allFds[0] = serv_fd;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_SET(serv_fd, &readfds);
  // testing
  printfds();
  //
  while (1)
    loop();
}
