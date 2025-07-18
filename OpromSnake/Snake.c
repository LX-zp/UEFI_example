#include "Common.h"

extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL      gColorTable[];
extern EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL    *gPCIRootBridgeIO;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL       *gGraphicsOutput;

//------------------------- snake ------------------------------
#define X_MAP 50      //容纳多少个SnakeBlock
#define Y_MAP 50
#define SNAKEBLOCK 8
#define SNAKEBLANK 2

#define CROSSWALL 1
#define BITESELF  2
#define USEREXIT  3

#define SnakeUP     1
#define SnakeDOWN   2
#define SnakeLEFT   3
#define SnakeRIGHT  4

typedef struct GREEDSNAKE //贪吃蛇的数据结构
{
    INT32 x;
    INT32 y;
    INT32 BlockNumber; //总共X_MAP*Y_MAP个SnakeBlock
    struct GREEDSNAKE *next;
}greedsnake;

greedsnake *head,*food;  //蛇头指针，食物指针
greedsnake *pSnake; //遍历所用指针
UINT8 foodColor = BLACK;
UINT8 snakeColor = BLACK;
INT32 FoodBlocks[X_MAP*Y_MAP];  //保存可用的SnakeBlock
INT32 FoodBlockCounts;
INT32 SnakeStatus,SleepTime = 150; //运行的延时时间，ms为单位
INT32 Score=0;       //成绩，吃到一个食物则加10分
UINT8 EndGameFlag = 0;  //1 撞墙了  2 咬到自己  3 主动退出游戏
EFI_INPUT_KEY key={0,0};
INT32 flag_acc = 0;
INT32 time =0;
EFI_EVENT periodic_event[2];

UINT8 *s_crosswall = "对不起，撞到墙了，游戏结束！";
UINT8 *s_biteself =  "对不起，咬到自己了，游戏结束！";
UINT8 *s_userexit =  "您退出了游戏！";
UINT8 *s_gamescore = "您的得分是： ";
UINT8 *s_start    =  "按任意键开始";
UINT8 *s_level    =  "选择难度：1、2、3";
UINT8 *s_gamename =  "肉肉宝贝的贪吃蛇游戏";
UINT8 *s_time     =  "time：";
UINT8 *s_socre    =  "得分是：";

VOID SnakeProcess(VOID)
{
    UINT64 flag;
    flag = InintGloabalProtocols(S_TEXT_INPUT_EX | GRAPHICS_OUTPUT );
    Print(L"Welcome to the world of EDK II.\n");
    Print(L"flag=%x\n",flag);
    Delayms(2000);
restart:
    Score=0;
    flag_acc=0;
    time=0;

    SwitchGraphicsMode(TRUE);
    SetBKG(&(gColorTable[DEEPBLUE]));
    draw_string(s_start, 450, 350, &MyFontArray, &(gColorTable[WHITE]));

    //开始等待任意按键
    WaitKey();
    SetBKG(&(gColorTable[DEEPBLUE]));
    draw_string(s_level, 400, 350, &MyFontArray, &(gColorTable[WHITE]));

    Delayms(1000);
    FlushKeyBuffer();
    GetKey(&key);
    if(key.UnicodeChar=='1')
    {
        SleepTime=150;
    }
    else if(key.UnicodeChar=='2')
    {
        SleepTime=100;
    }
    else if(key.UnicodeChar=='3')
    {
        SleepTime=80;
        flag_acc=1;
    }
    else
    {
        SleepTime = 300;
    }

    TaskCreate(ReadKeyboard, (VOID*)NULL, TimerPeriodic, 10* 1000,&periodic_event[0]);
    TaskCreate(ShowTime, (VOID*)NULL, TimerPeriodic, 10 * 1000*1000,&periodic_event[1]);
    SwitchGraphicsMode(TRUE);
    SetBKG(&(gColorTable[DEEPBLUE]));
    CreateMap();
    InitSnake();
    RandomFood();
    GameRun();
    WaitKey();
    SetMyMode(0x0);
    SwitchGraphicsMode(FALSE);

    gBS->CloseEvent(periodic_event[0]);
    gBS->CloseEvent(periodic_event[1]);
    if(EndGameFlag !=USEREXIT)
        goto restart;
    return(0);
}


