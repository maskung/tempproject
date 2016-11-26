void log_message(char *filename, char *message);
void signal_handler(int sig);
void daemonize();

#define RUNNING_DIR	"/tmp"
#define LOCK_FILE	"tempproject.lock"
#define LOG_FILE	"tempproject.log"
