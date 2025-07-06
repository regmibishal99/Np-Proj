#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BOARD_SIZE 3

char board[BOARD_SIZE][BOARD_SIZE];

void initialize_board() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '1' + i * BOARD_SIZE + j;
        }
    }
}

void print_board() {
    printf("\nCurrent Board:\n");
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
    printf("\n");
}

int check_winner(char player) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) return 1;
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player) return 1;
    }
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) return 1;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) return 1;
    return 0;
}

int is_board_full() {
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] >= '1' && board[i][j] <= '9') return 0;
    return 1;
}

int make_move(int position, char player) {
    if (position < 1 || position > 9) return 0;
    
    int row = (position - 1) / BOARD_SIZE;
    int col = (position - 1) % BOARD_SIZE;
    
    if (board[row][col] == 'O' || board[row][col] == 'X') return 0;
    
    board[row][col] = player;
    return 1;
}

void send_board(int client_socket) {
    send(client_socket, board, BOARD_SIZE * BOARD_SIZE, 0);
}

void receive_board(int client_socket) {
    recv(client_socket, board, BOARD_SIZE * BOARD_SIZE, 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    initialize_board();
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    printf("Client connected. You are 'O', client is 'X'.\n");
    
    while (1) {
       
        print_board();
        printf("Your turn (O). Enter position (1-9): ");
        
        int position;
        scanf("%d", &position);
        
        while (!make_move(position, 'O')) {
            printf("Invalid move. Try again: ");
            scanf("%d", &position);
        }
        
        send_board(client_socket);
        
        if (check_winner('O')) {
            print_board();
            printf("You win!\n");
            break;
        }
        if (is_board_full()) {
            print_board();
            printf("It's a draw!\n");
            break;
        }
        
        printf("Waiting for client's move...\n");
        receive_board(client_socket);
        
        if (check_winner('X')) {
            print_board();
            printf("Client wins!\n");
            break;
        }
        if (is_board_full()) {
            print_board();
            printf("It's a draw!\n");
            break;
        }
    }
    
    close(client_socket);
    close(server_fd);
    return 0;
}