/** Compute a pseudo-random number.
  *
  * Compute x = (7^5 * x) mod (2^31 - 1)
  * without overflowing 31 bits:
  *      (2^31 - 1) = 127773 * (7^5) + 2836
  * From "Random number generators: good ones are hard to find",
  * Park and Miller, Communications of the ACM, vol. 31, no. 10,
  * October 1988, p. 1195.
**/
// #define RAND_MAX  0x7fffffff
INT32 robin_rand(VOID)
{
  INT32 hi, lo, x;
  static UINT32 next = 1;
  /* Can't be initialized with 0, so use another value. */
  if (next == 0)
    next = 123459876;
  hi = next / 127773;
  lo = next % 127773;
  x = 16807 * lo - 2836 * hi;
  if (x < 0)
    x += 0x7fffffff;
  return ((next = x) % ((UINT32)0x7fffffff + 1));
}

/**
  绘制Snake的元素，以8x8矩形块表示，占据10x10空间.
  
  @param  x            起始X坐标
  @param  y            起始Y坐标
  @param  ColorIndex   颜色索引
  @retval VOID              
**/
VOID SnakeElement(UINT32 x,UINT32 y,UINT8 ColorIndex)
{
  rectblock(x+ SNAKEBLANK,y+SNAKEBLANK,x+ SNAKEBLANK +SNAKEBLOCK,y+ SNAKEBLANK +SNAKEBLOCK,&(gColorTable[ColorIndex]));
}
/**
  绘制地图
  
  @param  VOID            
  @retval VOID              
**/
VOID CreateMap(VOID)
{
    INT32 xnum;
    INT32 ynum;
    INT32 counter=0;
    draw_string(s_gamename, 600, 50, &MyFontArray, &(gColorTable[WHITE]));
    for(xnum=0;xnum<X_MAP;xnum++)
    {
      SnakeElement(xnum*(SNAKEBLOCK+SNAKEBLANK),0,BLACK);
      SnakeElement(xnum*(SNAKEBLOCK+SNAKEBLANK),(Y_MAP-1)*(SNAKEBLOCK+SNAKEBLANK),BLACK);
    }

    for (ynum = 0; ynum < Y_MAP; ynum++)
    {
    SnakeElement(0, ynum*(SNAKEBLOCK + SNAKEBLANK), BLACK);
    SnakeElement((X_MAP - 1)*(SNAKEBLOCK + SNAKEBLANK), ynum*(SNAKEBLOCK + SNAKEBLANK), BLACK);
    }
    for(ynum=1;ynum<Y_MAP-1;ynum++)
      for(xnum=1;xnum<X_MAP-1;xnum++)
      {
        FoodBlocks[counter]=ynum*X_MAP + xnum;
        counter++;
      }
    FoodBlockCounts=counter;
}

