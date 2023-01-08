
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BUFF_SIZE = pow(10,6)
#define True 1


typedef struct pcc_total{
    unsigned int *counts[95];
}pcc_total;


int perror_exit_1(){
    perror("");
    exit(1);
}


int main(int argc, char *argv[]) {
    unsigned short port_server;
    pcc_total *pcc_data;
    int listen_fd, connect_fd, left_written, read_len, write_len, offset;
    socklen_t address_size;
    struct sockaddr_in server_addr;
    unsigned int C, N;

    if (argc != 2){
        perror_exit_1();
    }

    port_server = atoi(argv[1]);
    pcc_data = (pcc_total*) malloc(sizeof(pcc_total));

    address_size = sizeof(struct sockaddr_in);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, address_size);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_server);

    if(bind(listen_fd, (struct sockaddr*) &server_addr, address_size) != 0){
        perror();
    }

    if(listen(listen_fd, 10) != 0){
        perror_exit_1();
    }

    while(True){
        connect_fd = accept(listenfd, (struct sockaddr*) &peer_addr, &address_size);
        if(connect_fd < 0 ){
            perror_exit_1();
        }

//        getsockname(connfd, (struct sockaddr*) &my_addr,   &addrsize);
//        getpeername(connfd, (struct sockaddr*) &peer_addr, &addrsize);
//        printf( "Server: Client connected.\n"
//                "\t\tClient IP: %s Client Port: %d\n"
//                "\t\tServer IP: %s Server Port: %d\n",
//                inet_ntoa( peer_addr.sin_addr ),
//                ntohs(     peer_addr.sin_port ),
//                inet_ntoa( my_addr.sin_addr   ),
//                ntohs(     my_addr.sin_port   ) );

//        totalsent = 0;
//        int notwritten = strlen(data_buff);
//
//
//        offset = 0;
//        while (read_len = read(fd, buff, BUFF_SIZE) > 0){
//            while (left_written > 0){
//                write_len = write(sockfd, buff+offset, read_len) > 0;
//                left_written -= write_len;
//                offset  += write_len;
//            }
//        }

        if ((read_len = read(connect_fd, &N, sizeof(unsigned int))) < 0){
            perror_exit_1();
        }

        char *buff[BUFF_SIZE];
        /* read bytes from client */
        while( notwritten > 0 )
        {

            nsent = read(connect_fd, data_buff + totalsent, notwritten);
            // check if error occured (client closed connection?)
            assert( nsent >= 0);
            printf("Server: wrote %d bytes\n", nsent);

            totalsent  += nsent;
            notwritten -= nsent;
        }

        // close socket
        close(connfd);
    }





    printf("\"char '%c' : %u times\\n\"", );
}