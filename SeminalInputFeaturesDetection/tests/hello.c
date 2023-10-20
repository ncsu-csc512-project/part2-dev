#include <stdio.h>

int main() {
    int x, y;
    scanf("%d %d", &x, &y);
    if (x > y) {
        printf("x is greater\n");
    } else {
        printf("y is greater or equal\n");
    }
    return 0;
}