/**
  初始化蛇身
  
  @param  VOID            
  @retval VOID              
**/
VOID InitSnake(VOID) 
{
    greedsnake *tail;
    INT32 i;
    tail = (greedsnake*)AllocateZeroPool(sizeof(greedsnake));//从蛇尾开始，头插法，以x,y设定开始的位置//
    tail->x = (X_MAP/2)*(SNAKEBLOCK + SNAKEBLANK);
    tail->y = (Y_MAP/2)*(SNAKEBLOCK + SNAKEBLANK);
      tail->BlockNumber = (Y_MAP/2) * X_MAP + (X_MAP/2);
    tail->next = NULL;
    
    for (i = 1; i <= 4; i++)  //4个SnakeBlock
    {
      head = (greedsnake*)AllocateZeroPool(sizeof(greedsnake));
      head->next = tail;
      head->x = (X_MAP/2)*(SNAKEBLOCK + SNAKEBLANK) + i*(SNAKEBLOCK + SNAKEBLANK);
      head->y = (Y_MAP/2)*(SNAKEBLOCK + SNAKEBLANK);
      head->BlockNumber = tail->BlockNumber + 1;
      tail = head;
    }
    while (tail != NULL)//从头到尾，输出蛇身
    {
      // Pos(tail->x, tail->y);
      // printf("■");
      SnakeElement(tail->x, tail->y,snakeColor);
      tail = tail->next;
    }
}
/**
  判断是否咬到自己
  
  @param  VOID            
  @retval 1     咬到自己
          0     没有咬到                           
**/
UINT8 BiteSelf()
{
    greedsnake *self;
    self = head->next;
    while (self != NULL)
    {
      if (self->x == head->x && self->y == head->y)
      {
        return 1;
      }
      self = self->next;
    }
    return 0;
}
/**
  随机出现食物
  
  @param  VOID            
  @retval VOID              
**/
VOID RandomFood(VOID)
{
    greedsnake *tempfood;
    INT32 randNum;

    randNum = robin_rand() % FoodBlockCounts;  //不能超过食物可出现位置的总数
    tempfood = (greedsnake*)AllocateZeroPool(sizeof(greedsnake));
    tempfood->BlockNumber = FoodBlocks[randNum];
    //递归判断蛇身与食物是否重合
    pSnake=head;
    while(pSnake == NULL)
    {
        if(pSnake->BlockNumber == FoodBlocks[randNum])  //重合了
        {
        FreePool(tempfood);  //释放内存
        RandomFood();
        }
        pSnake = pSnake->next;
    }

    tempfood->x = ((tempfood->BlockNumber) % X_MAP)  *(SNAKEBLOCK + SNAKEBLANK);
    tempfood->y = ((tempfood->BlockNumber) / X_MAP)  *(SNAKEBLOCK + SNAKEBLANK);
    tempfood->next = NULL;

    if(tempfood->x == 10)
        tempfood->x=20;
    
    if(tempfood->x == 480)
        tempfood->x=470;

    if(tempfood->y == 10)
        tempfood->y=20;
    
    if(tempfood->y == 480)
        tempfood->y=470;

    food = tempfood;
    foodColor = (UINT8)(robin_rand() % 10);  //共10个颜色可选
    if(foodColor == DEEPBLUE)
        foodColor = BLACK;
    SnakeElement(food->x,food->y,foodColor);
}
/**
  不能穿墙
  
  @param  VOID            
  @retval 1   穿墙了
          0   没有穿墙              
**/
UINT8 NotCrossWall(VOID)
{
    UINT32 BlockX,BlockY;
    BlockX = ((head->BlockNumber) % X_MAP);
    BlockY = ((head->BlockNumber) / X_MAP);
    if((BlockX==0) || (BlockX==(X_MAP-1)) || (BlockY==0) || (BlockY==(Y_MAP-1)))  
    {
        EndGameFlag =  CROSSWALL;
        return 1;
    }
    return 0;
}
/**
  蛇的移动
  
  @param  VOID            
  @retval 1     撞到自己或墙了
          0     啥事没有             
**/
UINT8 SnakeMove(VOID)
{
    greedsnake *nexthead;

    if(NotCrossWall())
        return 1;

    nexthead = (greedsnake*)AllocateZeroPool(sizeof(greedsnake));

    switch(SnakeStatus)
    {
        case SnakeUP:
        nexthead->BlockNumber = head->BlockNumber - X_MAP; 
        break;
        case SnakeDOWN:
        nexthead->BlockNumber = head->BlockNumber + X_MAP; 
        break;
        case SnakeLEFT:
        nexthead->BlockNumber = head->BlockNumber -1; 
        break;
        case SnakeRIGHT:
        nexthead->BlockNumber = head->BlockNumber +1; 
        break;
        default:
        break;
    }
    if(nexthead->BlockNumber == food->BlockNumber)  //找到食物!
    {
        nexthead->x = food->x;
        nexthead->y = food->y;
        nexthead->next=head;
        head=nexthead;
        pSnake = head; //准备遍历
        snakeColor = foodColor;
        while(pSnake != NULL)
        {
            SnakeElement(pSnake->x,pSnake->y,snakeColor);  //变成食物的颜色
            Delayms(30);
            pSnake=pSnake->next;
        }
        Score+=10;
        RandomFood();
    }
    else
    {
        nexthead->x = ((nexthead->BlockNumber) % X_MAP)  *(SNAKEBLOCK + SNAKEBLANK);
        nexthead->y = ((nexthead->BlockNumber) / X_MAP)  *(SNAKEBLOCK + SNAKEBLANK);
        nexthead->next = head;
        head = nexthead;

        pSnake = head; //准备遍历
        while(pSnake->next->next !=NULL)
        {
        SnakeElement(pSnake->x,pSnake->y,snakeColor);  
        pSnake=pSnake->next;  
        }
        SnakeElement(pSnake->next->x,pSnake->next->y,DEEPBLUE);  //消除，即变成背景色
        FreePool(pSnake->next);  //释放内存
        pSnake->next=NULL;
    }
    
    if(BiteSelf() == 1)
    {
        EndGameFlag = BITESELF;
        return 1;
    }
    return 0;
}
/**
  运行游戏
  
  @param  VOID            
  @retval VOID              
**/
VOID GameRun(VOID)
{
    SnakeStatus = SnakeRIGHT;
    while(1)
    {
        if((key.ScanCode==0x01) && (SnakeStatus!=SnakeDOWN)) //UP key
        {
        SnakeStatus=SnakeUP;
        }
        else if((key.ScanCode==0x02) && (SnakeStatus!=SnakeUP))   //DOWN key
        {
        SnakeStatus=SnakeDOWN;
        }
        else if((key.ScanCode==0x03) && (SnakeStatus!=SnakeLEFT))   //RIGHT key
        {
        SnakeStatus=SnakeRIGHT;
        }
        else if((key.ScanCode==0x04) && (SnakeStatus!=SnakeRIGHT))   //LEFT key
        {
        SnakeStatus=SnakeLEFT;
        }
        else if(key.ScanCode==0x17)   //ESC
        {
        EndGameFlag = USEREXIT;
        break;
        }
        Delayms(SleepTime);
        if(SnakeMove())
        break;
    }
    EndGame();
}
/**
  结束游戏
  
  @param  VOID            
  @retval VOID              
**/
VOID EndGame(VOID)
{
    CHAR8 buffsStr[12]={0};
    UINTN strX=20;
    UINTN strY=200;
    greedsnake *temp;

    pSnake = head; //准备遍历
    while(pSnake!=NULL)
    {
        temp = pSnake->next;
        FreePool(pSnake);  //释放内存
        pSnake = temp;
    }

    if(EndGameFlag == CROSSWALL)
        draw_string(s_crosswall, strX, strY, &MyFontArray, &(gColorTable[WHITE]));
    else if(EndGameFlag == BITESELF)
        draw_string(s_biteself, strX, strY, &MyFontArray, &(gColorTable[WHITE]));
    else if(EndGameFlag == USEREXIT)
        draw_string(s_userexit, strX, strY, &MyFontArray, &(gColorTable[WHITE]));

    draw_string(s_gamescore, strX, strY+20, &MyFontArray, &(gColorTable[WHITE]));
    AsciiSPrint(buffsStr,5,"%d",Score);
    draw_string(buffsStr, strX+108+2, strY+20, &MyFontArray, &(gColorTable[WHITE])); //108+2是计算s_gamescore长度的结果
    
    Delayms(3000);
}

