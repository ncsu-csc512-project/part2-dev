#include <stdio.h>
int main(){
   int id;
   int n;
   scanf("%d",  &n);
   int s = 0;
   id = n;
   for (int i=0;i<id;i++){
      s += 1;
   }
   printf("id=%d; sum=%d\n", id, n); 
}
