#pragma once
#include "pubSysCls.h"
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

using namespace std;
using namespace sFnd;

// FIXED: Removed the invalid parentheses from class definition
class Homer {
public: 

    int homing(int milliseconds = 20000) ;
    void home(SysManager* manager, INode* motor, const char* name, const int timeout);
        
};