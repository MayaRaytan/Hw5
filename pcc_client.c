
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>


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


int main(int argc, char *argv[]) {
    uint32_t C, N, C_get, N_send;
    unsigned short port_server;
    char *send_path;
    int left_written, sockfd, fd, read_len, write_len, offset, fd_offset;
    char buff[BUFF_SIZE];
    struct sockaddr_in serv_addr;
    char *N_buffer;

    if (argc != 4){
        errno = EINVAL;
        perror_exit_1();
    }

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr) < 0)   {
        perror_exit_1(); /* ????????????????????????????????????????????????????????? */
    }

    port_server = atoi(argv[2]);
    send_path = argv[3];

    fd = open(send_path, O_RDONLY);
    if (fd < 0) {
        perror_exit_1();
    }

    N = get_size_of_file(send_path);

    /* code from recitation */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit_1();
    }

    if (memset(buff, 0,sizeof(buff)) < 0){
        perror_exit_1();
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_server);


    if (connect(sockfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
        perror_exit_1();
    }


    /* while loop !!!!!!!!!!!!! */

    N_send = htonl(N);
    N_buffer = (char *)&(htonl(N));
    /* send N to server */
    offset = 0;
    while ((write_len = write(sockfd, N_buffer + offset, sizeof(uint32_t) - offset)) > 0){
        offset += write_len;
    }
    if (write_len < 0 || (write_len == 0 && sizeof(uint32_t) - offset > 0)){
        perror_exit_1();
    }

    /* send N bytes to server */
    int keep_N = N;
    while (keep_N > 0){
        fd_offset = 0;
        offset = 0;
        /* read 1MB from file to buffer */
        while ((read_len = read(fd, buff + fd_offset, BUFF_SIZE - fd_offset)) > 0){
            fd_offset += read_len;
        }
        if (read_len < 0){
            perror_exit_1();
        }
        if (read_len == 0 && BUFF_SIZE - fd_offset > 0 && BUFF_SIZE < N){
            perror("");
            continue;
        }

        keep_N -= fd_offset;

        while ((write_len = write(sockfd, buff + offset, fd_offset - offset)) > 0) {
            offset += write_len;
        }
        if (write_len < 0){
            perror_exit_1();
        }
        else if (write_len == 0 && fd_offset - offset < 0){
            perror("");
//            close(connect_fd);
            break;
        }
    }



//    left_written = N;
//    offset = 0;
//    fd_offset = 0;
//    while ((read_len = read(fd, buff + fd_offset, BUFF_SIZE - fd_offset)) > 0){
//        while ((write_len = write(sockfd, buff+offset, read_len)) > 0) {
//            left_written -= write_len;
//            offset += write_len;
//        }
//        else if (write_len < 0){
//            perror_exit_1();
//        }
//        else if (write_len == 0 && left_written < 0){
//            perror("");
//            close(connect_fd);
//            break;
//        }
//    }

    offset = 0;
    while ((read_len = read(sockfd, (char *)C_get + offset, sizeof(uint32_t) - offset)) > 0){
        offset += read_len;
    }
    if (read_len < 0){
        perror_exit_1();
    }

    C = ntohl(C_get);

    close(fd);
    close(sockfd);

    printf("# of printable characters: %u\n", C);

    exit(0);
}
