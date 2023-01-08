
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BUFF_SIZE = 1000000


int perror_exit_1(){
    perror("");
    exit(1);
}


int get_size_of_file(char *path){
    unsigned long N = 0;
    struct stat st;
    if (stat(path, &st) == 0) {
        N = st.st_size;
    }
    return N;
}


int main(int argc, char *argv[]) {
    int IP_server; ////// ????????????????????????????????? ///////
    uint32_t C, N;
    unsigned short port_server;
    char *send_path;
    int left_written, sockfd, fd, read_len, write_len, offset;
    char *buff[BUFF_SIZE];
    struct sockaddr_in serv_addr;

    if (argc != 4){
        perror_exit_1();
    }

    IP_server = inet_pton(argv[1]);
    port_server = atoi(argv[2]);
    send_path = argv[3];

    fd = open(send_path, O_RDONLY);
    if (fd < 0) {
        perror_exit_1();
    }

    N = get_size_of_file();

    /* code from recitation */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit_1();
    }

    memset(buff, 0,sizeof(buff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_server);
    serv_addr.sin_addr.s_addr = inet_addr(IP_server); /* ??????????????? */

    if (connect(sockfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
        perror_exit_1();
    }

    /* send N to server */
    write(sockfd, N, sizeof(uint32_t));

    /* send N bytes to server */
    left_written = N;
    offset = 0;
    while (read_len = read(fd, buff, BUFF_SIZE) > 0){
        while (left_written > 0){
            write_len = write(sockfd, buff+offset, read_len) > 0;
            left_written -= write_len;
            offset  += write_len;
        }
    }

    bytes_read = read(sockfd, &C, sizeof(uint32_t));

    close(fd);
    close(sockfd);

    printf("# of printable characters: %u\n", C);

    exit(0);
}
