
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>


#define BUFF_SIZE 1048576


int perror_exit_1(){
    perror("");
    exit(1);
}


uint32_t get_size_of_file(char *path){
    uint32_t N = 0;
    struct stat st;
    if (stat(path, &st) == 0) {
        N = st.st_size;
    }
    else{
        perror_exit_1();
    }
    return N;
}


void client_to_server(char *buff, int upside_down, int sockfd, long left){
    long done = 0;
    int res;
    while (left > 0){
        /* server to client */
        if (upside_down){
            res = read(sockfd, buff + done, left);
        }
        /* client to server */
        else{
            res = write(sockfd, buff + done, left);
        }
        if (errno == EINTR){
            errno = 0;
            continue;
        }
        if (res < 0){
            perror_exit_1();
        }
        left -= res;
        done += res;
    }
    buff[done] = '\0';
}


int main(int argc, char *argv[]) {
    uint32_t C, N, C_get, N_send, keep_N;
    unsigned short port_server;
    char *send_path;
    int sockfd, file_read;
    char buff[BUFF_SIZE];
    struct sockaddr_in serv_addr;
    FILE *file;


    if (argc != 4){
        errno = EINVAL;
        perror_exit_1();
    }


    port_server = (unsigned short)strtol(argv[2], NULL, 10);
    send_path = argv[3];

    memset(&serv_addr, 0,sizeof(serv_addr));

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr) != 1){
        perror_exit_1();
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_server);

    /* code from recitation */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit_1();
    }


    if (connect(sockfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
        perror_exit_1();
    }


    if ((file = fopen(send_path, "r")) == NULL){
        perror_exit_1();
    }

    N = get_size_of_file(send_path);
    N_send = htonl(N);
    /* send N to server */
    client_to_server((char *) (&N_send), 0, sockfd, 4);

    /* send N bytes to server */
    keep_N = N;
    while (keep_N > 0){
        file_read = fread(buff, 1, BUFF_SIZE, file);
        if (file_read <= 0 && !feof(file)){
            perror_exit_1();
        }
        buff[file_read] = '\0';
        client_to_server(buff, 0, sockfd, file_read);
        keep_N -= file_read;
    }

    client_to_server((char *) (&C_get), 1, sockfd, 4);
    C = ntohl(C_get);

    if (fclose(file) == EOF){
        perror_exit_1();
    }
    if (close(sockfd) < 0){
        perror_exit_1();
    }

    printf("# of printable characters: %u\n", C);

    exit(0);
}
