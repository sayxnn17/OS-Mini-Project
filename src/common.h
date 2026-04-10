#ifndef COMMON_H
#define COMMON_H

// ===== STANDARD LIBRARIES =====
#include <stdio.h>      // input/output
#include <stdlib.h>     // system(), exit()
#include <string.h>     // string operations
#include <unistd.h>     // sleep(), close()

// ===== THREADING =====
#include <pthread.h>    // multithreading

// ===== NETWORKING =====
#include <arpa/inet.h>  // sockets
#include <sys/sysinfo.h> // getloadavg, CPU cores

// ===== PORT DEFINITIONS =====
#define PORT_TCP 5000   // TCP → task execution
#define PORT_UDP 5001   // UDP → gossip communication

// ===== TIMEOUT =====
// Prevent infinite loops
#define TIMEOUT_SEC "5s"

#endif
