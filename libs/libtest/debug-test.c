#include <stdio.h>
#include "../my_preprocessor.h"


void check_me(){
    if_debug {
        printf("Debugging is on!\n");
    }
    else {
        printf("Debugging is off!\n");
    }
}

int main(){
    int y = 0;
    int *x = &y;
    (__builtin_choose_expr(__type_is_void(check_me()), NULL, 42));

    if_debug
        printf("Debugging is on and macro works!\n");

    if_debug {
        int x = 3;
        printf("x in this block is equal %d\n", x);
    }

    *x = 13;
    printf("x := %d\n", *x);
    (*x) = 10;
    printf("x := %d\n", *x);

    with_debug(printf("This should ALWAYS RUN!\n"));

    check_me();

    with_debug(check_me());

}