#pragma once

#define static_assert _Static_assert

typedef enum
{
    WB_ASSERTION_RETRY,  /**< Retry the assert immediately. */
    WB_ASSERTION_BREAK,  /**< Make the debugger trigger a breakpoint. */
    WB_ASSERTION_ABORT,  /**< Terminate the program. */
    WB_ASSERTION_IGNORE,  /**< Ignore the assert. */
    WB_ASSERTION_ALWAYS_IGNORE  /**< Ignore the assert from now on. */
} wb_assert_state;

typedef struct wb_assert_data {
	int always_ignore;
	unsigned int trigger_count;
	const char* condition;
	const char* filename;
	int linenum;
	const char* function;
	const struct wb_assert_data* next;
} wb_assert_data;

#if defined(_MSC_VER)
extern void __cdecl __debugbreak(void);
#define wb_debug_break() __debugbreak()
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
#define wb_debug_break() __asm__ __volatile__("int $3\n\t")
#endif

#ifdef _MSC_VER
#define WB_NULL_WHILE_LOOP_CONDITION (0,0)
#else
#define WB_NULL_WHILE_LOOP_CONDITION (0)
#endif

wb_assert_state wb_report_assertion(wb_assert_data*,
                                    const char*,
                                    const char*, int);

#define wb_disabled_assert(condition) \
    do { (void) sizeof ((condition)); } while (WB_NULL_WHILE_LOOP_CONDITION)

#define wb_enabled_assert(condition)                                                                                \
	do {                                                                                                            \
		while (!(condition))                                                                                        \
		{                                                                                                           \
			static struct wb_assert_data assert_data = {															\
					0, 0, #condition, 0, 0, 0, 0};                                                                  \
			const wb_assert_state assert_state = wb_report_assertion(&assert_data, __func__, __FILE__, __LINE__);	\
			if (assert_state == WB_ASSERTION_RETRY)                                                                 \
			{                                                                                                       \
				continue; /* go again. */                                                                           \
			}                                                                                                       \
			else if (assert_state == WB_ASSERTION_BREAK)                                                            \
			{                                                                                                       \
				wb_debug_break();                                                                                   \
			}                                                                                                       \
			break; /* not retrying. */                                                                              \
		}                                                                                                           \
	} while (WB_NULL_WHILE_LOOP_CONDITION)