#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <queue>

#define DELETED 0 //销户
#define NORMAL 1
#define NONEXIST 2             //账户不存在
#define BANNED 0               //禁用
#define LOSS 2                 //挂失
#define EXPDATE 20240715       //过期时间
#define FIRSTCARDNUMBER 312346 //第一张卡卡号
#define MAJORNUM 100
#define STUDENTPERMAJOR 500
#define STUDENTSNUM 50000 //学生上限
#define OK 1
#define FAILED 0
#define OPENACC 0
#define DELETEACC 1
#define OPENCARD 2
#define REPOLOSS 3
#define CANCELLOSS 4
#define TOPUP 5
#define PAY 6
#define EMPTY 7
#define TIMENOW 2021090309220510

using namespace std;

typedef struct card
{
    long studentNum;
    int cardNum;
    int status;
    int balance;
    int expDate;
    int pwd;
    struct card *next; //指向该学生的下一张卡
    struct card *down; //指向卡号递增的下一张卡
    //#TODO类似十字链表，在窗口和卡片处均存储log
} Card;

typedef struct opelog
{
    int type; //操作类型
    long studentNum;
    char name[20];
    int cardNum;
    int result; //操作结果
    int value;  //金额
    long time;
    struct opelog *next;
} OpeLog;

typedef struct student
{
    char name[20];
    long studentNum;
    Card *front;
    Card *rear;
    int status;
    OpeLog *head;
    OpeLog *tail;
} Student;

typedef struct window
{
    int index;
    int logQuantity;
    OpeLog *rear;
} Window;

unordered_map<long, Student*> students;
unordered_map<int, Card*> cards;
vector<OpeLog*> windowRec[100];
int cardSum = 0;
int studentSum = 0;
// Student students[MAJORNUM][STUDENTPERMAJOR]; //#TODO数组可直接存放指向Student的指针，以减小数组大小
// Card *top = NULL;                            //卡链表
int studentsPerMajor[MAJORNUM];
Window windows[100];
//所有学生状态初始化为不存在
void initStatus()
{
    // for (int i = 0; i < MAJORNUM; i++)
    // {
    //     for (int j = 0; j < STUDENTPERMAJOR; j++)
    //     {
    //         students[i][j].status = NONEXIST;
    //     }
    // }
    for (int i = 0; i < 100; i++)
    {
        windows[i].index = i + 1;
        windows[i].rear = NULL;
        windows[i].logQuantity = 0;
    }
}

//用卡号计算出验证位并返回
int getValidDigit(long num)
{
    int sum = 0;
    while (num != 0)
    {
        sum += num % 10;
        num /= 10;
    }
    return 9 - (sum % 10);
}

//依次序输出卡号
int cardNumberFactory()
{
    int currentCardNum = FIRSTCARDNUMBER + cardSum;
    cardSum++;
    int validDigit = getValidDigit(currentCardNum);
    return currentCardNum * 10 + validDigit;
}

int getYearFromId(long studentNum)
{
    return studentNum / 1000000;
}

int getMajorFromId(long studentNum)
{
    return studentNum / 10000 % 100;
}

int getIndexFromId(long studentNum)
{
    return studentNum / 10 % 1000;
}

int balanceToInt(float balance)
{
    return (int)(balance * 100);
}

//初始化学生结构，返回指向该学生的指针
Student *initStu(char *name, long studentNum)
{
    Student *stu;
    int i = 0;
    stu = (Student *)malloc(sizeof(Student));
    if (stu == NULL)
    {
        printf("动态空间申请失败！\n");
        exit(1);
    }
    for (i = 0; name[i] != '\0'; i++)
    {
        stu->name[i] = name[i];
    }
    stu->name[i] = '\0';
    stu->studentNum = studentNum;
    stu->front = NULL;
    stu->rear = NULL;
    stu->status = NORMAL;
    stu->head = NULL;
    stu->tail = NULL;
    return stu;
}

//通过已知信息初始化卡片并返回指针
Card *initCard(long studentNum, int status, float balance, int expDate, int pwd)
{
    Card *card;
    card = (Card *)malloc(sizeof(Card));
    if (card == NULL)
    {
        printf("动态空间申请失败!\n");
        exit(1);
    }
    // if (top == NULL)
    // { //卡链表为空
    //     top = card;
    //     top->down = NULL;
    // }
    // else
    // {
    //     card->down = top; //单向链表，新生成的卡片放在表头
    //     top = card;
    // }
    int abc = (int)(balance * 100);
    card->cardNum = cardNumberFactory();
    card->balance = abc;
    card->expDate = expDate;
    card->status = status;
    card->studentNum = studentNum;
    card->pwd = pwd;
    card->next = NULL;
    cards[card->cardNum] = card;    //将卡指针存入unordered_map
    return card;
}

