#ifndef DISPLAYAPPLICATION_H
#define DISPLAYAPPLICATION_H

// Free RTOS task junk.
extern osThreadId defaultTaskHandle;
extern osThreadId receiverTaskHandle;



extern void (*TickHook)(void);

void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);
bool extractData(char *buffer,uint32_t *phase,uint32_t *seconds, uint32_t *round,
                 uint32_t *heat,uint32_t *flight);
void extractBlink(char *buffer,bool volatile *doBlinkPtr);
void extractReverse(char *buffer,bool volatile *doInvertedPtr);
void showData(uint32_t minutes, uint32_t seconds, uint32_t round, uint32_t heat, int32_t flight, uint32_t symbol, uint32_t color);
void getInput(void);
void putPatternModulo(const uint64_t *pattern, int32_t row, int32_t column,uint8_t color);;


void animate(void);
void receiverTask(void const *argument);
void StartDefaultTask(void const *argument);
void startDisplayDriver(void);

#endif
