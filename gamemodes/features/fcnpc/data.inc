// SAN ANDREAS ROLEPLAY
// Copyright (c) 2021 - 2025
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

#define MAX_FCNPC (16)

enum E_FCNPC_STATUS
{
    E_FCNPC_STATUS_IDLE = 0,
    E_FCNPC_STATUS_WANDER,
    E_FCNPC_STATUS_CHASE,
    E_FCNPC_STATUS_ATTACK,
    E_FCNPC_STATUS_DEAD,
}

enum E_FCNPC_TYPE
{
    E_FCNPC_TYPE_NONE = 0,
    E_FCNPC_TYPE_DEER,
}

const E_FCNPC_DATA_MIN_TYPE = 0;
const E_FCNPC_DATA_MAX_TYPE = 1;

enum E_FCNPC_DATA
{
    E_FCNPC_DATA_FCNPC,
    E_FCNPC_TYPE:E_FCNPC_DATA_TYPE,
    E_FCNPC_STATUS:E_FCNPC_DATA_STATUS
}

new FCNPC[MAX_FCNPC][E_FCNPC_DATA];
new Iterator:FCNPC_Iterator<MAX_FCNPC>;