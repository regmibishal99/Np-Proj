#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

char board[9] = {'1','2','3','4','5','6','7','8','9'};
int current_player = 0; // 0 for server (X), 1 for client (O)

void draw_board() {
    std::cout << "\n";
    std::cout << " " << board[0] << " | " << board[1] << " | " << board[2] << "\n";
    std::cout << "-----------\n";
    std::cout << " " << board[3] << " | " << board[4] << " | " << board[5] << "\n";
    std::cout << "-----------\n";
    std::cout << " " << board[6] << " | " << board[7] << " | " << board[8] << "\n";
    std::cout << "\n";
}

bool check_win(char player) {
    // Check rows, columns, and diagonals
    for (int i = 0; i < 3; i++) {
        if (board[i*3] == player && board[i*3+1] == player && board[i*3+2] == player)
            return true;
        if (board[i] == player && board[i+3] == player && board[i+6] == player)
            return true;
    }
    if (board[0] == player && board[4] == player && board[8] == player)
        return true;
    if (board[2] == player && board[4] == player && board[6] == player)
        return true;
    return false;
}

bool check_draw() {
    for (int i = 0; i < 9; i++) {
        if (board[i] != 'X' && board[i] != 'O')
            return false;
    }
    return true;
}

void send_game_state(int socket, const char* state) {
    send(socket, state, strlen(state), 0);
    usleep(100000); // Small delay to ensure message is sent
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Tic-Tac-Toe Server waiting for connection on port " << PORT << std::endl;
    
    // Accept connection
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Client connected! Game starting...\n";
    std::cout << "You are X, client is O\n";
    
    // Send initial board state
    send_game_state(client_socket, "BOARD:");
    send(client_socket, board, 9, 0);
    
    while (true) {
        draw_board();
        
        if (current_player == 0) { // Server's turn (X)
            int move;
            std::cout << "Your turn (X). Enter position (1-9): ";
            std::cin >> move;
            move--; // Convert to 0-based index
            
            if (move < 0 || move > 8 || board[move] == 'X' || board[move] == 'O') {
                std::cout << "Invalid move! Try again.\n";
                continue;
            }
            
            board[move] = 'X';
            current_player = 1;
            
            // Check for win or draw
            if (check_win('X')) {
                draw_board();
                std::cout << "You win!\n";
                send_game_state(client_socket, "SERVER_WIN");
                break;
            } else if (check_draw()) {
                draw_board();
                std::cout << "It's a draw!\n";
                send_game_state(client_socket, "DRAW");
                break;
            }
            
            // Send updated board to client
            send_game_state(client_socket, "BOARD:");
            send(client_socket, board, 9, 0);
        } else { // Client's turn (O)
            std::cout << "Waiting for client's move...\n";
            
            // Receive client's move
            int valread = read(client_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                std::cout << "Client disconnected.\n";
                break;
            }
            
            // Check for special messages
            if (strncmp(buffer, "CLIENT_WIN", 10) == 0) {
                draw_board();
                std::cout << "Client wins!\n";
                break;
            } else if (strncmp(buffer, "DRAW", 4) == 0) {
                draw_board();
                std::cout << "It's a draw!\n";
                break;
            }
            
            // Process normal move
            int move = atoi(buffer) - 1;
            if (move < 0 || move > 8 || board[move] == 'X' || board[move] == 'O') {
                send_game_state(client_socket, "INVALID_MOVE");
                continue;
            }
            
            board[move] = 'O';
            current_player = 0;
            
            // Check for win or draw
            if (check_win('O')) {
                draw_board();
                std::cout << "Client wins!\n";
                send_game_state(client_socket, "CLIENT_WIN");
                break;
            } else if (check_draw()) {
                draw_board();
                std::cout << "It's a draw!\n";
                send_game_state(client_socket, "DRAW");
                break;
            }
            
            // Send updated board to client
            send_game_state(client_socket, "BOARD:");
            send(client_socket, board, 9, 0);
        }
    }
    
    close(client_socket);
    close(server_fd);
    return 0;
}