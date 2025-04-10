

#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H


#define CHUNK_SIZE 512
typedef char PAYLOAD[512];

typedef enum  {
    SIGNAL_CLOSE,
    SIGNAL_CLIPBOARD,
    GET_CLIPBOARD,
    SIGNAL_CLIPBOARD_SET,
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
