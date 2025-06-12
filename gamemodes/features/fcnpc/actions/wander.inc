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

FCNPC_SetWander(fcnpc_id)
{
    if(FCNPC_IsValid(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC]) == false)
        return 0;

    if(FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] == E_FCNPC_STATUS:E_FCNPC_STATUS_WANDER)
        return 0;

    FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_WANDER;

    new path_id = FCNPC_CreateMovePath();
    
    if(FCNPC_IsValidMovePath(path_id) == false)
    {
        FCNPC[fcnpc_id][E_FCNPC_DATA_STATUS] = E_FCNPC_STATUS:E_FCNPC_STATUS_IDLE;
        return 0;
    }
    
    new Float:pos_x, Float:pos_y, Float:pos_z;
    FCNPC_GetPosition(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], pos_x, pos_y, pos_z);

    new Float:random_x, Float:random_y, Float:random_z;

    for(new i = 0; i < 10; i++)
    {
        random_x = pos_x + -15.0 + float(random(30)); // Random X position within -15 to +15 units
        random_y = pos_y + -15.0 + float(random(30)); // Random Y position within -15 to +15 units
        CA_FindZ_For2DCoord(random_x, random_y, random_z);

        if(random_z - pos_z > 5.0 || pos_z - random_z > 5.0) // If the Z coordinate is more than 3 units above the current position, skip this point
        {
            i++;
            continue;
        }

        FCNPC_AddPointToMovePath(path_id, random_x, random_y, random_z);
    }

    FCNPC_SetMinHeightPosCall(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], 0.1);
    FCNPC_GoByMovePath(FCNPC[fcnpc_id][E_FCNPC_DATA_FCNPC], path_id, 0, FCNPC_MOVE_TYPE_WALK, FCNPC_MOVE_SPEED_WALK, FCNPC_MOVE_MODE_AUTO, FCNPC_MOVE_PATHFINDING_AUTO);
    return 1;
}
