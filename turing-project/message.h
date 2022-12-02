typedef enum Message
{
    HELP_MESSAGE,
    LACK_FILE,
    FILE_OPEN_ERROR,
    ARG_FORMAT_ERROR,
    LACK_INPUT,
    UNKNOWN_GRAMMAR,
    SYNTAX_ERROR,
} Message;

void printMessage(Message m, Message exit_code);