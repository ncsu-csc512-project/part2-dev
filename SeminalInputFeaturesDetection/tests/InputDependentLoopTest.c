#include <stdio.h>

// Function A: Takes input and returns a value that determines the loop length
int FunctionA() {
    int a;
    printf("Enter a value for Function A: ");
    scanf("%d", &a);
    
    int sumA = 0;
    for (int i = 0; i < a; i++) {
        sumA += i;  // Some operation
    }

    printf("Sum in Function A: %d\n", sumA);
    return a;
}

// Function B: Calls Function A and uses its return value for its loop
void FunctionB() {
    int b = FunctionA(); // Call Function A and get the return value

    int sumB = 0;
    for (int j = 0; j < b; j++) {
        sumB += j;  // Some operation
    }

    printf("Sum in Function B: %d\n", sumB);
}

int main() {
    FunctionB();
    return 0;
}
