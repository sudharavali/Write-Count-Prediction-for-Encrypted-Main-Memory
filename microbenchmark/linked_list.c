#include<stdio.h>
#include<stdlib.h>

#define LIST_SIZE 100000

typedef struct node{
        int val;
        struct node* next;
}node;
int insert(node *head, int val){
        node* prev = head;
        node* next = head->next;
        while(next !=NULL){
                if(next->val == val){
                        return 0;
                }else if(next->val < val){
                        prev = next;
                        next = next->next;
                        continue;
                }else{
                       break;
                }
        }
        node* new = (node*) malloc(sizeof(node));
        if(new == NULL)
                return 0;
        new->val = val;
        prev->next = new;
        new->next = next;
        return 1;
}

int find(node *head, int val){
        node* prev = head;
        node* next = head->next;
        while(next != NULL){
                if(next->val == val){
                        return 1;
                }else if(next->val < val){
                        prev = next;
                        next = next->next;
                        continue;
                }else{
                       break;
                }
        }
        return 0;
}
int main(){

        node* head = (node*)malloc(sizeof(node));
        head->next = NULL;
        int i;
        int ret;
        for(i = 0; i < LIST_SIZE; i++){
                if(i%1000 == 0)
//                        printf("i = %d\n",i);
                ret = insert(head, i);
        }
        ret = find(head, 1000);
        printf("retval = %d\n", ret);
        return 0;

}
