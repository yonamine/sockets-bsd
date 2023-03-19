#include <cassert>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <csignal>

#include <chrono>
#include <iostream>
#include <string>
#include <strings.h>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

static constexpr int kMaxConnectedClients{ 5 };
static constexpr int kMaxSizeBuffer{ CHAR_MAX * 2 };
static constexpr int kPortNumber{ 9091 };

bool is_opened{ false };
bool is_running{ false };
unsigned int client_id{ 0 };
int port_number{ kPortNumber };
socklen_t client_length{ UINT_MAX };

struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

int sock_fd{ INT_MIN };

struct sigaction signal_handler;

template <typename... Args>
void PrintLn(Args&& ...args) {
  (std::cout << ... << std::forward<Args>(args));
  std::cout << '\n';
}

static void SignalCallbackHandler(int sig_num){
  PrintLn("Signal '", sig_num,"'has caught");
  if (is_opened == true) {
    PrintLn("Closing socket...");
    close(sock_fd);
  }

  exit(EXIT_FAILURE); 
}

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int32_t 
 */
auto main([[maybe_unused]] int32_t argc, [[maybe_unused]] char **argv) -> int32_t {

  signal_handler.sa_handler = SignalCallbackHandler;
  sigemptyset(&signal_handler.sa_mask);
  signal_handler.sa_flags = 0;
  sigaction(SIGINT, &signal_handler, NULL);


  PrintLn("Opening a server socket.");
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    PrintLn("Error on opening socket, due to '", strerror(errno), "'.");
    abort();
  }

  is_opened = true;

  bzero((char *) &server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port_number);

  if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    PrintLn("Error on binding, due to '", strerror(errno), "'.");
    abort();
  }

  listen(sock_fd, kMaxConnectedClients);

  client_length = sizeof(client_addr);

  is_running = true;
  while (is_running) {
    int client_fd{ -1 };

    PrintLn("Waiting for a new client socket request connection...");
    client_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_length);
    if (client_fd < 0) {
      PrintLn("Error on accepting new socket client, due to '", strerror(errno), "'.");
      continue;
    }

    client_id++;

    PrintLn("A new client socket has connected. The client id is '", client_id, "'.");
    PrintLn("The new connection from ", inet_ntoa(client_addr.sin_addr), " port ", ntohs(client_addr.sin_port), ".");

    std::thread task([](int client_id, int client_fd) {
      using namespace std::chrono_literals;
      char input_buffer[kMaxSizeBuffer];
      char output_buffer[kMaxSizeBuffer];

      PrintLn("Task for client id '", client_id, "' has been started.");

      while (true) {

        bzero(input_buffer, kMaxSizeBuffer);

        PrintLn("Waiting message from the client.");
        ssize_t bytes_read{ read(client_fd, input_buffer, kMaxSizeBuffer) };
        if (bytes_read < 0) {
          PrintLn("Error on reading from socket, due to '", strerror(errno), "'.");
          break;
        }

        if (strlen(input_buffer) == 0) {
          PrintLn("Received an empty message. Connection was broken.");
          break;
        }
        if (strcmp(input_buffer, "quit") == 0) {
          PrintLn("The quit message has received");
          break;
        }

        PrintLn("Client #", client_id, " has received a message: '", input_buffer, "'.");

        bzero(output_buffer, kMaxSizeBuffer);
        snprintf(output_buffer, strlen(input_buffer), "%s", input_buffer);

        PrintLn("Sending message '", output_buffer,"' to the client id #", client_id, ".");
        ssize_t bytes_written{ write(client_fd, output_buffer, strlen(output_buffer)) };
        if (bytes_written < 0) {
          PrintLn("Error on writing, due to '", strerror(errno), "'.");
          break;
        }

        std::this_thread::sleep_for(100ms);
      }

      PrintLn("Client #", client_id, " has been closed");

    }, client_id, client_fd); // std::thread

    task.detach();

  }


  is_running = false;

  if (is_opened == true) {
    close(sock_fd);
    is_opened = false;
    PrintLn("Server Socket has been closed.");
  }

  PrintLn("Bye!");
  return EXIT_SUCCESS;
}
