# Chat Application Using Message Queues in C

## Overview

This is a simple chat application implemented in C using System V message queues. The application consists of two programs: a receiver and a sender, allowing users to exchange messages in real-time. Each user runs both programs in separate terminals to participate in the chat.

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Compilation](#compilation)
- [Program Structure](#program-structure)
- [Message Queue Architecture](#message-queue-architecture)
- [How It Works](#how-it-works)
- [Usage Guide](#usage-guide)
- [Code Explanation](#code-explanation)
- [Sample Output](#sample-output)
- [Limitations](#limitations)
- [Possible Improvements](#possible-improvements)

## Features

- **Multi-user support**: Multiple users can communicate with each other
- **Timestamp display**: Each received message shows the exact time it was received
- **Simple addressing**: Send messages using the format `receiverName-message`
- **Input validation**: Handles empty messages and incorrect formats
- **Clean shutdown**: Properly removes message queues on exit
- **Cross-process communication**: Uses System V message queues for IPC

## Prerequisites

- Linux/Unix-based operating system
- GCC compiler
- Basic understanding of C programming and IPC concepts

## Compilation

Compile both programs using gcc:

```bash
# Compile the receiver program
gcc -o receiver receiver.c

# Compile the sender program
gcc -o sender sender.c
```

## Program Structure

### Files

1. **receiver.c** - Listens for and displays incoming messages
2. **sender.c** - Sends messages to other users

### Common Data Structure

Both programs use the same message structure:

```c
struct Message
{
    long mtype;              // Message type (1 for sender, 2 for receiver)
    char mtext[MAX_MESSAGE_SIZE];  // Message content
}
```

## Message Queue Architecture

The application uses two types of message queues:

1. **Receiving Queue**: Each user has a dedicated queue for receiving messages (created by receiver.c)
2. **Sending Queue**: Used for sending messages to other users

### Queue Keys

- **Receiver's Queue**: Created using `ftok("chat_app", 65)` - this is the well-known queue where all receivers listen
- **Sender's Queue**: Created with `IPC_PRIVATE` - unique for each sender
- **Target Queue**: When sending to a specific user, the sender uses `ftok("chat_app", receiverName[0])` to locate the receiver's queue

## How It Works

1. **User A (Receiver) starts:**
   - Enters their name
   - Creates a message queue with key based on "chat_app" and constant 65
   - Waits for messages of type RECEIVER_TYPE

2. **User B (Sender) starts:**
   - Enters their name
   - Creates a private queue for acknowledgments
   - Prompts for messages in format "receiverName-message"

3. **Message Flow:**
   - Sender formats message as "senderName: actualMessage"
   - Locates receiver's queue using first character of receiver's name
   - Sends message to receiver's queue
   - Receiver displays message with timestamp

## Usage Guide

### Step 1: Start the Receiver

In Terminal 1:
```bash
./receiver
Enter your name: jahangir
Receiver ready. Waiting for messages...
```

### Step 2: Start the Sender

In Terminal 2:
```bash
./sender
Enter your name: nasir
Sender ready. Enter 'quit' to exit.
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit':
```

### Step 3: Send Messages

In Terminal 2 (Sender):
```
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': jahangir-hello, how are you?
Message sent successfully!

Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': jahangir-what are you doing?
Message sent successfully!
```

### Step 4: View Received Messages

In Terminal 1 (Receiver), you'll see:
```
Enter your name: jahangir
Receiver ready. Waiting for messages...
Tue Apr  9 14:23:45 2024 - nasir: hello, how are you?
Tue Apr  9 14:24:12 2024 - nasir: what are you doing?
```

### Step 5: Exit

Type 'quit' in the sender program to exit:
```
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': quit
```

## Code Explanation

### Receiver Program (receiver.c)

1. **User Input**: Prompts for the user's name
2. **Queue Creation**: Creates a message queue using `msgget()`
3. **Infinite Loop**: Continuously waits for messages using `msgrcv()`
4. **Timestamp**: Gets current time using `time()` and formats it with `strftime()`
5. **Display**: Shows the timestamp and received message

### Sender Program (sender.c)

1. **User Input**: Gets sender's name
2. **Queue Setup**: Creates both sending and receiving queues
3. **Input Parsing**:
   - Uses `strchr()` to find the '-' separator
   - Splits input into receiver name and message
   - Trims whitespace using custom `trim()` function
4. **Message Preparation**: Formats message as "senderName: actualMessage"
5. **Queue Lookup**: Finds receiver's queue using first character of receiver's name
6. **Message Sending**: Sends using `msgsnd()`
7. **Cleanup**: Removes queues when quitting

## Sample Output

### Complete Chat Session

**Terminal 1 (Jahangir - Receiver):**
```bash
$ ./receiver
Enter your name: jahangir
Receiver ready. Waiting for messages...
Tue Apr  9 14:30:15 2024 - nasir: Hey jahangir, are you there?
Tue Apr  9 14:30:45 2024 - nasir: Let's discuss the project
Tue Apr  9 14:31:20 2024 - rahim: Meeting at 3 PM
Tue Apr  9 14:32:05 2024 - nasir: Did you see the email?
^C
```

**Terminal 2 (Nasir - Sender):**
```bash
$ ./sender
Enter your name: nasir
Sender ready. Enter 'quit' to exit.
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': jahangir-Hey jahangir, are you there?
Message sent successfully!

Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': jahangir-Let's discuss the project
Message sent successfully!

Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': rahim-Is the report ready?
Message sent successfully!

Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': jahangir-Did you see the email?
Message sent successfully!

Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': quit
```

**Terminal 3 (Rahim - Sender/Receiver):**
```bash
$ ./receiver
Enter your name: rahim
Receiver ready. Waiting for messages...
Tue Apr  9 14:31:20 2024 - nasir: Is the report ready?
^C
```

### Error Handling Examples

**Invalid Input Format:**
```
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': jahangirhello
Invalid input format. Please use the format 'receiverName-message'.
```

**Empty Fields:**
```
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': -hello
Invalid input format. Receiver name and message cannot be empty.
```

**Receiver Not Found:**
```
Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': unknown-Hello
Failed to find the receiver's message queue: No such file or directory
```

## Limitations

1. **Single Queue per User**: All messages for a user go to one queue (first character based)
2. **No Message Persistence**: Messages are lost if receiver is offline
3. **No Authentication**: Any sender can message any receiver
4. **One-way Communication**: No acknowledgment of message delivery
5. **Message Size Limit**: Limited to 100 characters
6. **Name-based Queue Lookup**: Only uses first character of name, causing collisions

## Possible Improvements

1. **Full Name-based Queues**: Use complete names or unique IDs for queue keys
2. **Message Acknowledgment**: Implement read receipts
3. **Multi-threading**: Allow sending and receiving simultaneously
4. **GUI Interface**: Create a graphical user interface
5. **Message History**: Store messages in a file/database
6. **User Discovery**: Implement a way to discover active users
7. **Encryption**: Add message encryption for security
8. **Group Chat**: Support multiple recipients
9. **File Transfer**: Add capability to send files
10. **Connection Status**: Show online/offline status of users

## Conclusion

This chat application demonstrates the fundamentals of inter-process communication using message queues in C. While simple in design, it effectively shows how processes can communicate asynchronously through the kernel. The code provides a solid foundation for understanding IPC mechanisms and can be extended to create more sophisticated communication systems.

---

**Note**: The application uses System V message queues which are persistent until explicitly removed or the system reboots. Always ensure proper cleanup by typing 'quit' in the sender program or manually removing the message queues using `ipcrm` commands if needed.