//通过卡号查找对应卡并返回其指针，若查找失败返回NULL#TODOrewrite
Card *getCard(int cardNum)
{
    unordered_map<int, Card*>::iterator iter = cards.find(cardNum);
    if(cards.find(cardNum) == cards.end()) {
        printf("未找到该卡号，查找失败！\n");
        return NULL;
    }
    pair<int, Card*> pr = *iter;
    return pr.second;
    // Card *p = top;
    // while (p)
    // {
    //     if (p->cardNum == cardNum)
    //     {
    //         return p;
    //     }
    //     p = p->down;
    // }
    // return NULL;
}

//通过学号索引对应学生并返回其指针， 若学号超出范围exit
Student *getStudent(long studentNum)
{
    unordered_map<long, Student*>::iterator iter = students.find(studentNum);
    if(students.find(studentNum) == students.end()) {
        return NULL;
    }
    pair<long, Student*> pr = *iter;
    return pr.second;
    // if (getMajorFromId(studentNum) < 0 || getMajorFromId(studentNum) >= MAJORNUM || getIndexFromId(studentNum) < 0 || getIndexFromId(studentNum) >= STUDENTPERMAJOR)
    // {
    //     printf("ERROR: 学号超出范围！\n");
    //     exit(FAILED);
    // }
    // if(students[getMajorFromId(studentNum)][getIndexFromId(studentNum)].status == NONEXIST) {
    //     printf("ERROR: 学生不存在！\n");
    //     exit(FAILED);
    // }
}

//初始化操作日志
OpeLog *initOpeLog(int type, long studentNum, int cardNum, int result, int value, long time)
{
    OpeLog *opelog;
    opelog = (OpeLog *)malloc(sizeof(OpeLog));
    if (opelog == NULL)
    {
        printf("动态空间申请失败!\n");
        exit(1);
    }
    opelog->type = type;
    opelog->studentNum = studentNum;
    opelog->cardNum = cardNum;
    opelog->result = result;
    opelog->value = value;
    opelog->time = time;
    if(studentNum == -1) {   //若学号为-1，说明为占位所用
        strcpy(opelog->name, "EMPTY");
        return opelog;
    }
    strcpy(opelog->name, getStudent(studentNum)->name);
    return opelog;
}

//传入学生信息及日志信息，将日志保存到学生信息中
int saveOpeLogToStu(Student *stu, int type, int result, int value, long time)
{
    if (stu->tail)
    {
        stu->tail->next = initOpeLog(type, stu->studentNum, getStudent(stu->studentNum)->rear->cardNum, result, value, time);
        stu->tail = stu->tail->next;
    }
    else
    {
        printf("ERROR:保存失败!");
        exit(1);
    }
}

// //通过学号索引最新一张卡的卡号，若无卡则返回-1;
// int getCardNum(long studentNum)
// {
//     if (getStudent(studentNum)->rear)
//         return getStudent(studentNum)->rear->cardNum;
//     else
//         return -1;
// }

//手动开户操作
int openAccount(char *name, long studentNum)
{
    // char name[20];
    // long studentNum;
    Student *temp;

    /*
    #TODO 测试用，暂停键盘输入，改为直接读入数据
    printf("Input name: ");
    scanf("%s", name);
    printf("\nInput student ID: ");
    scanf("%ld", &studentNum);
    */

    Student *target = getStudent(studentNum); //学生在学生数组中的位置指针
    //#TODO 检查输入学号年份和专业是否存在
    if (target != NULL)
    {
        printf("ERROR: 该学号已开户！\n");
        exit(1);
    }
    // for(int i = 0; students[getMajorFromId(studentNum)][i].status != NONEXIST && i < STUDENTPERMAJOR; i++) {
    //         if(students[getMajorFromId(studentNum)][i].studentNum == studentNum) {
    //         printf("学号已存在！");
    //         return FAILED;
    //     }
    // }
    temp = initStu(name, studentNum);
    // *target = *temp;
    students[studentNum] = temp;    //将生成的学生指针存入unordered_map
    if (temp->head == NULL)
    {
        temp->head = initOpeLog(OPENACC, studentNum, 0, OK, 0, 0);  //生成循环链表存储日志 #TODO日志从中间位置存储
        temp->tail = temp->head;
    }
    else
    {
        printf("该学生日志不为空，日志记录失败！");
        return FAILED;
    }
    return OK;
}

