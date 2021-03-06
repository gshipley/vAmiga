// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _MESSAGE_QUEUE_INC
#define _MESSAGE_QUEUE_INC

#include "AmigaObject.h"

class MessageQueue : public AmigaObject {
    
private:
    
    // Maximum number of queued messages
    const static size_t capacity = 64;
    
    // Ring buffer storing all pending messages
    Message queue[capacity];
    
    // The ring buffer's read and write pointers
    int r = 0;
    int w = 0;
    
    // Mutex for controlling parallel reads and writes
    pthread_mutex_t lock;
    
    // List of all registered listeners
    map <const void *, Callback *> listeners;
    
public:
    
    // Constructor and destructor
    MessageQueue();
    ~MessageQueue();
    
    // Registers a listener together with it's callback function.
    void addListener(const void *listener, Callback *func);
    
    // Unregisters a listener.
    void removeListener(const void *listener);
    
    // Returns the next pending message, or NULL if the queue is empty.
    Message getMessage();
    
    // Writes a message into the queue and propagates it to all listeners.
    void putMessage(MessageType type, uint64_t data = 0);
    
private:
    
    /* Propagates a single message to all registered listeners.
     * Called by function putMessage(...)
     */
    void propagateMessage(Message *msg);
};

#endif
