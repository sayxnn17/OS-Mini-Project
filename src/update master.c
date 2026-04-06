#include "common.h"

// Function to get the current load of a worker machine
float get_worker_load(char *ip) {
    // Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;          // IPv4 family
    addr.sin_port = htons(PORT);        // Convert port to network byte order

    // Convert IP string to binary and connect to worker
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0 ||
        connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return 999.0;  // Return high load if connection fails
    }

    // Send "LOAD" request to worker
    write(sock,  "LOAD", 4);

    // Receive response (load value)
    char res[32] = {0};
    read(sock, res, sizeof(res));

    close(sock); 

    // Convert received string to float and return
    return atof(res);
}

int main(int argc, char *argv[]) {

   // Validate argument
   if (argc < 6 || (argc - 4) % 2 != 0) {
        printf("Usage: %s <My_User> <My_IP> <User1> <IP1> ... <file.c>\n", argv[0]);
        return 1;
    }

    char *my_user = argv[1];        // Current user's username
    char *my_ip = argv[2];          // Current machine IP
    char *source_file = argv[argc - 1];  // Source file to compile

    printf("[MASTER] Compiling %s locally...\n", source_file);

    // Compile source file into executable
    char comp[256];
    snprintf(comp, sizeof(comp),
             "gcc %s -o /tmp/payload_task", source_file);

    // Execute compilation command
    if (system(comp) != 0) {
        printf("Compilation failed!\n");
        return 1;
    }

    // Initialize minimum load to a high value
    float min_load = 999.0;
    char best_ip[50], best_user[50];

    // Iterate through all workers to find least loaded one
    for (int i = 3; i < argc - 1; i += 2) {

        // Get load of worker
        float load = get_worker_load(argv[i + 1]);

        printf("[MASTER] Worker %s (%s) Load: %.2f\n",
               argv[i], argv[i + 1], load);

        // Update best worker if current has lower load
        if (load < min_load) {
            min_load = load;
            strcpy(best_user, argv[i]);
            strcpy(best_ip, argv[i + 1]);
        }
    }

    printf("[MASTER] Selected Worker: %s (%s)\n", best_user, best_ip);

    // Copy compiled executable to selected worker using SCP
    char scp_cmd[256];
    snprintf(scp_cmd, sizeof(scp_cmd),
             "scp /tmp/payload_task %s@%s:/tmp/payload_task",
             best_user, best_ip);
    system(scp_cmd);

    // Create socket to communicate with selected worker
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in w_addr;
    w_addr.sin_family = AF_INET;           // IPv4
    w_addr.sin_port = htons(PORT);         // Port number
    inet_pton(AF_INET, best_ip, &w_addr.sin_addr); // Convert IP

    // Connect to worker
    connect(sock, (struct sockaddr*)&w_addr, sizeof(w_addr));

    // Prepare RUN command with master's user and IP
    char run_msg[128];
    snprintf(run_msg, sizeof(run_msg),
             "RUN %s %s", my_user, my_ip);

    // Send RUN command to worker
    write(sock, run_msg, strlen(run_msg));
    close(sock);  // Close connection

    printf("[MASTER] Waiting for result...\n");

    sleep(5);  // Wait for the  worker to complete execution

    // Display result received from worker
    printf("\n--- RESULT FROM %s ---\n", best_ip);
    system("cat /tmp/final_output.txt");
  

    return 0;
}
