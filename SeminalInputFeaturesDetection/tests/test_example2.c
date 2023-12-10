#include <stdio.h>

int main() {
    char str1[1000]; 
    FILE *fp = fopen("file.txt", "r"); 
    char c;
    int len = 0;
    if (fp == NULL) {
        printf("Failed to open file\n");
        return 1;
    }
    while (1) {
        c = getc(fp);
        if (c == EOF || len >= 999) break;
        str1[len++] = c;
    }
    str1[len] = '\0'; 
    printf("%s\n", str1);
    fclose(fp); 
    return 0;
}
