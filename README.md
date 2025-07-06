# Network Tic Tac Toe Game

## Table of Contents
1. [Project Description](#project-description)
2. [Features](#features)
3. [Technology Stack](#technology-stack)
4. [Installation](#installation)
5. [Prerequisites](#Prerequisites)
6. [Compilation](#Compilation)


## Project Description
A complete implementation of the classic Tic Tac Toe game using client-server architecture in C. This allows two players to compete over a network connection, with one player hosting the game (server) and another connecting to it (client).

## Features
✔️ Simple number-based input (1-9)  
✔️ Real-time board synchronization  
✔️ Win detection for all possible combinations  
✔️ Draw detection when board is full  
✔️ Clear visual board display  
✔️ Input validation  
✔️ Network error handling  

## Technology Stack
- **Programming Language**: C
- **Networking**: TCP sockets
- **Platform**: Linux/Unix systems
- **Compiler**: GCC

## Installation

### Prerequisites
- Linux/Unix operating system
- GCC compiler installed
- Basic network connectivity

### Compilation
```bash
# Compile server
gcc server.c -o server

# Compile client
gcc client.c -o client
