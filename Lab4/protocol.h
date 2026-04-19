#include <stdint.h>

enum CommandType : int32_t {
    CONFIG = 1,
    SEND_DATA = 2,
    START_TASK = 3,
    GET_STATUS = 4,
    GET_RESULT = 5
};


enum ResponseType : int32_t {
    ACK_CONFIG = 10,
    ACK_DATA = 11,
    ACK_START = 12,
    STATUS_IN_PROGRESS = 13,
    STATUS_DONE = 14,
    ERR_BAD_DATA = 90,
    ERR_NO_DATA = 91,
    ERR_NOT_READY = 92
};

struct PacketHeader {
    int32_t command;
    int32_t payload_size;
};

struct ConfigPayload {
    int32_t matrix_size;
    int32_t threads_count;
};