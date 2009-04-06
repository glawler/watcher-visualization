#include <iostream>

#include "messageStream.h"
#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

int main(int argc, char **argv)
{
    TRACE_ENTER(); 

    LOAD_LOG_PROPS("log.properties"); 

    MessageStreamPtr ms=MessageStream::createNewMessageStream("glory"); 
    MessagePtr mp(new Message);

    ms->startStream(); 

    unsigned int messageNumber=0;
    while(ms->getNextMessage(mp))
        cout << "Message #" << (++messageNumber) << ": " << *mp;

    TRACE_EXIT_RET(EXIT_SUCCESS); 
    return EXIT_SUCCESS;
}
