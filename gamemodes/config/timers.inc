// SAN ANDREAS ROLEPLAY
// Copyright (c) 2021 - 2025 pigeon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    // This has an explanation, y_timers is definitely bad. That's why we will use these “global” functions and hook to them.
    SetTimer("Task500", 500, true);
    SetTimer("Task1000", 1000, true);
    SetTimer("Task5000", 5000, true);
    SetTimer("Task10000", 10000, true);
    SetTimer("Task60000", 60000, true);
    // (...)
    return 1;
}

function Task500()
{
    return 1;
}

function Task1000()
{
    return 1;
}

function Task5000()
{
    return 1;
}

function Task10000()
{
    return 1;
}

function Task60000()
{
    return 1;
}