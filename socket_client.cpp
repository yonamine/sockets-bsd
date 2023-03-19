#include <cassert>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <csignal>
#include <ctime>

#include <chrono>
#include <iostream>
#include <string>
#include <strings.h>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <unistd.h>

static constexpr size_t kMaxTries{ 5 };
static constexpr int kPortNumber{ 9091 };
static constexpr int kMaxSizeBuffer{ CHAR_MAX * 2 };
static const std::string kMessageQuit{ "quit" };

bool is_opened{ false };
bool is_running{ false };

int max_tries{ kMaxTries };
int port_number{ kPortNumber /*INT_MIN*/ };

struct sockaddr_in server_addr;
struct hostent *server{ nullptr };

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

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(sock_fd < 0) {
    PrintLn("Error on opening socket, due to '", strerror(errno), "'.");
    abort();
  }

  is_opened = true;

  server = gethostbyname("localhost");
  if (server == nullptr) {
    PrintLn("Error on retrieving hostname information, due to '", strerror(errno), "'.");
    abort();
  }
  PrintLn("Server Hostname: ", server->h_name);

  bzero((char *) &server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
  server_addr.sin_port = htons(port_number);

  if(connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    PrintLn("Error on connecting, due to '", strerror(errno), "'.");
    abort();
  }

  using namespace std::chrono_literals;

  int tries_counter{ -1 };
 
  is_running = true;
  char buffer[kMaxSizeBuffer];
  while (is_running) {
    PrintLn("Sending message to the server.");
    bzero(buffer, kMaxSizeBuffer);
    if (++tries_counter < max_tries) {
      std::time_t current_time{ std::time(nullptr) };
      std::strftime(buffer, sizeof(buffer), "%c", std::localtime(&current_time));
    }
    else {
      snprintf(buffer, kMessageQuit.length(), kMessageQuit.c_str());
      is_running = false;
    }

    ssize_t bytes_written{ write(sock_fd, buffer, strlen(buffer)) };
    if (bytes_written < 0) {
      PrintLn("Error on writing, due to '", strerror(errno), "'.");
      is_running = false;
      break;
    }

    if (is_running == false) break;

    PrintLn("Waiting message from the server.");
    bzero(buffer, kMaxSizeBuffer);
    ssize_t bytes_read{ 0 };
    // do {
      bytes_read = read(sock_fd, buffer, kMaxSizeBuffer);
      if (bytes_read < 0) {
        PrintLn("Error on reading, due to '", strerror(errno), "'.");
        is_running = false;
        break;
      }
      PrintLn("Client has received a message: '", buffer, "'.");
    // } while ((bytes_read - kMaxSizeBuffer) > 0);


    std::this_thread::sleep_for(100ms);
  }


  if (is_opened == true) {
    close(sock_fd);
    is_opened = false;
    PrintLn("Client Socket has been closed.");
  }

  PrintLn("Bye!");
  return EXIT_SUCCESS;
}
