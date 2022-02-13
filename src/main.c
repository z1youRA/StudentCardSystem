#include <stdio.h>
#include <stdlib.h>
#define DELETED 0 //销户
#define NORMAL 1
#define NONEXIST 2 //账户不存在
#define BANNED 0 //禁用
#define LOST 2 //挂失
#define EXPDATE 20240715 //过期时间
#define FIRSTCARDNUMBER 312346 //第一张卡卡号
#define MAJORNUM 100
#define STUDENTPERMAJOR 500
#define STUDENTSNUM 50000 //学生上限
#define OK 1
#define FAILED 0

typedef struct card {
    long studentNum;
    int status;
    int balance;
    int expDate;
    int pwd;
    struct card* next;
    //#TODO类似十字链表，在窗口和卡片处均存储log
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
int cardSum = 0;
int studentSum = 0;
Student students[MAJORNUM][STUDENTPERMAJOR];//#TODO数组可直接存放指向Student的指针，以减小数组大小
int studentsPerMajor[MAJORNUM];
//所有学生状态初始化为不存在
void initStatus() {
    for(int i = 0; i < MAJORNUM; i++) {
        for(int j = 0; j < STUDENTPERMAJOR; j++) {
            students[i][j].status = NONEXIST;
        }
    }
}

//将余额转化为float格式，便于输出
float balanceToFloat(int balance) {
    float flBalance = balance / 100.0;
    return flBalance;
}

//将指向初始化后

//初始化学生结构，返回指向该学生的指针
Student* initStu(char* name, long studentNum) {
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

//通过已知信息初始化卡片并返回指针
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
    card->next = NULL;
    return card;
}

//用卡号计算出验证位并返回
int getValidDigit(long num) {
    int sum = 0;
    while(num != 0) {
        sum += num % 10;
        num /= 10;
    }
    return 9 - (sum % 10);
}

//依次序输出卡号
int cardNumberFactory() {
    int currentCardNum = FIRSTCARDNUMBER + cardSum;
    cardSum++;
    int validDigit = getValidDigit(currentCardNum);
    return currentCardNum * 10 + validDigit;
}

int getYearFromId(long studentNum) {
    return studentNum / 1000000;
}

int getMajorFromId(long studentNum) {
    return studentNum / 10000 % 100;
}

int getIndexFromId(long studentNum) {
    return studentNum / 10 % 1000;
}

//通过学号索引对应学生并返回其指针， 若学号超出范围束调用
Student* getStudent(long studentNum) {
    if(getMajorFromId(studentNum) < 0 || getMajorFromId(studentNum) >= MAJORNUM || getIndexFromId(studentNum) < 0 || getIndexFromId(studentNum) >= STUDENTPERMAJOR) {
        printf("ERROR: 学号超出范围！\n");
        exit(FAILED);
    }
    // if(students[getMajorFromId(studentNum)][getIndexFromId(studentNum)].status == NONEXIST) {
    //     printf("ERROR: 学生不存在！\n");
    //     exit(FAILED);
    // }
    return &students[getMajorFromId(studentNum)][getIndexFromId(studentNum)];
}

//手动开户操作
int openDiscount() {
    char name[20];
    long studentNum;
    Student* temp;
    printf("Input name: ");
    scanf("%s", name);
    printf("\nInput student ID: ");
    scanf("%ld", &studentNum);
    Student* target = getStudent(studentNum); //学生在学生数组中的位置指针
    //#TODO 检查输入学号年份和专业是否存在
    if(target->status == DELETED || target->status == NORMAL) {
        printf("ERROR: 该学号已开户！");
        exit(FAILED);
    }
    // for(int i = 0; students[getMajorFromId(studentNum)][i].status != NONEXIST && i < STUDENTPERMAJOR; i++) {
    //         if(students[getMajorFromId(studentNum)][i].studentNum == studentNum) {
    //         printf("学号已存在！");
    //         return FAILED;
    //     }
    // }
    temp = initStu(name, studentNum);
    *target = *temp;
    free(temp);
    return OK;
}

//为指定学生开卡
int openCard(long studentNum) {
    Student* stu = getStudent(studentNum);
    Card* card = initCard(studentNum, NORMAL, 0, EXPDATE, 8888); //默认密码8888
    if(stu->rear == NULL) { //该学生卡片数量为0
        stu->front = card;
        stu->rear = card;
        card->next = NULL;
    }
    else if(stu->rear->status == LOST){
        stu->rear->status = BANNED;
        stu->rear->next = card;
        card->balance = stu->rear->balance; //将上张卡余额转入本张卡， 上张卡balance清零
        stu->rear->balance = 0;
        stu->rear = card;
        card->next = NULL;
    }
    else if(stu->rear->status == BANNED) {
        printf("ERROR: 上张卡被禁用，开卡失败！");
        exit(FAILED);
    }
    else { // 上张卡正常使用，未挂失
        printf("卡未挂失，请先挂失后开卡！");
        return FAILED;
    }
}

int main() {
    // int abc = cardNumberFactory();
    // initCard(123, NORMAL, 100.12, EXPDATE, 8888);
    initStatus();
    openDiscount();
    openDiscount();
    openCard(2020010011);
    return 0;
}