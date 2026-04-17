# Decentralized P2P Task Execution System

## Introduction
This project implements a decentralized peer-to-peer (P2P) system for distributed task execution. Instead of relying on a central coordinator, each node in the network independently participates in load sharing, task scheduling, and execution.

The system uses a gossip-based protocol to exchange load information and dynamically assigns tasks to the least-loaded node. It also includes mechanisms for failure detection, timeout handling, and recovery of partially executed tasks.

---

## Problem Statement
Traditional distributed systems often depend on a central server for task scheduling, which introduces:
- Single point of failure  
- Scalability limitations  
- Increased latency under load  

This project addresses these issues by designing a **fully decentralized system** that:
- Distributes decision-making  
- Balances load dynamically  
- Handles node failures gracefully  

---

## Objectives
- Design a decentralized task execution system  
- Implement dynamic load balancing using real-time metrics  
- Eliminate dependency on a central scheduler  
- Ensure fault tolerance and recovery  
- Prevent indefinite execution using timeout mechanisms  

---

## Key Features

### 1. Decentralized Architecture
Each node operates independently and participates equally in the network. No node has special privileges.

### 2. Gossip-Based Load Sharing
Nodes periodically broadcast their CPU load using UDP. This allows all nodes to maintain an approximate view of the network.

### 3. Load-Based Scheduling
Tasks are assigned to the node with the lowest current load, improving efficiency and resource utilization.

### 4. Remote Task Execution
Tasks are executed on remote nodes using TCP communication.

### 5. Secure File Transfer
Compiled binaries are transferred using SCP over SSH.

### 6. Multithreading
Each node handles multiple responsibilities concurrently:
- Sending load updates  
- Listening for other nodes  
- Handling execution requests  

### 7. Fault Tolerance
The system detects failures during execution and handles:
- Node crashes  
- Network failures  
- Execution errors  

### 8. Timeout Protection
Prevents infinite loops or long-running tasks using a timeout mechanism.

### 9. Rescue Recovery System
If a remote node fails:
- Partial progress is retrieved  
- Execution resumes locally  

---

## System Architecture

Each node runs three concurrent components:

- **Gossip Sender:** Broadcasts current CPU load using UDP every few seconds.
- **Gossip Listener:** Receives load data from other nodes and maintains the least-loaded node.
- **TCP Server:** Accepts execution requests, executes tasks, and returns output.

---

## Workflow

### Step 1: Node Initialization
```bash
./node
```
- Determines the node’s local IP address
- Starts gossip sender, listener, and TCP server
- Begins load broadcasting

### Step 2: Load Exchange (Gossip Protocol)
Nodes continuously exchange messages:
```text
LOAD:<value>
```
- Each node updates its knowledge of other nodes
- The least-loaded node is tracked dynamically

### Step 3: Task Submission
```bash
./node submit test.c
```
- Compiles the file into `/tmp/payload`
- Selects the least-loaded node

### Step 4: Task Execution Pipeline
**If remote node is selected:**
- Transfer binary using SCP
- Connect via TCP
- Send execution request

**If local node is selected:**
- Execute locally
- Output is captured and returned

### Step 5: Failure Handling (Rescue System)
If remote execution fails:
- Detect error or timeout
- Attempt to fetch `/tmp/progress.txt`
- Switch execution to local node
- Resume execution

---

## Project Structure
```text
.
├── node.c        # Core logic (networking + execution)
├── common.h      # Shared constants
├── nodes.txt     # IP to username mapping
└── test files    # Sample programs
```

---

## Setup Instructions

**1. Install Dependencies**
```bash
sudo apt update
sudo apt install gcc openssh-server -y
```

**2. Start SSH Service**
```bash
sudo systemctl start ssh
```

**3. Configure SSH Access**
```bash
ssh-keygen
ssh-copy-id username@IP
```

**4. Configure Nodes**
Create a `nodes.txt` file:
```text
<IP_ADDRESS> <USERNAME>
```

**5. Compile**
```bash
gcc node.c -o node -pthread
```

**6. Run Node**
```bash
./node
```

**7. Submit Task**
```bash
./node submit test.c
```

---

## Test Programs

**Normal Program**
```c
#include <stdio.h>
int main(){ printf("Hello System\n"); }
```

**Infinite Loop (Timeout Test)**
```c
int main(){ while(1){} }
```

**Crash Test**
```c
int main(){ int *p=NULL; *p=5; }
```

**Heavy CPU Task**
```c
int main(){ for(long long i=0;i<1e10;i++); }
```

---

## Design Decisions
- **UDP for gossip:** Lightweight and fast
- **TCP for execution:** Reliable communication
- **SCP for transfer:** Avoids custom protocols
- **Multithreading:** Parallel processing
- **Mutex locks:** Thread safety
- **Timeout:** Prevents system hang

---

## Limitations
- Works only in LAN environments
- Requires SSH setup
- No centralized monitoring
- No persistent task queue
- Limited recovery (basic progress handling)

---

## Future Improvements
- Custom file transfer mechanism
- Distributed task queue
- Authentication and security enhancements
- WAN support (NAT traversal)
- Monitoring dashboard

---

## Conclusion
This project demonstrates a decentralized approach to distributed task execution. By combining gossip-based load sharing, dynamic scheduling, and fault tolerance, it achieves efficient and reliable execution without a central coordinator.

**One-Line Summary:** Decentralized load-balanced task execution using gossip protocol, TCP communication, and fault-tolerant recovery.
