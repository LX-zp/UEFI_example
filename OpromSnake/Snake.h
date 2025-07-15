#ifndef _OPROMSNAKE_H_
#define _OPROMSNAKE_H_
#include "Common.h"

INT32 robin_rand(); //robin's random function
VOID CreateMap(VOID);
VOID SnakeElement(UINT32 x,UINT32 y,UINT8 ColorIndex);
VOID InitSnake(VOID);
UINT8 BiteSelf();
VOID RandomFood(VOID);
UINT8 NotCrossWall(VOID);
UINT8 SnakeMove(VOID);
VOID GameRun(VOID);
VOID EndGame(VOID);
EFI_STATUS TaskCreate (IN  EFI_EVENT_NOTIFY NotifyFunction,IN  VOID *Context,IN  EFI_TIMER_DELAY  Type,IN    UINT64 TriggerTime,IN EFI_EVENT *event);
VOID ReadKeyboard(VOID);
VOID ShowTime(VOID);
VOID SnakeProcess(VOID);

#endif
