#ifndef GNU_PREPROCESSOR_MAGIC_H
#define GNU_PREPROCESSOR_MAGIC_H

#ifndef __GNUC__
    #pragma error "This file should be only used with GNU Compiler Extensions"
#endif

#include <assert.h>

/*******************************************************************************
 *                                MAGIC
*******************************************************************************/
#define DO_PRAGMA(x)    _Pragma (#x)
#define MAKE_PRAGMA(x)  DO_PRAGMA(x)

#define LABEL_(x)       CONCAT_(unique_name_, x)
#define UNIQUE_NAME     LABEL_(__LINE__)

#define USE_ONCE(x)                             \
__attribute__ ((error("USE ONCE")))             \
void ___use_##x##_only_once___(void) {return;}

#ifndef SHOW_MACRO_DEBUG
    #define __NL__ 
    #define __T
    #define __NLT__
#endif


/*******************************************************************************
 *                        PREPROCESSOR UTILITIES
*******************************************************************************/
#define STR_(x) #x
#define STR(x) STR_(x)

/* Repetition of something */
#define REP0(X)
#define REP1(X) X
#define REP2(X) REP1(X) X
#define REP3(X) REP2(X) X
#define REP4(X) REP3(X) X
#define REP5(X) REP4(X) X
#define REP6(X) REP5(X) X
#define REP7(X) REP6(X) X
#define REP8(X) REP7(X) X
#define REP9(X) REP8(X) X
#define REP10(X) REP9(X) X

#define REP(HUNDREDS,TENS,ONES,X)       \
        REP##HUNDREDS(REP10(REP10(X)))  \
        REP##TENS(REP10(X))             \
        REP##ONES(X)

/* Binary literals */
#define B_0000    0
#define B_0001    1
#define B_0010    2
#define B_0011    3
#define B_0100    4
#define B_0101    5
#define B_0110    6
#define B_0111    7
#define B_1000    8
#define B_1001    9
#define B_1010    a
#define B_1011    b
#define B_1100    c
#define B_1101    d
#define B_1110    e
#define B_1111    f

#define _hex(n) 0x##n
#define hex(n) _hex(n)

#define _CCAT(a,b) a##b
#define CCAT(a,b) _CCAT(a,b)

#define b2hex(bits) B_##bits
#define binary(bitsH, bitsL) hex(CCAT(b2hex(bitsG), b2hex(bitL)))

/*******************************************************************************
                                DEBUGGING
*******************************************************************************/
#ifdef NDEBUG
    #define IS_DEBUG_ENABLED 0
    volatile _Bool __is_debugging_enabled = 0;
    #define do_if_debug(x) {}
#else
    #define IS_DEBUG_ENABLED 1
    volatile _Bool __is_debugging_enabled = 1;
    #define do_if_debug(x) {x}
#endif

#define __type_is_void(expr) __builtin_types_compatible_p(__typeof__(expr), void)
#define __expr_or_zero(expr) _Generic((typeof(expr)*){0}, void*:0, default:(expr))

#define if_debug if (__is_debugging_enabled)

#define with_debug(expr)                            \
    /* Test against void can be turned
       to test against void*         */             \
    _Generic((typeof(expr)*){0},                    \
        void*:  __with_debug_void(expr),            \
        default:__with_debug(__expr_or_zero(expr)))

#define __with_debug(expr)                          \
    ({ __is_debugging_enabled = 1;                  \
       typeof(expr) __ret; __ret = (expr);          \
       __is_debugging_enabled = IS_DEBUG_ENABLED;   \
       __ret; })

#define __with_debug_void(expr)                         \
    (void)({ __is_debugging_enabled = 1;                \
            (void)(expr);                               \
            __is_debugging_enabled = IS_DEBUG_ENABLED;  \
          })

    




