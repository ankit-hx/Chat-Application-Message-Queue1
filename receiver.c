#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

int main()
{
    key_t key = ftok("chat_app", 65); // Unique key for message queue

    // Prompt the user to enter their name
    char receiverName[100];
    printf("Enter your name: ");
    if (fgets(receiverName, sizeof(receiverName), stdin) == NULL)
    {
        fprintf(stderr, "Failed to read name.\n");
        return 1;
    }
    receiverName[strcspn(receiverName, "\n")] = 0; // Remove newline

    // Create the message queue for receiving messages
    int receivingQueueId = msgget(key, IPC_CREAT | 0666);
    if (receivingQueueId == -1)
    {
        perror("Failed to create receiving message queue");
        return 1;
    }

    printf("Receiver ready. Waiting for messages...\n");

    while (1)
    {
        // Receive the message
        struct Message msg;
        if (msgrcv(receivingQueueId, &msg, sizeof(msg.mtext), RECEIVER_TYPE, 0) == -1)
        {
            perror("Failed to receive the message");
            continue;
        }

        // Get the current time
        time_t currentTime = time(NULL);
        struct tm *local = localtime(&currentTime);
        char timeStr[100];
        strftime(timeStr, sizeof(timeStr), "%c", local);

        // Display the received message
        printf("%s - %s\n", timeStr, msg.mtext);
    }

    return 0;
}