#include <stdio.h>

int main() {
    int x, y, z;
    scanf("%d %d", &x, &y);

    z = x + y;

    if (z > 20) {
        printf("Sum is greater than 20\n");
    } else {
        printf("Sum is less than or equal to 20\n");
    }
    return 0;
}
