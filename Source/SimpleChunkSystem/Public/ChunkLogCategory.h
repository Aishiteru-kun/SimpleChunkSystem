#pragma once
#include "CoreMinimal.h"

#define SCHUNK_LOG(LogCategory, LogVerbosity, Format, ...) \
	UE_LOG(LogCategory, LogVerbosity, TEXT("[%s] " Format), ANSI_TO_TCHAR(__FUNCTION__), ##__VA_ARGS__)
