#ifndef SD_H
#define SD_H
#include <Arduino.h> 
#include"average.h"

void initSDCard();
bool writeAvgToCSV(Average avg);
void initAverageFile();
Average readLastAverageData();
void deleteAverageFile();

#endif // LOGGER_H