/*******************************************************************************
 *                              ASSERTIONS          
*******************************************************************************/
#ifdef NDEBUG
#define assertfmt(expr, fmt, ...)   (__ASSERT_VOID_CAST(0))
#else
#define assertfmt(expr, fmt, ...)                                           \
    ((void) sizeof ((expr) ? 1 : 0), __extension__ ({                       \
        if (expr)                                                           \
            ;                                                               \
        else {                                                              \
            fprintf(stderr, fmt, __VA_ARGS__);                              \
            __assert_fail(#expr, __FILE__, __LINE__, __ASSERT_FUNCTION);    \
        }                                                                   \
    }))
#endif /* NDEBUG */



/*******************************************************************************
 *                            BIT OPERATIONS
*******************************************************************************/

#define SETBIT(bit, num)    ((num) |= (1 << ((bit) % 8)))
#define CLEARBIT(bit, num)  ((num) &= ~(1 << ((bit) % 8)))
#define CHECKBIT(bit, num)  ((num) & (1 << ((bit) % 8)))

#define ARRAYSETBIT(bit, byte_array)    \
    ((byte_array)[(bit) / 8] |= (1 << ((bit) % 8)))
#define ARRAYCLEARBIT(bit, byte_array)  \
    ((byte_array)[(bit) / 8] &= ~(1 << ((bit) % 8)))
#define ARRAYCHECKBIT(bit, byte_array)  \
    ((byte_array)[(bit) / 8] & (1 << ((bit) % 8)))

/*******************************************************************************
 *                              EXPRESSIONS
*******************************************************************************/
#define max(a, b)                   \
    __extension__                   \
    ({  __typeof__ (a) __a = (a);   \
        __typeof__ (b) __b = (b);   \
        __a < __b ? __b : __a; })

#define min(a, b)                   \
    __extension__                   \
    ({  __typeof__ (a) __a = (a);   \
        __typeof__ (b) __b = (b);   \
        __a < __b ? __a : __b; })

#define swap(a, b)                      \
    __extension__                       \
    ({  __typeof__ (a) __tmp = (b);     \
        b = a;                          \
        a = __tmp;                })

#define array_size(arr) (sizeof(arr)/sizeof(*arr))

/*******************************************************************************
                            LOGIC OPERATORS
*******************************************************************************/

#ifdef USE_CUSTOM_OPERATORS
#define not !
#define and &&
#define or ||
#endif

/*******************************************************************************
                        CONSTANT SWITCH STATEMENTS
*******************************************************************************/
// with added debugging syntax

#define SWITCH_1(func) \
case  1: {__NLT__ func( 1); __NLT__ break; __NLT__ } 
#define SWITCH_2(func) \
case  2: {__NLT__ func( 2); __NLT__ break; __NLT__ } __NLT__ SWITCH_1(func)
#define SWITCH_3(func) \
case  3: {__NLT__ func( 3); __NLT__ break; __NLT__ } __NLT__ SWITCH_2(func)
#define SWITCH_4(func) \
case  4: {__NLT__ func( 4); __NLT__ break; __NLT__ } __NLT__ SWITCH_3(func)
#define SWITCH_5(func) \
case  5: {__NLT__ func( 5); __NLT__ break; __NLT__ } __NLT__ SWITCH_4(func)
#define SWITCH_6(func) \
case  6: {__NLT__ func( 6); __NLT__ break; __NLT__ } __NLT__ SWITCH_5(func)
#define SWITCH_7(func) \
case  7: {__NLT__ func( 7); __NLT__ break; __NLT__ } __NLT__ SWITCH_6(func)
#define SWITCH_8(func) \
case  8: {__NLT__ func( 8); __NLT__ break; __NLT__ } __NLT__ SWITCH_7(func)
#define SWITCH_9(func) \
case  9: {__NLT__ func( 9); __NLT__ break; __NLT__ } __NLT__ SWITCH_8(func)
#define SWITCH_10(func)\
case 10: {__NLT__ func(10); __NLT__ break; __NLT__ } __NLT__ SWITCH_9(func)

#define CONST_SWITCH(N, x, func) switch(x) {__NLT__ SWITCH_##N(func) }

#define CONST_SWITCH_RANGE(N, start, expr, func) \
    switch ((expr) + 1 - (start)) {__NLT__ SWITCH_##N(func)}

    

#endif /* GNU_PREPROCESSOR_MAGIC_H */
