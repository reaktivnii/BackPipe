#ifndef LOGS_H
#define LOGS_H

#include <string>

void logInfo(const std::string& msg);
void logDebug(const std::string& msg);
void logError(const std::string& msg);
void logProgress(const std::string& msg);

#endif