//为指定学生开卡
int openCard(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        Card *card = initCard(studentNum, NORMAL, 0, EXPDATE, 8888); //默认密码8888
        if (stu->rear == NULL)
        { //该学生卡片数量为0
            stu->front = card;
            stu->rear = card;
            card->next = NULL;
            saveOpeLogToStu(stu, OPENCARD, OK, 0, 0);
        }
        else if (stu->rear->status == LOSS)
        {
            stu->rear->status = BANNED;
            stu->rear->next = card;
            card->balance = stu->rear->balance; //将上张卡余额转入本张卡， 上张卡balance清零
            stu->rear->balance = 0;
            stu->rear = card;
            card->next = NULL;
            saveOpeLogToStu(stu, OPENCARD, OK, 0, 0);
        }
        else if (stu->rear->status == BANNED)
        {
            printf("ERROR: 上张卡被禁用，开卡失败！\n");
            saveOpeLogToStu(stu, OPENCARD, FAILED, 0, 0);
            exit(FAILED);
        }
        else
        { // 上张卡正常使用，未挂失
            printf("卡未挂失，请先挂失后开卡！\n");
            saveOpeLogToStu(stu, OPENCARD, FAILED, 0, 0);
            return FAILED;
        }
    }
    else
    {
        printf("ERROR: 账户不可用，开卡失败！\n");
        saveOpeLogToStu(stu, OPENCARD, FAILED, 0, 0);
        return FAILED;
    }
}

//向学号指定的账户充值，充值成功返回OK，失败返回FAILED
int topupBalance(long studentNum, float topupAmout)
{
    Student *stu = getStudent(studentNum);
    int temp = balanceToInt(topupAmout);
    if (stu->status == NORMAL)
    {
        if (stu->rear == NULL)
        {
            printf("该学生无卡，请先开卡！\n");
            saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0);
            return FAILED;
        }
        else if (stu->rear->status == NORMAL)
        {
            if (temp <= 0)
            {
                saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0);
                printf("充值金额需大于0, 充值失败！\n");
                return FAILED;
            }
            if ((temp + stu->rear->balance) >= 100000)
            { //充值后金额大于1000元
                saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0);
                printf("卡内余额需小于1000元， 充值失败！\n");
                return FAILED;
            }
            stu->rear->balance += temp;
            saveOpeLogToStu(stu, TOPUP, OK, topupAmout, 0);
            printf("充值成功！\n");
            return OK;
        }
        else
        {
            saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0);
            printf("该学生卡已被挂失或禁用，充值失败！\n");
            return FAILED;
        }
    }
    else
    {
        saveOpeLogToStu(stu, TOPUP, FAILED, topupAmout, 0);
        printf("账户被注销或不存在，充值失败！\n");
        return FAILED;
    }
}

//挂失
int reportLoss(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        if (stu->rear == NULL)
        {
            saveOpeLogToStu(stu, REPOLOSS, FAILED, 0, 0);
            printf("该学生无卡，请先开卡！\n");
            return FAILED;
        }
        else if (stu->rear->status == NORMAL)
        {
            stu->rear->status = LOSS;
            saveOpeLogToStu(stu, REPOLOSS, OK, 0, 0);
            printf("挂失成功！\n");
            return OK;
        }
        else
        {
            saveOpeLogToStu(stu, REPOLOSS, FAILED, 0, 0);
            printf("该学生卡已被挂失或禁用，挂失失败！\n");
            return FAILED;
        }
    }
    else
    {
        saveOpeLogToStu(stu, REPOLOSS, FAILED, 0, 0);
        printf("ERROR: 账户不可用，挂失失败！\n");
        return FAILED;
    }
}

int cancelLoss(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        if (stu->rear == NULL)
        {
            saveOpeLogToStu(stu, CANCELLOSS, FAILED, 0, 0);
            printf("该学生无卡，请先开卡！\n");
            return FAILED;
        }
        else if (stu->rear->status == LOSS)
        {
            stu->rear->status = NORMAL;
            saveOpeLogToStu(stu, CANCELLOSS, OK, 0, 0);
            printf("解挂成功！\n");
            return OK;
        }
        else
        {
            printf("该学生卡未挂失或已禁用，解挂失败！\n");
            saveOpeLogToStu(stu, CANCELLOSS, FAILED, 0, 0);
            return FAILED;
        }
    }
    else
    {
        printf("ERROR: 账户不可用，解挂失败！\n");
        saveOpeLogToStu(stu, CANCELLOSS, FAILED, 0, 0);
        return FAILED;
    }
}

