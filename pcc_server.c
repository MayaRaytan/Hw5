
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


#define BUFF_SIZE 1000000
#define True 1


uint32_t pcc_total[95] = {0};
int finish = 0;


int perror_exit_1(){
    perror("");
    exit(1);
}


unsigned int count_printable_update_pcc(unsigned int *buff, unsigned int N, int C){
    unsigned int i;
    for (i = 0; i < N; i++){
        if (32 <= buff[i] && buff[i] <= 126){
            pcc_total[buff[i] - 32]++;
            C++;
        }
    }
    return C;
}


void my_signal_handler() {
    finish = 1;
}


void init_new_sigint(){
    struct sigaction new_action = {
            .sa_sigaction = my_signal_handler,
            .sa_flags = SA_SIGINFO
    };
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        perror_exit_1();
    }
}


void print_printable_characters(void){
    int i;
    uint32_t d;
    for (i = 0; i < 95; i++){
        d = pcc_total[i];
        printf("char '%c' : %u times\n", (i + 32), d);
    }
}


int main(int argc, char *argv[]) {
    unsigned short port_server;
    int listen_fd, connect_fd, left_written, write_len;
    int offset, read_len = 0;
    socklen_t address_size;
    struct sockaddr_in server_addr;
    uint32_t C, N;
    uint32_t
    buff[BUFF_SIZE];
    int helper = 1;

    if (argc != 2) {
        perror_exit_1();
    }

    port_server = atoi(argv[1]);

    address_size = sizeof(struct sockaddr_in);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, address_size);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_server);

    if (bind(listen_fd, (struct sockaddr *) &server_addr, address_size) != 0) {
        perror_exit_1();
    }

    if (listen(listen_fd, 10) != 0) {
        perror_exit_1();
    }

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &helper, sizeof(int)) < 0){
        perror_exit_1();
    }

    init_new_sigint();

    while (True) {
        C = 0;
        finish = 0;
        connect_fd = accept(listen_fd, NULL, NULL);
        if (connect_fd < 0) {
            perror_exit_1();
        }

        while ((read_len = read(connect_fd, &N + offset, sizeof(uint32_t) - read_len)) < 0){
            if (errno != ETIMEDOUT && errno != ECONNRESET && errno != EPIPE){
                perror_exit_1();
            }
            else{
                perror("");
                close(connect_fd);
                continue;
            }
        }
        else if(read_len == 0 && sizeof(uint32_t) - read_len > 0){
            perror("");
            close(connect_fd);
            continue;
        }

        offset = 0;
        left_written = N;
        /* read bytes from client */
        while (left_written > 0) {
            if ((read_len = read(connect_fd, buff + offset, left_written)) < 0){
                if (errno != ETIMEDOUT && errno != ECONNRESET && errno != EPIPE){
                    perror_exit_1();
                }
                else{
                    perror("");
                    close(connect_fd);
                    break;
                }
            }
            else if(read_len == 0 && left_written > 0){
                perror("");
                close(connect_fd);
                break;
            }
            offset += read_len;
            left_written -= read_len;
            C = count_printable_update_pcc(buff, N, C);
        }



        /* while loop !!!!!!!!!!!!! */
        if ((write_len = write(connect_fd, &C, sizeof(uint32_t))) < 0) {
            if (errno != ETIMEDOUT && errno != ECONNRESET && errno != EPIPE) {
                perror_exit_1();
            } else {
                perror("");
                close(connect_fd);
                continue;
            }
        }
        else if (write_len == 0){
            perror("");
            close(connect_fd);
            continue;
        }

        close(connect_fd);

        /* in the beginning or at the end ????????????????????????????????? */
        if (finish){
            break;
        }
    }
    print_printable_characters();
    exit(0);
}