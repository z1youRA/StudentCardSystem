#include <stdio.h>
#include <stdlib.h>
#define DELETED 0
#define NORMAL 1
#define LOST 2
#define BANNED 0
#define EXPDATE 20240715

typedef struct card {
    long studentNum;
    int status;
    int balance;
    int expDate;
    int pwd;
} Card;

typedef struct student {
    char name[20];
    long studentNum;
    Card* front;
    Card* rear;
    int status;
} Student;

Student* front = NULL;
Student* rear = NULL;

//将余额转化为float格式，便于输出
float balanceToFloat(int balance) {
    float flBalance = balance / 100.0;
    return flBalance;
}

//初始化学生结构，返回指向该学生的指针
Student* initStu(char* name, long studentNum, int pwd) {
    Student* stu;
    int i = 0;
    stu = (Student*)malloc(sizeof(Student));
    if(stu == NULL) {
        printf("动态空间申请失败！\n");
        exit(1);
    }
    for(i = 0; name[i] != '\0'; i++) {
        stu->name[i] = name[i];
    }
    stu->name[i] = '\0';
    stu->studentNum = studentNum;
    stu->front = NULL;
    stu->rear = NULL;
    stu->status = NORMAL;
    return stu;
}

Card* initCard(long studentNum, int status, float balance,int expDate, int pwd) {
    Card* card;
    card = (Card*)malloc(sizeof(Card));
    if(card == NULL) {
        printf("动态空间申请失败!\n");
        exit(1);
    } 
    int abc = (int)(balance * 100);
    card->balance = abc;
    card->expDate = expDate;
    card->status = status;
    card->studentNum = studentNum;
    card->pwd = pwd;
    return card;
}

int main() {
    initCard(123, NORMAL, 100.12, EXPDATE, 8888);
    return 0;
}