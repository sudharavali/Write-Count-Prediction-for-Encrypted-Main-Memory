#include<stdio.h>
#define SIZE 2000000
int main(){
        int arr[SIZE];
        int i;
        for(i = 0; i < SIZE; i++){
                arr[i] = i;
        }
        int sum = 0;
        for(i = 0; i < SIZE; i++){
                sum += arr[i];
        }
        return 0;
}

