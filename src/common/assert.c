//#include "assert.h"
//#include "types.h"
//
//wb_assert_state wb_report_assertion(wb_assert_data* data, const char* func, const char* file, int line)
//{
//    wb_assert_state state = WB_ASSERTION_IGNORE;
//    static int assertion_running = 0;
//
//    static SDL_SpinLock spinlock = 0;
//    SDL_AtomicLock(&spinlock);
//    if (assertion_mutex == NULL) { /* never called SDL_Init()? */
//        assertion_mutex = SDL_CreateMutex();
//        if (assertion_mutex == NULL) {
//            SDL_AtomicUnlock(&spinlock);
//            return WB_ASSERTION_IGNORE;   /* oh well, I guess. */
//        }
//    }
//    SDL_AtomicUnlock(&spinlock);
//
//    if (SDL_LockMutex(assertion_mutex) < 0) {
//        return WB_ASSERTION_IGNORE;   /* oh well, I guess. */
//    }
//
//    /* doing this because Visual C is upset over assigning in the macro. */
//    if (data->trigger_count == 0) {
//        data->function = func;
//        data->filename = file;
//        data->linenum = line;
//    }
//
//    SDL_AddAssertionToReport(data);
//
//    assertion_running++;
//    if (assertion_running > 1) {   /* assert during assert! Abort. */
//        if (assertion_running == 2) {
//            SDL_AbortAssertion();
//        } else if (assertion_running == 3) {  /* Abort asserted! */
//            SDL_ExitProcess(42);
//        } else {
//            while (1) { /* do nothing but spin; what else can you do?! */ }
//        }
//    }
//
//    if (!data->always_ignore) {
//        state = assertion_handler(data, assertion_userdata);
//    }
//
//    switch (state)
//    {
//        case WB_ASSERTION_ALWAYS_IGNORE:
//            state = SDL_ASSERTION_IGNORE;
//            data->always_ignore = 1;
//            break;
//
//        case WB_ASSERTION_IGNORE:
//        case WB_ASSERTION_RETRY:
//        case WB_ASSERTION_BREAK:
//            break;  /* macro handles these. */
//
//        case WB_ASSERTION_ABORT:
//            SDL_AbortAssertion();
//            /*break;  ...shouldn't return, but oh well. */
//    }
//
//    assertion_running--;
//
//    SDL_UnlockMutex(assertion_mutex);
//
//    return state;
//}
