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
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = '1' + i * BOARD_SIZE + j;
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

void send_board(int sock) {
    send(sock, board, BOARD_SIZE * BOARD_SIZE, 0);
}

void receive_board(int sock) {
    recv(sock, board, BOARD_SIZE * BOARD_SIZE, 0);
}

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }
    
    initialize_board();
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    printf("Connected to server. You are 'X', server is 'O'.\n");
    
    while (1) {
        
        printf("Waiting for server's move...\n");
        receive_board(sock);
        
        if (check_winner('O')) {
            print_board();
            printf("Server wins!\n");
            break;
        }
        if (is_board_full()) {
            print_board();
            printf("It's a draw!\n");
            break;
        }
        
        
        print_board();
        printf("Your turn (X). Enter position (1-9): ");
        
        int position;
        scanf("%d", &position);
        
        while (!make_move(position, 'X')) {
            printf("Invalid move. Try again: ");
            scanf("%d", &position);
        }
        
        send_board(sock);
        
        if (check_winner('X')) {
            print_board();
            printf("You win!\n");
            break;
        }
        if (is_board_full()) {
            print_board();
            printf("It's a draw!\n");
            break;
        }
    }
    
    close(sock);
    return 0;
}