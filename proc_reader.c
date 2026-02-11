#include "proc_reader.h"

int list_process_directories(void) {
    //open /proc directory using opendir() (called DIR)
    DIR *dir = opendir("/proc");
    
    //if dir failed (is null) print error
    if(dir == NULL) {
      perror("opendir");
      return -1;
    }
    
    //struct dirent pointer for directory entries
    struct dirent *entry;
    
    //Initialize process counter to 0
    int process_count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    //looping until you reach the end (NULL) 
    while((entry = readdir(dir)) != NULL) {
      //check if the name is a number using is_number()
      if(is_number(entry->d_name)) {
      //print it as a PID and increment counter
      printf("%-8s %-20s\n", entry->d_name, "process");
      process_count++;
      }
    }

    //close dir(closedir()) with check for failure (not 0)
    if(closedir(dir) != 0) {
      perror("closedir");
      return -1;
    }
    
    //printf total count of process directories found
    printf("Found %d process directories\n", process_count);
    return 0;
}

int read_process_info(const char* pid) {
    char filepath[256];

    //path to /proc/[pid]/status using snprintf()
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);

    printf("\n--- Process Information for PID %s ---\n", pid);

    //call read_file_with_syscalls() to read the status file
    //check if the function succeeded (not 0 = fail)
    if(read_file_with_syscalls(filepath) != 0) {
      fprintf(stderr, "Error: Failed to read %s\n", filepath);
      return -1;
    }

    //path to /proc/[pid]/cmdline using snprintf()
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    printf("\n--- Command Line ---\n");

    //call read_file_with_syscalls() to read the cmdline file
    //check if the function succeeded (not 0)
    if(read_file_with_syscalls(filepath) != 0) {
      fprintf(stderr, "Error: Failed to read %s\n", filepath);
      return -1;
    }

    printf("\n"); // Add extra newline for readability
    return 0;
}

int show_system_info(void) {
    int line_count = 0;
    const int MAX_LINES = 10;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);

    //open /proc/cpuinfo using fopen() with "r" mode
    FILE *file = fopen("/proc/cpuinfo", "r");
    
    // check if fopen() failed (fopen == null)
    if(file == NULL) {
      perror("fopen");
      return -1;
    }
    

    //declare a char array for reading lines
    char line[256];
    
    //read lines using fgets() in a loop, limit to MAX_LINES
    //add check if null is returned
    while(line_count < MAX_LINES && fgets(line, sizeof(line), file) != NULL) {
      // print each line
      printf("%s", line);
      line_count++;
    }
    
    //close the file using fclose()
    //check for error (not 0)
    if(fclose(file) != 0) {
      perror("fclose");
      return -1;
    }
    
    

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);

    //open /proc/meminfo using fopen() with "r" mode
    file = fopen("/proc/meminfo", "r");
    
    //check if fopen() failed (null)
    if(file == NULL) {
      perror("fopen");
      return -1;
    }
    
    //reset line_count to use again
    line_count = 0;
    
    //read lines using fgets() in a loop, limit to MAX_LINES 
    //check for not returning null
    while(line_count < MAX_LINES && fgets(line, sizeof(line), file) != NULL) {
      //print each line
      printf("%s", line);
      line_count++;
    }
    
    
    //close the file using fclose()
    //check for failure to close (not 0)
    if(fclose(file) != 0) {
      perror("fclose");
      return -1;
    }
    
    return 0;
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

int read_file_with_syscalls(const char* filename) {
    //variables: file descriptor (int), buffer (char array), bytes_read (ssize_t)
    int fd;
    char buffer[1024];
    ssize_t bytes_read;

    //open the file using open() with O_RDONLY flag
    fd = open(filename, O_RDONLY);

    //check if open() failed (fd == -1) and return -1
    if(fd == -1) {
      perror("open");
      return -1;
    }

    //read the file in a loop using read()
    //use sizeof(buffer) - 1 for buffer size to leave space for null terminator
    //check if read() returns > 0 (data was read)
    while((bytes_read = read(fd, buffer, sizeof(buffer) -1)) > 0) {
      // Null-terminate the buffer after each read
      buffer[bytes_read] = '\0';
      //print each chunk of data read
      printf("%s", buffer);
    }

    // Handle read() errors (return value -1)
    if(bytes_read == -1) {
      perror("read");
      // If read() fails, close the file and return -1
      close(fd);
      return -1;
    }
  

    //Close the file using close()
    //Check if close() failed (is -1)
    if(close(fd) == -1) {
      perror("close");
      return -1;
    }
}

int read_file_with_library(const char* filename) {
    //declare variables: FILE pointer, buffer (char array)
    FILE *file2;
    char buffer2[256];
    
    
    //open the file using fopen() with "r" mode
    file2 = fopen(filename, "r");

    //check if fopen() failed and return -1 (== null)
    if(file2 == NULL) {
      perror("fopen");
      return -1;
    }

    //Read the file using fgets() in a loop
    //continue until fgets() returns NULL
    while(fgets(buffer2, sizeof(buffer2), file2) != NULL) {
    //print each line read
    printf("%s", buffer2);
    }
    
    //close the file using fclose()
    //check if fclose() failed(not zero)
    if(fclose(file2) != 0) {
      perror("fclose");
      return -1;
    }
}

int is_number(const char* str) {
    //handle empty strings - check if str is NULL or empty strings('/0')
    if(str == NULL || *str =='\0') {
      //return 0 for empty strings
      return 0;
    }


    //check if the string contains only digits
    //loop through each character using a while loop
    while(*str) {
      //use isdigit() function to check each character
      //if any character is not a digit, return 0
      if(!isdigit((unsigned char)*str)) {
        return 0;
      }
      str++;
    }
    
    //return 1 if all characters are digits
    return 1;
}
