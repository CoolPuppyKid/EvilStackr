#pragma once
#include "imgui.h"

class Theme
{
private:
public:
    Theme();
    ~Theme();
    virtual void Apply();
};