#include <stdio.h>

int main() {
    int x;
    scanf("%d", &x);

    switch (x) {
        case 1:
            printf("One\n");
            break;
        case 2:
            printf("Two\n");
            break;
        default:
            printf("Other\n");
    }

    for (int i = 0; i < x; i++) {
        printf("%d ", i);
    }

    return 0;
}
