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

// Vehículos de cosechadoras para el minijuego
new HarvesterVehicles[6];

// cosechadora4 -115.14 49.87 4.09
// cosechadora5 -119.06 38.13 4.09
// cosechadora6 -123.79 25.73 4.09


hook OnGameModeInit()
{
    // Crear cosechadoras en las posiciones predeterminadas
    HarvesterVehicles[0] = CreateVehicle(532, -103.31, 84.79, 4.09, 73.70, -1, -1, -1, false); // cosechadora 1
    HarvesterVehicles[1] = CreateVehicle(532, -99.44, 97.32, 4.08, 73.76, -1, -1, -1, false); // cosechadora 2
    HarvesterVehicles[2] = CreateVehicle(532, -95.52, 110.04, 4.09, 77.44, -1, -1, -1, false); // cosechadora 3
    HarvesterVehicles[3] = CreateVehicle(532, -114.89, 48.71, 4.09, 70.74, -1, -1, -1, false); // cosechadora 4
    HarvesterVehicles[4] = CreateVehicle(532, -120.18, 35.00, 4.09, 60.04, -1, -1, -1, false); // cosechadora 5
    HarvesterVehicles[5] = CreateVehicle(532, -124.94, 22.78, 4.09, 76.87, -1, -1, -1, false); // cosechadora 6
    
    return 1;
}
