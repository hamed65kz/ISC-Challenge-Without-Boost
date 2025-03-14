# ISC Programming Challenge: Message Routing System

  

## System Components

  

-  **Member**

- Unique ID (1-999).

- Connects to the router via TCP.

-  **Router**

- Routes messages between members.

- Logs all sent/received messages in a file.

-  **Message**

- Fixed 32-character format:

`ID (3) | MTI (4) | Trace (6) | PAN (16) | Dest.ID (3)`

- Example Request (Member 5 → 3):

`00522001234561111111111111111003`

- Example Reply (Member 3 → 5):

`00322101234561111111111111111005`

  

## Implementation Steps

  

1.  **Setup**

- Implement members and router.

- Members connect to the router and register their IDs via TCP.

2.  **Message Routing**

- Members generate messages in the specified format.

- Router forwards messages to the `Dest.ID`.

3.  **Reply Handling**

- Destination member increments the `MTI` by 10 (e.g., `2200` → `2210`).

- Reply is sent back to the source member via the router.

4.  **Logging**

- Router logs all messages in a file.

  

## Requirements

  

- Use **CMake** for project generation.

- Code in **Modern C++ (11/14/17)**.

- External libraries are allowed.

  
  
  

## Solution

The system solution prioritizes _performance at the router_ and _simplicity at the endpoints_.

  

### Router Design

In the context of router design, we aim to efficiently manage up to 999 concurrent connections while minimizing resource overhead. To achieve this goal, I propose employing the Proactor Pattern in conjunction with Boost.Asio, utilizing a single `io_context` paired with a thread pool consisting of 4 to 8 workers.

  

This event-driven model offers several advantages:

  

1.  **Resource Efficiency**: By avoiding the need for a large number of idle threads, we can significantly reduce resource overhead. This is particularly beneficial in environments where system resources are constrained.

2.  **Scalability**: The architecture can easily scale to accommodate increased traffic or additional connections without a complete redesign.

3.  **Minimal Logic**: The Proactor Pattern is well-suited for handling simple operations asynchronously, allowing the router to process a large number of connections efficiently.

  

However, there are some trade-offs to consider:

  

-  **Increased Complexity**: Implementing an event-driven model can complicate the codebase. Developers must be diligent in managing state and handling asynchronous events, which can lead to challenges in debugging and maintaining the system.

-  **Debugging Difficulties**: The asynchronous nature of the Proactor Pattern can make it more challenging to trace the flow of execution and diagnose issues, necessitating effective logging and monitoring strategies.

  

In designing a router, it is reasonable to prioritize performance over simplicity, particularly when dealing with a high volume of concurrent connections.

  

### Node Design

In the Node, we utilize a single-threaded request-response model. The process logic is straightforward and lightweight (involving just one addition), resulting in low CPU usage and high agility. Due to this simplicity, I've chosen to maintain a single-threaded approach (primarily due to time constraints :D), which led me to disregard the use of sending and receiving queues. I believe that a blocking TCP socket with one thread per member is adequate for keeping the node structure uncomplicated. Furthermore, I haven't identified any benefits of a non-blocking mechanism compared to a blocking one. If the node isn't receiving any messages and bypasses the `recv` method, it has no tasks to perform. Consequently, non-blocking adds unnecessary complexity to the code logic.

  

### Node-Router Messaging

In a microservices environment, it is common to utilize inter-process communication (IPC) mechanisms such as gRPC and various serialization routines. However, the fixed message size and straightforward format of raw sockets often lead me to bypass these methods in favor of using direct memory access and simple socket communication. This choice can help avoid the overhead associated with gRPC and serialization, thereby enhancing performance and reducing latency for services that require quick and lightweight communication.

  

## Supported Platforms

-  **Windows**

-  **Linux** - Not Tested

  
  

## Build Routine for CMake Project

### Build Steps

  

#### 1. Clone the Repository

This project includes `spdlog` as a Git submodule, so it is necessary to clone the repository recursively to include the `spdlog` repository as well. Use the following command:

  

```bash
git  clone  --recurse-submodules  https://github.com/hamed65kz/ISC-Challenge.git
```

  

#### 2. VCPKG Setup

This project utilizes [vcpkg](https://github.com/microsoft/vcpkg) for managing external libraries. Specifically, we require `Boost.Asio` for asynchronous network operations and `gtest` for unit testing.

  

Fortunately, you do not need to install vcpkg and its dependencies manually. The project includes an automated routine that installs vcpkg and subsequently installs the dependencies listed in the `Dependencies.txt` file located in the root of the project. This routine will be triggered when you start configuring the CMake project.

  

To ensure CMake can correctly access and link to the vcpkg-installed packages, you need to set the following parameters in your CMake command or CMake preset file:

  

```plaintext

"toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
"CMAKE_PREFIX_PATH": "$env{VCPKG_ROOT}/installed/x64-windows" // Use this if the host architecture is x64

```

  

#### 3. Build the Project

As mentioned, since we are using vcpkg, it is essential to set `CMAKE_TOOLCHAIN_FILE` and `CMAKE_PREFIX_PATH` for a successful build. Below is an example build command for a Windows x64 host machine:

  

```bash

cmake  -S  .  -B  out  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"  -DCMAKE_PREFIX_PATH="%VCPKG_ROOT%/installed/x64-windows"
cd  out
cmake  --build  .  --config  Release

```

During the CMake configuration process, the vcpkg installer script will run in a separate console, pausing cmake until it finishes. This script will set up vcpkg and install the external packages specified in the Dependencies.txt file. After the installation is complete, the user should close the console to allow the CMake process to proceed with its configurations.

  

Follow these steps to successfully build the project. If you encounter any issues, please ensure that your environment variables and paths are correctly set.


## How to Run

### Running the Router Executable

To run the router executable, use the following command:

```bash
ISC-Router.exe <listen_port>
```

For example, to run the router on port 6060:

```bash
ISC-Router.exe 6060
```

### Running the Node Executable

To run the node executable, use the following command:

```bash
ISC-Node.exe <id> <dstid> <router_ip> <router_port>
```

For example, to run a node with ID 3 that communicates with destination ID 5 through the router at IP address 127.0.0.1 on port 6060, use:

```bash
ISC-Node.exe 3 5 "127.0.0.1" 6060
```
a sample running example is like:
```bash
ISC-Router.exe 6060
ISC-Node.exe 5 3 "127.0.0.1" 6060
ISC-Node.exe 3 5 "127.0.0.1" 6060
```

### Important Note

It is hard-coded in the Node application to start messaging only if the ID is set to 3. Therefore, at least one node must have an ID of 3, and this node should be started only after the remote node has been established. Node 3 sends the first message at startup, and if the remote node does not exist or does not respond, the messaging will stop.

## Demonstration

This program does amazing things. Below is a demonstration of its execution:
![Program Execution](assets/demo.gif)

## Future Works
A. The router operates on a single thread, so I need to enable a thread pool to enhance its performance.
B. The node should incorporate receive and send queues to manage bursts of traffic effectively.