# ISC Programming Challenge: Message Routing System

## System Components

- **Member**  
  - Unique ID (1-999).  
  - Connects to the router via TCP.  
- **Router**  
  - Routes messages between members.  
  - Logs all sent/received messages in a file.  
- **Message**  
  - Fixed 32-character format:  
    `ID (3) | MTI (4) | Trace (6) | PAN (16) | Dest.ID (3)`  
  - Example Request (Member 5 → 3):  
    `00522001234561111111111111111003`  
  - Example Reply (Member 3 → 5):  
    `00322101234561111111111111111005`  

## Implementation Steps

1. **Setup**  
   - Implement members and router.  
   - Members connect to the router and register their IDs via TCP.  
2. **Message Routing**  
   - Members generate messages in the specified format.  
   - Router forwards messages to the `Dest.ID`.  
3. **Reply Handling**  
   - Destination member increments the `MTI` by 10 (e.g., `2200` → `2210`).  
   - Reply is sent back to the source member via the router.  
4. **Logging**  
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

1. **Resource Efficiency**: By avoiding the need for a large number of idle threads, we can significantly reduce resource overhead. This is particularly beneficial in environments where system resources are constrained.
    
2. **Scalability**: The architecture can easily scale to accommodate increased traffic or additional connections without a complete redesign.
    
3. **Minimal Logic**: The Proactor Pattern is well-suited for handling simple operations asynchronously, allowing the router to process a large number of connections efficiently.

However, there are some trade-offs to consider:

- **Increased Complexity**: Implementing an event-driven model can complicate the codebase. Developers must be diligent in managing state and handling asynchronous events, which can lead to challenges in debugging and maintaining the system.
    
- **Debugging Difficulties**: The asynchronous nature of the Proactor Pattern can make it more challenging to trace the flow of execution and diagnose issues, necessitating effective logging and monitoring strategies.

In designing a router, it is reasonable to prioritize performance over simplicity, particularly when dealing with a high volume of concurrent connections.

### Node Design
In the Node, we utilize a single-threaded request-response model. The process logic is straightforward and lightweight (involving just one addition), resulting in low CPU usage and high agility. Due to this simplicity, I've chosen to maintain a single-threaded approach (primarily due to time constraints :D), which led me to disregard the use of sending and receiving queues. I believe that a blocking TCP socket with one thread per member is adequate for keeping the node structure uncomplicated. Furthermore, I haven't identified any benefits of a non-blocking mechanism compared to a blocking one. If the node isn't receiving any messages and bypasses the `recv` method, it has no tasks to perform. Consequently, non-blocking adds unnecessary complexity to the code logic.

### Node-Router Messaging
In a microservices environment, it is common to utilize inter-process communication (IPC) mechanisms such as gRPC and various serialization routines. However, the fixed message size and straightforward format of raw sockets often lead me to bypass these methods in favor of using direct memory access and simple socket communication. This choice can help avoid the overhead associated with gRPC and serialization, thereby enhancing performance and reducing latency for services that require quick and lightweight communication.