int deleteAccount(long studentNum)
{
    Student *stu = getStudent(studentNum);
    if (stu->status == NORMAL)
    {
        stu->status = DELETED;
        saveOpeLogToStu(stu, DELETEACC, OK, 0, 0);
        printf("账户注销成功！");
        return OK;
    }
    else
    {
        printf("ERROR: 账户不可用，注销失败！\n");
        saveOpeLogToStu(stu, DELETEACC, FAILED, 0, 0);
        return FAILED;
    }
}

//食堂支付
int pay(int cardNum, float payAmount)
{
    Card *card = getCard(cardNum);
    Student *stu = getStudent(card->studentNum);
    int payInt = balanceToInt(payAmount);
    if (stu->status == NORMAL)
    { //学生账户状态正常
        if (card->status == NORMAL)
        { //卡状态正常
            if (card->balance - payInt >= 0)
            { //卡中余额充足
                card->balance -= payInt;
                printf("支付成功！");
                saveOpeLogToStu(stu, PAY, OK, payAmount, 0);
                return OK;
            }
            else
            {
                printf("卡中余额不足，支付失败!\n");
                saveOpeLogToStu(stu, PAY, FAILED, payAmount, 0);
                return FAILED;
            }
        }
        else
        {
            printf("卡已挂失或禁用，支付失败！\n");
            saveOpeLogToStu(stu, PAY, FAILED, payAmount, 0);
            return FAILED;
        }
    }
    else
    {
        printf("卡号对应学生账户不存在或已注销，支付失败!\n");
        saveOpeLogToStu(stu, PAY, FAILED, payAmount, 0);
        return FAILED;
    }
}

//初始化窗口日志，使日志从指定位置开始存储
int initWindow(int index, int position) {
    OpeLog* log = initOpeLog(EMPTY, -1, 0, OK, 0, TIMENOW);
     if(windows[index].rear == NULL) { // 窗口日志为空， 生成一个初始日志
        windows[index].rear = log;
        log->next = log;
        windows[index].logQuantity++;
    }
    for(int i = 0; i < position - 2; i++) { // 生成position - 2个占位日志
        OpeLog* log = initOpeLog(EMPTY, -1, 0, OK, 0, TIMENOW);
        log->next = windows[index].rear->next;
        windows[index].rear->next = log;
        windows[index].rear = log;
        windows[index].logQuantity++;
    }
    return OK;
}

//模拟在窗口出消费，输入窗口序号，卡号，支付金额，时间戳
int payAtWindow(int index, int cardNum, float payAmount, int time)
{
    OpeLog *temp = initOpeLog(PAY, getCard(cardNum)->studentNum, cardNum, OK, payAmount, time);
    if (pay(cardNum, payAmount) == OK)
    {
        if (windows[index].logQuantity < 60000)
        {
            if (windows[index].rear)
            { //该窗口日志不为空
                temp->next = windows[index].rear->next;
                windows[index].rear->next = temp;
                windows[index].rear = temp;
            }
            else
            {
                windows->rear = temp;
                temp->next = temp;
            }
        }
        else  //60000条日志已满
        {
            temp = windows[index].rear->next->next;
            windows[index].rear->next = temp;
            windows[index].rear = temp;
        }
        return OK;
    }
    else
    {
        return FAILED;
    }
}

//从文件中读入信息，进行批量开户操作
int importOpenDisInfo()
{
    FILE *file = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/kh001.txt", "r"); //#TODO change to relative path
    char *num = NULL;
    char *name = NULL;
    char *ptr;
    long numLong = 0;
    if (file == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(FAILED);
    }
    char str[30];
    if (fgets(str, 30, file))
    {
        if (!strncmp(str, "KH", 2))
        { //检测验证文件正确性
            while (fgets(str, 30, file))
            { //逐行读入信息
                num = strtok(str, ",");
                name = strtok(NULL, ";");        //从str中拆分出学号和姓名两部分
                numLong = strtol(num, &ptr, 10); // str转换为long形式，便于存储
                openAccount(name, numLong);
            }
        }
        else
        {
            printf("file error\n");
            exit(1);
        }
    }
    fclose(file);
    return OK;
}

int importPositionInfo() {
    FILE *file = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/wz003.txt", "r");

    char* index = NULL;
    char* position = NULL;
    int i;
    int p;
    char str[30];
    if (file == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(1);
    }
    if(fgets(str, 10, file)) {
        if(!strncmp(str, "WZ", 2)) { //#TODO strcmp返回1
            while(fgets(str, 30, file)) {
                index = strtok(str, ",");
                position = strtok(NULL, ";");
                i = atoi(index);
                p = atoi(position);
                initWindow(i, p);
            }
        }
        else {
            printf("FILE ERROR\n");
            exit(1);
        }
    }
    return OK;
}

