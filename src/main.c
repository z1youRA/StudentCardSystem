#include "stdio.h"
#include "stdlib.h"
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
    int balance;
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
Student* initStu(char* name, int studentNum, int balance, int pwd) {
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
    stu->balance = balance;
    stu->front = NULL;
    stu->rear = NULL;
    stu->status = NORMAL;
    return stu;
}

int main() {
    return 0;
}