EFI_STATUS
TaskCreate (
    IN  EFI_EVENT_NOTIFY NotifyFunction,
    IN  VOID             *Context,
    IN  EFI_TIMER_DELAY  Type,
    IN  UINT64           TriggerTime,
    IN  EFI_EVENT   *event
)
{
    EFI_STATUS  Status;
    Status = gBS->CreateEvent (
                        EVT_TIMER | EVT_NOTIFY_SIGNAL,
                        TPL_NOTIFY,
                        (EFI_EVENT_NOTIFY)NotifyFunction,
                        Context, 
                        event
                        );
    if (EFI_ERROR (Status)) {
        return EFI_UNSUPPORTED;
    }
    Status = gBS->SetTimer (*event, Type, TriggerTime);
    if (EFI_ERROR (Status)) {
        return EFI_UNSUPPORTED;
    }
    return Status;
}

VOID ReadKeyboard(VOID)
{
    CheckKey(&key);
}

VOID ShowTime(VOID)
{
    CHAR8 buffer1[10]={0};
    CHAR8 buffer2[10]={0};
    //每一帧时间间隔减小1ms
    if(flag_acc == 1 && SleepTime >=60)
        SleepTime = SleepTime-1;

    AsciiSPrint(buffer1,5,"%d",time);
    draw_string(s_time, 600, 100, &MyFontArray, &(gColorTable[WHITE]));
    
    rectblock(660,100,660+200,100+40, &(gColorTable[DEEPBLUE]));
    draw_string(buffer1, 660, 100, &MyFontArray, &(gColorTable[WHITE]));

    AsciiSPrint(buffer2,5,"%d",Score);
    draw_string(s_socre,600, 150, &MyFontArray, &(gColorTable[WHITE]));
    
    rectblock(670,150,670+200,150+40, &(gColorTable[DEEPBLUE]));
    draw_string(buffer2,670, 150, &MyFontArray, &(gColorTable[WHITE]));

    time++;
}