int importOpeInfo() {
    FILE* opeFile = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/cz002.txt", "r");
    // FILE* payFile = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/xf014.txt");
    char opeStr[50];
    char* ope = NULL;
    char *ptr;
    long stuNum;
    int type;
    long time;
    
    if (opeFile == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(1);
    }
    
    if(fgets(opeStr, 50, opeFile)) {
        if(!strncmp(opeStr, "CZ", 2)) {
            while(fgets(opeStr, 50, opeFile)) {
                time = strtol(strtok(opeStr, ","), &ptr, 10); //时间戳转为long存储
                ope = strtok(NULL, ",");
                if(!strcmp(ope, "挂失")) 
                    type = REPOLOSS;
                else if(!strcmp(ope, "解挂"))
                    type = CANCELLOSS;
                else if(!strcmp(ope, "销户"))
                    type = DELETEACC;
                else if(!strcmp(ope, "充值"))
                    type = PAY;
                else if(!strcmp(ope, "补卡"))
                    type = OPENCARD;
                else
                    printf("ERROR: 数据读取有误!\n");
                stuNum = atoi(strtok(NULL, ";"));
                OpeLog* log = initOpeLog(type, stuNum, 0, 0, 0, time);
                windowRec[0].push_back(log);
            }
        }
    }
    else {
        return FAILED;
    }
}

int importPayInfo() {
    FILE* payFile = fopen("/home/z1youra/repos/C/StudentCardSystem/testFile/xf014.txt", "r");
    char payStr[50];
    OpeLog* log = NULL;
    char *ptr, *p;
    int cardNum;
    char* time1;
    char* time2;
    int windowIndex;
    long time;
    int value;
    if (payFile == NULL)
    {
        printf("ERROR: 文件路径有误！");
        exit(1);
    }
    
    if(fgets(payStr, 50, payFile)) {
        if(!strncmp(payStr, "XF", 2)) {
            while(fgets(payStr, 50, payFile)) {
                if(payStr[0] == 'W') {
                    int i = 0;
                    while(payStr[i] != '\0') {
                        payStr[i] = payStr[i + 1];
                        i++;
                    }
                    windowIndex = strtol(payStr, &ptr, 10);
                }
                else {
                    cardNum = atoi(strtok(payStr, ","));
                    time1 = strtok(NULL, ",");
                    time2 = strtok(NULL, ",");
                    strcat(time1, time2);
                    time = strtol(time1, &ptr, 10);
                    value = balanceToInt(stof(strtok(NULL, ";")));
                    log = initOpeLog(PAY, -1, cardNum, windowIndex, value, time); //result存储windowIndex便于调用pay函数
                    windowRec[windowIndex].push_back(log);
                }
            }
        }
    }
    else {
        return FAILED;
    }
    return OK;
}

struct comp
{
    bool operator()(const struct opelog *a, const struct opelog *b) {
        // printf("%ld, %ld\n", a->time, b->time);
        return a->time < b->time;
}
};


std::vector<OpeLog*> mergesort(vector<OpeLog*> *array){
    clock_t start = clock();
    vector<OpeLog*> result;
    priority_queue<OpeLog*, std::vector<OpeLog*>, comp> pq;
    for(int i = 0; i < 100; i++){
        if(array[i].size() > 0){
            pq.push(array[i][0]);
        }
    }
    int count = 0;
    while(!pq.empty()){
        OpeLog* tmp = pq.top();
        pq.pop();
        result.push_back(tmp);
        array[tmp->result].erase(array[tmp->result].begin());
        if(array[tmp->result].size() > 0) {
            pq.push(array[tmp->result][0]);
        }
        count++;
        if (count % 10000 ==0) {
            std::cout << count << std::endl;
        }
    }
    std::cout<<"merge_k time: "<<(clock() - start)/(double)CLOCKS_PER_SEC<<"s"<<std::endl;
    cout << result.size();
    return result;
}

int main() {
    // int abc = cardNumberFactory();
    // initCard(123, NORMAL, 100.12, EXPDATE, 8888);
    initStatus();
    openAccount("a", 2020010011);
    openAccount("b", 2020010021);
    openCard(2020010011);
    openCard(2020010021);
    topupBalance(2020010011, 100);
    topupBalance(2020010011, 10000);
    payAtWindow(1, getStudent(2020010011)->rear->cardNum, 20, 0);
    initOpeLog(OPENACC, 2020010011, getStudent(2020010011)->rear->cardNum, 1, 0, 0);
    importOpenDisInfo();
    importPositionInfo();
    importOpeInfo();
    importPayInfo();
    mergesort(windowRec);
    return 0;
}