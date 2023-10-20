#include <stdio.h>

void foo() {
    printf("Function foo called\n");
}

void bar() {
    printf("Function bar called\n");
}

int main() {
    void (*func_ptr)();

    int x;
    scanf("%d", &x);
    
    if (x > 10) {
        func_ptr = foo;
    } else {
        func_ptr = bar;
    }
    
    func_ptr();
    return 0;
}
