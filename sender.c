#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_MESSAGE_SIZE 100
#define SENDER_TYPE 1
#define RECEIVER_TYPE 2

struct Message
{
    long mtype;
    char mtext[MAX_MESSAGE_SIZE];
};

char *trim(char *str)
{
    char *end;
    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0) // All spaces?
        return str;
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    // Write new null terminator
    *(end + 1) = 0;
    return str;
}

int main()
{
    key_t key = ftok("chat_app", 65); // Unique key for message queue

    // Create the message queue for sending messages
    int sendingQueueId = msgget(key, IPC_CREAT | 0666);
    if (sendingQueueId == -1)
    {
        perror("Failed to create sending message queue");
        return 1;
    }

    // Prompt the user to enter their name
    char senderName[100];
    printf("Enter your name: ");
    if (fgets(senderName, sizeof(senderName), stdin) == NULL)
    {
        fprintf(stderr, "Failed to read name.\n");
        return 1;
    }
    senderName[strcspn(senderName, "\n")] = 0; // Remove newline

    // Create the message queue for receiving messages
    int receivingQueueId = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (receivingQueueId == -1)
    {
        perror("Failed to create receiving message queue");
        return 1;
    }

    printf("Sender ready. Enter 'quit' to exit.\n");

    while (1)
    {
        // Prompt the user to enter a message or quit
        char input[200];
        printf("Enter receiver name and message (e.g., 'jahangir-whats up?') or 'quit': ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }
        input[strcspn(input, "\n")] = 0; // Remove newline

        if (strcmp(input, "quit") == 0)
            break;

        // Find the separator
        char *separator = strchr(input, '-');
        if (separator == NULL)
        {
            fprintf(stderr, "Invalid input format. Please use the format 'receiverName-message'.\n");
            continue;
        }

        // Extract receiver name and message
        *separator = '\0'; // Split the string
        char *receiverName = trim(input);
        char *actualMessage = trim(separator + 1);

        if (strlen(receiverName) == 0 || strlen(actualMessage) == 0)
        {
            fprintf(stderr, "Invalid input format. Receiver name and message cannot be empty.\n");
            continue;
        }

        // Prepare the message
        struct Message msg;
        msg.mtype = RECEIVER_TYPE;
        snprintf(msg.mtext, MAX_MESSAGE_SIZE, "%s: %s", senderName, actualMessage);

        // Send the message to the receiver's message queue
        key_t receiverKey = ftok("chat_app", receiverName[0]);
        int receiverQueueId = msgget(receiverKey, 0666);
        if (receiverQueueId == -1)
        {
            perror("Failed to find the receiver's message queue");
            continue;
        }

        if (msgsnd(receiverQueueId, &msg, sizeof(msg.mtext), IPC_NOWAIT) == -1)
        {
            perror("Failed to send the message");
            continue;
        }

        printf("Message sent successfully!\n");
    }

    // Cleanup
    msgctl(sendingQueueId, IPC_RMID, NULL);
    msgctl(receivingQueueId, IPC_RMID, NULL);

    return 0;
}