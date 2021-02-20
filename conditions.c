#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include "conditions.h"

// http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html
int read_sensor(char* pin, char *dest, size_t size) {
  char data_stderr[1024];
  int stdout_pipe[2];
  int stderr_pipe[2];
  pid_t pid;

  // create pipe, return if unsuccessful
  if (pipe(stdout_pipe) < 0) {
    perror("stdout_pipe");
    return 1;
  }

  if (pipe(stderr_pipe) < 0) {
    perror("stderr_pipe");
    return 1;
  }

  pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(1);
  } else if (pid == 0) {
    while((dup2(stdout_pipe[1], STDOUT_FILENO) == -1) && (errno == EINTR)) { }
    while((dup2(stderr_pipe[1], STDERR_FILENO) == -1) && (errno == EINTR)) { }
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);
    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    execl("read_sensor", "read_sensor", pin, (char*)0);
    perror("execl");
    _exit(1);
  }

  close(stdout_pipe[1]);
  close(stderr_pipe[1]);

  while (1) {
    ssize_t count = read(stdout_pipe[0], dest, size);
    if (count == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        perror("read");
        exit(1);
      }
    } else if (count == 0) {
      break;
    } else {
      // successful
      break;
    }
  }

  while (1) {
    ssize_t count = read(stderr_pipe[0], data_stderr, sizeof(data_stderr));
    if (count == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        perror("read");
        exit(1);
      }
    } else if (count == 0) {
      break;
    } else {
      fprintf(stderr, "%s\n", data_stderr);
      break;
    }
  }

  close(stdout_pipe[0]);
  close(stderr_pipe[0]);

  return 0;
}

struct Conditions get_conditions(char *data) {
    char *token;
    char *rest = data;

    bool temperatureSet = false;
    bool humiditySet = false;
    struct Conditions conditions;

    while ((token = strtok_r(rest, ",", &rest))) {
      if (!temperatureSet) {
        char* temp_str = strtok(token, "temp=");
        if (temp_str != NULL) {
          conditions.temperature = atof(temp_str);
          temperatureSet = true;
          continue;
        }
      }

      if (!humiditySet) {
        char* hum_str = strtok(token, "hum=");
        if (hum_str != NULL) {
          conditions.humidity = atof(hum_str);
          humiditySet = true;
          continue;
        }
      }
    }

  return conditions;
}