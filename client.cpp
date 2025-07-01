#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

char board[9] = {'1','2','3','4','5','6','7','8','9'};

void draw_board() {
    std::cout << "\n";
    std::cout << " " << board[0] << " | " << board[1] << " | " << board[2] << "\n";
    std::cout << "-----------\n";
    std::cout << " " << board[3] << " | " << board[4] << " | " << board[5] << "\n";
    std::cout << "-----------\n";
    std::cout << " " << board[6] << " | " << board[7] << " | " << board[8] << "\n";
    std::cout << "\n";
}

void send_move(int socket, const char* move) {
    send(socket, move, strlen(move), 0);
    usleep(100000); // Small delay to ensure message is sent
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IP address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }
    
    std::cout << "Connected to server! Game starting...\n";
    std::cout << "You are O, server is X\n";
    
    while (true) {
        // Receive game state from server
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            std::cout << "Server disconnected.\n";
            break;
        }
        
        // Check for special messages
        if (strncmp(buffer, "SERVER_WIN", 10) == 0) {
            draw_board();
            std::cout << "Server wins!\n";
            break;
        } else if (strncmp(buffer, "DRAW", 4) == 0) {
            draw_board();
            std::cout << "It's a draw!\n";
            break;
        } else if (strncmp(buffer, "INVALID_MOVE", 12) == 0) {
            std::cout << "Your previous move was invalid. Try again.\n";
            continue;
        } else if (strncmp(buffer, "BOARD:", 6) == 0) {
            // Receive the actual board state
            valread = read(sock, board, 9);
            if (valread <= 0) break;
        }
        
        draw_board();
        
        // Player's turn (O)
        int move;
        std::cout << "Your turn (O). Enter position (1-9): ";
        std::cin >> move;
        
        if (move < 1 || move > 9 || board[move-1] == 'X' || board[move-1] == 'O') {
            std::cout << "Invalid move! Try again.\n";
            continue;
        }
        
        board[move-1] = 'O';
        
        // Check for win or draw
        bool win = false;
        bool draw = true;
        
        // Check rows, columns, and diagonals for win
        for (int i = 0; i < 3; i++) {
            if (board[i*3] == 'O' && board[i*3+1] == 'O' && board[i*3+2] == 'O')
                win = true;
            if (board[i] == 'O' && board[i+3] == 'O' && board[i+6] == 'O')
                win = true;
        }
        if (board[0] == 'O' && board[4] == 'O' && board[8] == 'O')
            win = true;
        if (board[2] == 'O' && board[4] == 'O' && board[6] == 'O')
            win = true;
        
        // Check for draw
        for (int i = 0; i < 9; i++) {
            if (board[i] != 'X' && board[i] != 'O') {
                draw = false;
                break;
            }
        }
        
        if (win) {
            draw_board();
            std::cout << "You win!\n";
            send_move(sock, "CLIENT_WIN");
            break;
        } else if (draw) {
            draw_board();
            std::cout << "It's a draw!\n";
            send_move(sock, "DRAW");
            break;
        }
        
        // Send move to server
        send_move(sock, std::to_string(move).c_str());
    }
    
    close(sock);
    return 0;
}