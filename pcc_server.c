
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
#define True 1


uint32_t pcc_total[95] = {0};
int connect_fd = 1;
int accept_flag, finish = 0;


int perror_exit_1();
void print_printable_characters(void);
int client_to_server(char *buff, int upside_down, int sockfd, long left);
unsigned int count_printable_update_pcc(char *buff, uint32_t N, uint32_t *pcc);
void my_signal_handler();
void init_new_sigint();


int perror_exit_1(){
    perror("");
    exit(1);
}

void print_printable_characters(void){
    int i;
    uint32_t d;
    for (i = 0; i < 95; i++){
        d = pcc_total[i];
        printf("char '%c' : %u times\n", (i + 32), d);
    }
}


int client_to_server(char *buff, int upside_down, int sockfd, long left){
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
            if (errno != ETIMEDOUT && errno != ECONNRESET && errno != EPIPE){
                perror_exit_1();
            }
            perror("");
            return 0;
        }
        if (res == 0){
            perror("");
            return 0;
        }
        left -= res;
        done += res;
    }
    buff[done] = '\0';
    return 1;
}



unsigned int count_printable_update_pcc(char *buff, uint32_t N, uint32_t *pcc){
    uint32_t i;
    uint32_t C = 0;
    for (i = 0; i < N; i++){
        if (32 <= buff[i] && buff[i] <= 126){
            pcc[buff[i] - 32]++;
            C++;
        }
    }
    return C;
}


void my_signal_handler() {
    if (!accept_flag) {
        print_printable_characters();
        exit(0);
    }
    else{
        finish = 1;
    }
}


void init_new_sigint(){
    struct sigaction new_action = {
            .sa_sigaction = &my_signal_handler,
            .sa_flags = SA_RESTART
    };
    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        perror_exit_1();
    }
}



int main(int argc, char *argv[]) {
    char buff[BUFF_SIZE];
    int listen_fd, i;
    int move_to_next_client = 0;
    int helper, res = 1;
    uint16_t port_server;
    socklen_t address_size;
    struct sockaddr_in serv_addr;
    uint32_t C, C_send, N, N_get, keep_N;
    uint32_t now_pcc[95];

    init_new_sigint();

    if (argc != 2) {
        errno = EINVAL;
        perror_exit_1();
    }

    port_server = atoi(argv[1]);

    address_size = sizeof(struct sockaddr_in);
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit_1();
    }


    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &helper, sizeof(int)) < 0){
        perror_exit_1();
    }

    memset(pcc_total, 0, sizeof(pcc_total));
    memset(&serv_addr, 0, address_size);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port_server);


    if (bind(listen_fd, (struct sockaddr *) &serv_addr, address_size) != 0) {
        perror_exit_1();
    }

    if (listen(listen_fd, 10) != 0) {
        perror_exit_1();
    }


    while (True) {
        /* SIGINT */
        if (finish){break;}
        accept_flag = 0;

        memset(now_pcc, 0, sizeof(now_pcc));
        C = 0;
        move_to_next_client = 0;
        if ((connect_fd = accept(listen_fd, NULL, NULL)) < 0){
            perror_exit_1();
        }
        accept_flag = 1;
        res = client_to_server((char *)&N_get, 1, connect_fd, 4);
        if (!res){
            if (close(connect_fd) < 0){
                perror_exit_1();
            }
            /* continue to next client */
            continue;
        }
        N = htonl(N_get);
        if (N < 1){
            move_to_next_client = 1;
        }
        /* read N bytes from client */
        keep_N = N;

        while (keep_N > 0){
            res = read(connect_fd, buff, BUFF_SIZE);
            if (errno == EINTR){
                errno = 0;
                continue;
            }
            if (res < 0){
                if (errno != ETIMEDOUT && errno != ECONNRESET && errno != EPIPE){
                    perror_exit_1();
                }
                move_to_next_client = 1;
                perror("");
                break;
            }
            if (res == 0){
                move_to_next_client = 1;
                perror("");
                break;
            }
            keep_N -= res;
            buff[res] = '\0';
            C += count_printable_update_pcc(buff, res, now_pcc);
        }

        if (!move_to_next_client) {
            /* update pcc total with current client */
            for (i = 0; i < 95; i++){
                pcc_total[i] += now_pcc[i];
                now_pcc[i] = 0;
            }
            C_send = htonl(C);
            client_to_server((char *)&C_send, 0, connect_fd, 4);
        }
        if (close(connect_fd) < 0){
            perror_exit_1();
        }
        accept_flag = 0;
    }
    print_printable_characters();
    if (close(listen_fd) < 0){
        perror_exit_1();
    }
    exit(0);
}