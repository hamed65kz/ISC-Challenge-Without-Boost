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