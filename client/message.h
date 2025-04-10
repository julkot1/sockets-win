

#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H


#define CHUNK_SIZE 512
typedef char PAYLOAD[512];

typedef enum  {
    SIGNAL_CLOSE,
    SIGNAL_CLIPBOARD,
    SIGNAL_CLIPBOARD_SET,
    SIGNAL_MOUSE_LOCK,
    SIGNAL_MOUSE_UNLOCK,
    GET_CLIPBOARD,
    RESPONSE_OK,
    RESPONSE_FAILED
} HEADER;


typedef enum {
    NEXT_TRUE,
    NEXT_FALSE
} NEXT;

typedef struct {
    HEADER header;
    NEXT hasNext;
    PAYLOAD payload;
} MESSAGE;





#endif //SERVER_MESSAGE_H
