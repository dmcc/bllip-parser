/*
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <pthread.h>
#include "ThreadManager.h"
#include "Feature.h"

static pthread_mutex_t threadBookkeepingLock = PTHREAD_MUTEX_INITIALIZER;
// the above lock is for the following structure:
bool threadSlotsUsed[MAXNUMTHREADS];

/* Upon construction, we attempt to acquire a thread slot. */
ThreadSlot::ThreadSlot() {
    threadSlotIndex = -1;
    acquire();
}

/* Recycles this thread slot upon destruction. */
ThreadSlot::~ThreadSlot() {
    recycle();
}

/* Attempts to acquire a thread slot.  Returns true if acquisition was successful. */
bool ThreadSlot::acquire() {
    pthread_mutex_lock(&threadBookkeepingLock);
    for (int slotIndex = 0; slotIndex < MAXNUMTHREADS; slotIndex++) {
        if (!threadSlotsUsed[slotIndex]) {
            threadSlotIndex = slotIndex;
            threadSlotsUsed[slotIndex] = true;
            break;
        }
    }
    pthread_mutex_unlock(&threadBookkeepingLock);

    return acquiredThreadSlot();
}

/* This will return our thread slot to the pool.  This thread will no longer be usable for parsing. */
void ThreadSlot::recycle() {
    pthread_mutex_lock(&threadBookkeepingLock);
    threadSlotsUsed[threadSlotIndex] = false;
    threadSlotIndex = -1;
    pthread_mutex_unlock(&threadBookkeepingLock);
}

/* Returns true if we were able to acquire a thread slot. */
bool ThreadSlot::acquiredThreadSlot() {
    return threadSlotIndex != -1;
}

/* Returns the internal thread slot index. */
int ThreadSlot::getThreadSlotIndex() {
    return threadSlotIndex;
}
