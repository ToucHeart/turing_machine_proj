#include <functional>
#include <regex>
#include "message.h"
#include "tm.h"
#include <unordered_map>
using namespace std;

static bool verbose = false;

static void cmdHelp()
{
    printMessage(NORMAL, NORMAL);
}

static void cmdVerbose()
{
    verbose = true;
}
static void cmdNoFile()
{
    printMessage(LACK_FILE, LACK_FILE);
}
static unordered_map<string, function<void()>> arg2Func = {
    {"-h", cmdHelp},
    {"--help", cmdHelp},
    {"-v", cmdVerbose},
    {"--verbose", cmdVerbose},
    {"", cmdNoFile},
};

int main(int argc, char *argv[])
{
    int i = 1;
    for (; i < argc; ++i)
    {
        auto it = arg2Func.find(argv[i]);
        if (it != arg2Func.end())
        {
            it->second();
            if (it->first == "-h" || it->first == "--help")
                exit(NORMAL);
        }
        else
            break;
    }

    string filename;
    if (argc == i)
        printMessage(LACK_FILE, LACK_FILE);
    else if (regex_match(argv[i], regex(".+\\.tm")))
        filename = string(argv[i++]);
    else
        printMessage(ARG_FORMAT_ERROR, ARG_FORMAT_ERROR);

    string tmInput;
    if (i == argc)
        printMessage(LACK_INPUT, LACK_INPUT);
    tmInput = string(argv[i]);

    TM *tm = new TM(filename, verbose);
    tm->run(tmInput);
    tm->printResult();
    delete tm;
    return 0;
}