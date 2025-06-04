/*

  Creator: plaYmaker (from Portugal!)

  Shapes made easier!

  Maintainer: Kar

*/

#if defined _graphic_included
        #endinput
#endif
#define _graphic_included

#tryinclude <a_samp>

// Natives, function list.
/*
	native Graph:GRAPHIC::Create(Float:x, Float:y, Float:x_min, Float:y_min, Float:x_max, Float:y_max);
	native GRAPHIC::XYAxisColor(Graph:_id, _x_color, _y_color);
	native GRAPHIC::UseBackground(Graph:_id, use);
	native GRAPHIC::BackgroundColor(Graph:_id, color);
	native GRAPHIC::GRAPHIC::AddPoint(Graph:_id, Float:x, Float:y, color);
	native GRAPHIC::ShowForPlayer(playerid, Graph:_id);
	native GRAPHIC::HideForPlayer(playerid, Graph:_id);
	native GRAPHIC::ShowForAll(Graph:_id);
	native GRAPHIC::HideForAll(Graph:_id);
	native GRAPHIC::Update(Graph:_id, playerid = INVALID_PLAYER_ID);
	native GRAPHIC::Destroy(Graph:_id);
	native GRAPHIC::OtherXYAxis(oper, playerid, Graph:_id, xAxis, yAxis);
*/

// Include's version.
#define _graphic_version			(0x01B)
#define GRAPHIC::               	GRAPHIC_
// Copy of an 'INVALID' inside a_samp.inc, with the tag 'Graph:'.
#define INVALID_GRAPHIC_ID          (Graph:-1)
// Thanks to Y_Less.
// macro @ (absolute number)
#define abs(%0) 					(((%0) < 0)?(-(%0)):((%0)))
// This macro..
// verify if %0 (probably an id) is different from -1 (INVALID_GRAPHIC_ID) and
// if it's created.
#define isValid(%0)					(%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][GRAPHIC::_crt_grph])
// verify if both x(x_min + x_max) %4(or/and/..) y(y_min + y_max) values are 0
#define __check:%4(<%0,%1>,<%2,%3>) (%0 >= 0 && %1 <= 0) %4 (%2 >= 0 && %3 <= 0)

// Use this callback to setup your graphic settings.
forward 							GRAPHIC::Init();

enum
	GRAPHIC::e_Graphic
{
	  Float:GRAPHIC::_x          // x-Axis textdraw coord.
	, Float:GRAPHIC::_x_min      // Minimum x-Axis.
	, Float:GRAPHIC::_x_max  	// Maximum x-Axis.
	, Float:GRAPHIC::_y      	// y-Axis textdraw coord.
	, Float:GRAPHIC::_y_min  	// Minimum y-Axis.
	, Float:GRAPHIC::_y_max  	// Maximum y-Axis.
	, Text:GRAPHIC::_td_0    	// Textdraw for x-Axis.
	, Text:GRAPHIC::_td_1    	// Textdraw for y-Axis.
	, Text:GRAPHIC::_td_box  	// Textdraw for background.
	, bool:GRAPHIC::_crt_grph    // Is Graphic created ?
	, bool:GRAPHIC::_crt_box     // Is background created ?
}
stock
	  Gr@phic[MAX_TEXT_DRAWS][GRAPHIC::e_Graphic] // enum
	, Text:__td[MAX_TEXT_DRAWS] = { Text:INVALID_TEXT_DRAW, ... } // GRAPHIC::AddPoint textdraw. (global)
	, _td = -1 // GRAPHIC::AddPoint textdraw id. (each point)
;

/*
    Graph:GRAPHIC::Create:
		Create a graphic textdraw.

    Arguments:
		Float:x         x-Axis textdraw coord.
		Float:y      	y-Axis textdraw coord.
        Float:x_min    	Minimum x-Axis.
        Float:y_min  	Minimum y-Axis
        Float:x_max  	Maximum x-Axis.
        Float:y_max  	Maximum y-Axis.

	Returns:
		Graphic id. (Graph:_id)
        
    Notes:
		-
*/
stock
	Graph:GRAPHIC::Create(Float:x, Float:y, Float:x_min, Float:y_min, Float:x_max, Float:y_max)
{
	// This var is using to check what is the next id empty
	// that could be used to identify the 'graphic'.
	new
		_id = 0;

	// Checking system.
	do
	{
	    if (!Gr@phic[_id][GRAPHIC::_crt_grph])
		{
			break;
		}
	    ++_id;
	}
	while (_id < sizeof Gr@phic);
	
	// Restrictions
	if (x_min > 0 || y_min > 0 || x_max < 0 || y_max < 0)
 	{
	    // Returning INVALID_GRAPHIC_ID. (Graph:-1)
		return INVALID_GRAPHIC_ID;
	}
	else
	{
	    if(!(__check:||(<x_min,x_max>,<y_min,y_max>)))
	    {
		    // Storing values that are useful in other functions.
			Gr@phic[_id][GRAPHIC::_x] = x;
			Gr@phic[_id][GRAPHIC::_x_min] = x_min;
			Gr@phic[_id][GRAPHIC::_x_max] = x_max;

			Gr@phic[_id][GRAPHIC::_y] = y;
			Gr@phic[_id][GRAPHIC::_y_min] = y_min;
			Gr@phic[_id][GRAPHIC::_y_max] = y_max;

			Gr@phic[_id][GRAPHIC::_crt_grph] = true;
			
			// Creating the x-Axis textdraw.
			Gr@phic[_id][GRAPHIC::_td_0] = TextDrawCreate(x + x_max + 4, y - 1, "_");
			TextDrawLetterSize(Gr@phic[_id][GRAPHIC::_td_0], 0, -0.2);
			TextDrawTextSize(Gr@phic[_id][GRAPHIC::_td_0], x + x_min + 2, 0);
			TextDrawUseBox(Gr@phic[_id][GRAPHIC::_td_0], true);
			
			// This one is used to ajust some coords for the y-Axis textdraw.
			new
				Float:aj[2] = {0.8, 0.0};
			if (y_min == 0 && y_max > 0 && x_min < 0)
			{
				aj[0] = 0.0;
			}
			else if (y_min == 0 && y_max > 0 && x_max > 0)
			{
				aj[0] = 0.3;
			}
			else if (y_min < 0 && y_max == 0)
			{
				aj[0] = 0.5;
				aj[1] = 0.3;
			}
			
	        // Creating the y-Axis textdraw.
			Gr@phic[_id][GRAPHIC::_td_1] = TextDrawCreate(x + 4, y - y_max - aj[1], "_");
			TextDrawLetterSize(Gr@phic[_id][GRAPHIC::_td_1], 0, abs(y_min / 10) + (y_max / 10) + aj[0] - aj[1]);
			TextDrawTextSize(Gr@phic[_id][GRAPHIC::_td_1], x + 2, 0);
			TextDrawUseBox(Gr@phic[_id][GRAPHIC::_td_1], true);
		}
	}
	// Returning the id used to identify the graphic.
	return Graph:_id;
}

/*
    GRAPHIC::XYAxisColor:
		Modify the color of x and y-Axis textdraw.

    Arguments:
		Graph:_id       Graphic id.
		_x_color     	x-Axis textdraw color.
        _y_color    	y-Axis textdraw color.

	Returns:
		1 on success, 0 on failure.

    Notes:
		-
*/
stock
	GRAPHIC::XYAxisColor(Graph:_id, _x_color, _y_color)
{
	// Checking if its valid. (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph])
	if (isValid(_id))
	{
	    // Modifying the colors.
		TextDrawBoxColor(Gr@phic[_:_id][GRAPHIC::_td_0], _x_color); // x-Axis textdraw color.
		TextDrawBoxColor(Gr@phic[_:_id][GRAPHIC::_td_1], _y_color); // y-Axis textdraw color.
		// Returning 1. success
	    return 1;
	}
	// Returning 0. failure
	return 0;
}

/*
    GRAPHIC::UseBackground:
		Add a background to the graphic textdraw.

    Arguments:
		Graph:_id       Graphic id.
		use             1 for use it, 0 for not to use it.

	Returns:
		1 on success, 0 on failure.

    Notes:
		- The default 'use' number is 0. When you create a graphic, noone background is created too.
*/
stock
	GRAPHIC::UseBackground(Graph:_id, use)
{
	// Simple int-bool conversion.
	// *1 NO NEED !
	/*
	new
		bool:__my_bool[4];
	__my_bool[0] = !!Gr@phic[_:_id][GRAPHIC::_x_min];
	__my_bool[1] = !!Gr@phic[_:_id][GRAPHIC::_y_min];
	__my_bool[2] = !!Gr@phic[_:_id][GRAPHIC::_x_max];
	__my_bool[3] = !!Gr@phic[_:_id][GRAPHIC::_y_max];
	*/
    // Checking if its valid. ' (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph]) ',
    // if 'use' is diferent from 0. (if seriously the function needs to create the background textdraw),
    // if noone background was previously created and finally
    // if Gr@phic does NOT have (x_min, y_min, x_max and y_max) 0 as values stored in, all of them. *1
	if (isValid(_id) && use != 0 && !Gr@phic[_:_id][GRAPHIC::_crt_box] && !(__check:&&(<Gr@phic[_:_id][GRAPHIC::_x_min],Gr@phic[_:_id][GRAPHIC::_x_max]>,<Gr@phic[_:_id][GRAPHIC::_y_min],Gr@phic[_:_id][GRAPHIC::_y_max]>)))
	{
	    if(!(__check:||(<Gr@phic[_:_id][GRAPHIC::_x_min],Gr@phic[_:_id][GRAPHIC::_x_max]>,<Gr@phic[_:_id][GRAPHIC::_y_min],Gr@phic[_:_id][GRAPHIC::_y_max]>)))
	    {
		    // This one is used to ajust some coords because of the y-Axis textdraw.
		    new
		        Float:aj[2] = {0.0, 0.0};
			if (Gr@phic[_:_id][GRAPHIC::_y_min] == 0 && Gr@phic[_:_id][GRAPHIC::_y_max] > 0)
			{
				aj[0] = -(Gr@phic[_:_id][GRAPHIC::_y_max]);
				aj[1] = 5;
			}
			else if (Gr@phic[_:_id][GRAPHIC::_y_min] < 0 && Gr@phic[_:_id][GRAPHIC::_y_max] == 0)
			{
				aj[0] = -(Gr@phic[_:_id][GRAPHIC::_y_min]);
				aj[1] = 5;
			}
		    // Creating the background textdraw.
			Gr@phic[_:_id][GRAPHIC::_td_box] = TextDrawCreate(Gr@phic[_:_id][GRAPHIC::_x] + Gr@phic[_:_id][GRAPHIC::_x_max] + 6, Gr@phic[_:_id][GRAPHIC::_y] + Gr@phic[_:_id][GRAPHIC::_y_min] - 3 + aj[0], "_");
			TextDrawLetterSize(Gr@phic[_:_id][GRAPHIC::_td_box], 0, (-Gr@phic[_:_id][GRAPHIC::_y_min] + Gr@phic[_:_id][GRAPHIC::_y_max] + 13 - aj[1]) / 10);
			TextDrawTextSize(Gr@phic[_:_id][GRAPHIC::_td_box], Gr@phic[_:_id][GRAPHIC::_x] + Gr@phic[_:_id][GRAPHIC::_x_min], 0);
			TextDrawUseBox(Gr@phic[_:_id][GRAPHIC::_td_box], 1);
	        // Storing value that is useful in other functions.
	        Gr@phic[_:_id][GRAPHIC::_crt_box] = true;
	        // Returning 1. success
		    return 1;
		}
	}
	// Returning 0. failure
	return 0;
}

/*
    GRAPHIC::BackgroundColor:
		Modify background color.

    Arguments:
		Graph:_id       Graphic id.
		color           Background color.

	Returns:
		1 on success, 0 on failure.

    Notes:
		-
*/
stock
	GRAPHIC::BackgroundColor(Graph:_id, color)
{
    // Checking if its valid.  (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph])  and
    // if the id matched with a graphic is previosly created. (Gr@phic[_:_id][_crt_box])
	if (isValid(_id) && Gr@phic[_:_id][GRAPHIC::_crt_box])
	{
	    // Modifying the box color.
		TextDrawBoxColor(Gr@phic[_:_id][GRAPHIC::_td_box], color);
  		// Returning 1. success
	    return 1;
	}
	// Returning 0. failure
	return 0;
}

/*
    GRAPHIC::AddPoint:
		Add a point to the graphic.

    Arguments:
		Graph:_id       Graphic id.
		Float:x     	x coord inside the graphic.
        Float:y    		y coord inside the graphic.
		color           Point color.

	Returns:
		1 on success, 0 on failure.

    Notes:
		-
*/
stock
	GRAPHIC::AddPoint(Graph:_id, Float:x, Float:y, color)
{
    // Checking if its valid. ' (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph]) '
	if(isValid(_id))
	{
	    // Restrictions
		if (Gr@phic[_:_id][GRAPHIC::_x_min] > x || Gr@phic[_:_id][GRAPHIC::_y_min] > y || Gr@phic[_:_id][GRAPHIC::_x_max] < x || Gr@phic[_:_id][GRAPHIC::_y_max] < y)
		{
			// Returning 0. failure
			return 0;
		}
		else
		{
		    // Creating the GRAPHIC::AddPoint  textdraw.
			__td[++_td] = TextDrawCreate(Gr@phic[_:_id][GRAPHIC::_x] + 4 + x, Gr@phic[_:_id][GRAPHIC::_y] - 18 - y, ".");
			TextDrawLetterSize(__td[_td], -0.41, 2.19);
			TextDrawColor(__td[_td], color);
			TextDrawSetOutline(__td[_td], 0);
			TextDrawSetShadow(__td[_td], 0);
			// Returning 1. success
			return 1;
		}
	}
	// Returning 0. failure
	return 0;
}

stock GRAPHIC::ResetTD()
{
 	_td = -1;
}

stock GRAPHIC::RemoveTD(a)
{
	for(new i = 0; i < a; i++)
	{
	    TextDrawDestroy(__td[i]);
	}
}

/*
    GRAPHIC::ShowForPlayer:
		Show the 'graphic' textdraw for player.

    Arguments:
		playerid        Playerid of the specific player.
		Graph:_id       Graphic id.

	Returns:
		1 on success, 0 on failure.

    Notes:
		-
*/
stock
	GRAPHIC::ShowForPlayer(playerid, Graph:_id)
{
    // Checking if its valid.  (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph])  and
    // if 'playerid' is successfuly connected. (IsPlayerConnected(playerid))
	if (isValid(_id) && IsPlayerConnected(playerid))
	{
	    // Showing the x-Axis textdraw.
	    TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_0]);
	    // Showing the y-Axis textdraw.
	    TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_1]);
	    // Checking if the id matched with a graphic is previosly created. (Gr@phic[_:_id][_crt_box])
		if (Gr@phic[_:_id][GRAPHIC::_crt_box])
		{
	    	// Showing the backgroung textdraw.
	    	TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_box]);
		}
	    // Loop all points.
		for (new i = 0 ; i <= _td; ++i)
		{
		    // Showing points one by one.
			TextDrawShowForPlayer(playerid, __td[i]);
		}
		// Returning 1. success
	    return 1;
	}
	// Returning 0. failure
	return 0;
}

/*
    GRAPHIC::HideForPlayer:
		Hide the 'graphic' textdraw for player.

    Arguments:
		playerid        Playerid of the specific player.
		Graph:_id       Graphic id.

	Returns:
		1 on success, 0 on failure.

    Notes:
		-
*/
stock
	GRAPHIC::HideForPlayer(playerid, Graph:_id)
{
    // Checking if its valid.  (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph]) and
    // if 'playerid' is successfuly connected. (IsPlayerConnected(playerid))
	if (isValid(_id) && IsPlayerConnected(playerid))
	{
	    // Hideing the x-Axis textdraw.
	    TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_0]);
	    // Hideing the y-Axis textdraw.
	    TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_1]);
	    // Checking if the id matched with a graphic is previosly created. (Gr@phic[_:_id][_crt_box])
		if (Gr@phic[_:_id][GRAPHIC::_crt_box])
		{
		    // Hideing the backgroung textdraw.
	    	TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_box]);
		}
		// Loop all points.
		for (new i = 0 ; i <= _td; ++i)
		{
		    // Hideing points one by one.
			TextDrawHideForPlayer(playerid, __td[i]);
		}
		// Returning 1. success
	    return 1;
	}
	// Returning 0. failure
	return 0;
}

/*
    GRAPHIC::ShowForAll:
		Show the 'graphic' textdraw for all.

    Arguments:
		Graph:_id       Graphic id.

	Returns:
		1 on success, 0 on failure.

    Notes:
		- graphicShowForPlayer used. (->i)
*/
stock
	GRAPHIC::ShowForAll(Graph:_id)
{
	// loop:
	// ..
	// Checking if foreach.inc is included. (recommended)
    #if defined _foreach_included
		foreach(Player, i)
    #else
        // If not included, use the fastest plain player-loop.
		for (new a = GetMaxPlayers(), i ; i < a ; i++ )
    		if (IsPlayerConnected(i))
    #endif
    // else checking if is NOT NPC.
    #if defined IsPlayerNPC
            if (!IsPlayerNPC(i))
    #endif
    {
			    // Calling the 'GRAPHIC::ShowForPlayer' function. (->i)
				GRAPHIC::ShowForPlayer(i, _id);
				// Returning 1. success
				return 1;
    }
    	// Returning 0. failure
		return 0;
}

/*
    GRAPHIC::HideForAll:
		Hide the 'graphic' textdraw for all.

    Arguments:
		Graph:_id       Graphic id.

	Returns:
		1 on success, 0 on failure.

    Notes:
		- graphicHideForPlayer used. (->i)
*/
stock
	GRAPHIC::HideForAll(Graph:_id)
{
	// loop:
	// ..
	// Checking if foreach.inc is included. (recommended)
    #if defined _foreach_included
		foreach(Player, i)
    #else
        // If not included, use the fastest plain player-loop.
		for (new a = GetMaxPlayers(), i ; i < a ; i++ )
    		if (IsPlayerConnected(i))
    #endif
    // else checking if is NOT NPC.
    #if defined IsPlayerNPC
            if (!IsPlayerNPC(i))
    #endif
    {
                // Calling the 'GRAPHIC::HideForPlayer' function. (->i)
				GRAPHIC::HideForPlayer(i, _id);
				// Returning 1. success
				return 1;
   	}
	    // Returning 0. failure
		return 0;
}

/*
    GRAPHIC::Update:
		Update the graphic id created and used by the script to the player. Read 'Notes' !

    Arguments:
		Graph:_id       Graphic id.
		(op) playerid   Playerid of the specific player.

	Returns:
		1 on success, 0 on failure.

    Notes:
		- Default 'playerid' is 'INVALID_PLAYER_ID': this means that the function updates the
		  specific textdraw for all connected players.
*/
stock
	GRAPHIC::Update(Graph:_id, playerid = INVALID_PLAYER_ID)
{
    // Checking if its valid. (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph])
	if (isValid(_id))
	{
	    // For all..
        if(playerid == INVALID_PLAYER_ID)
            // so..
            // ..
            // First hideing the textdraw and then showing it.
            GRAPHIC::HideForAll(_id), GRAPHIC::ShowForAll(_id);

        // For a specific player..
		else
			// so..
			// ..
			// First hideing the textdraw and then showing it.
		    GRAPHIC::HideForPlayer(playerid, _id), GRAPHIC::ShowForPlayer(playerid, _id);
		// Returning 1. success
	    return 1;
	}
	// Returning 0. failure
	return 0;
}

/*
    GRAPHIC::Destroy:
		Destroy all textdraws created and used by the script.

    Arguments:
		Graph:_id       Graphic id.

	Returns:
		1 on success, 0 on failure.

    Notes:
		- Used on the sufix 'Exit' callbacks.
*/
stock
	GRAPHIC::Destroy(Graph:_id)
{
    // Checking if its valid. ' (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph]) '
	if (isValid(_id))
	{
	    // Destroying x-Axis.
		TextDrawDestroy(Gr@phic[_:_id][GRAPHIC::_td_0]);
		// Destroying y-Axis.
		TextDrawDestroy(Gr@phic[_:_id][GRAPHIC::_td_1]);
		// Checking if the id matched with a graphic is previosly created. (Gr@phic[_:_id][_crt_box])
		if (Gr@phic[_:_id][GRAPHIC::_crt_box])
		{
	      	// Destroying background textdraw if had.
			TextDrawDestroy(Gr@phic[_:_id][GRAPHIC::_td_box]);
			// Store a 'false' value. (logic?)
			Gr@phic[_:_id][GRAPHIC::_crt_box] = false;
		}
		// Loop all points.
		for (new i = 0 ; i <= _td; ++i)
		{
		    // Destroying points.
			TextDrawDestroy(__td[i]);
		}
		// Store '0's and 'false's values. (logic?)
		Gr@phic[_:_id][GRAPHIC::_x] = 0.0;
		Gr@phic[_:_id][GRAPHIC::_x_min] = 0.0;
		Gr@phic[_:_id][GRAPHIC::_x_max] = 0.0;
		Gr@phic[_:_id][GRAPHIC::_y] = 0.0;
		Gr@phic[_:_id][GRAPHIC::_y_min] = 0.0;
		Gr@phic[_:_id][GRAPHIC::_y_max] = 0.0;
		Gr@phic[_:_id][GRAPHIC::_crt_grph] = false;
		// Returning 1. success
		return 1;
	}
	// Returning 0. failure
	return 0;
}

// #section (op)

/*
    GRAPHIC::OtherXYAxis:
		Hide/Shows the x-Axis/y-Axix (if required) 'graphic' textdraw for player.

    Arguments:
        oper            Operation: 0 to hide, 1 to show.
		playerid        Playerid of the specific player.
		Graph:_id       Graphic id.
		x-Axis          - ..
		y-Axis          - ..

	Returns:
		1 on success, 0 on failure.

    Notes:
		- INVALID_PLAYER_ID will be recognise as 'for all players online'
*/
stock
	GRAPHIC::OtherXYAxis(oper, playerid, Graph:_id, xAxis, yAxis)
{
	if(playerid != INVALID_PLAYER_ID)
	{
	    // Checking if its valid. ' (%0 != INVALID_GRAPHIC_ID && Gr@phic[_:%0][_crt_grph]) ' and
	    // if 'playerid' is successfuly connected. (IsPlayerConnected(playerid))
		if (isValid(_id) && IsPlayerConnected(playerid))
		{
		    if(!oper)
			{
			    if(xAxis)
			    {
	   				// Hideing the x-Axis textdraw.
				    TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_0]);
			    }
			    if(yAxis)
			    {
				    // Hideing the y-Axis textdraw.
				    TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_1]);
			    }
			}
			else
			{
			    if(xAxis)
			    {
				    // Showing the x-Axis textdraw.
				    TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_0]);
			    }
			    if(yAxis)
			    {
				    // Showing the y-Axis textdraw.
				    TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_1]);
			    }
			}
			// Returning 1. success
		    return 1;
		}
	}
	else
	{
		if (isValid(_id))
		{
			// loop:
			// ..
			// Checking if foreach.inc is included. (recommended)
		    #if defined _foreach_included
				foreach(Player, i)
		    #else
		        // If not included, use the fastest plain player-loop.
				for (new a = GetMaxPlayers(), i ; i < a ; i++ )
		    		if (IsPlayerConnected(i))
		    #endif
		    // else checking if is NOT NPC.
		    #if defined IsPlayerNPC
		            if (!IsPlayerNPC(i))
		    #endif
		    {
					    if(!oper)
						{
						    if(xAxis)
						    {
				   				// Hideing the x-Axis textdraw.
							    TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_0]);
						    }
						    if(yAxis)
						    {
							    // Hideing the y-Axis textdraw.
							    TextDrawHideForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_1]);
						    }
						}
						else
						{
						    if(xAxis)
						    {
							    // Showing the x-Axis textdraw.
							    TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_0]);
						    }
						    if(yAxis)
						    {
							    // Showing the y-Axis textdraw.
							    TextDrawShowForPlayer(playerid, Gr@phic[_:_id][GRAPHIC::_td_1]);
						    }
						}
						// Returning 1. success
					    return 1;
		    }
		}
	}
	// Returning 0. failure
	return 0;
}

// #section _HOOK_SYSTEM

static
	bool:_h@s;
	
#if defined FILTERSCRIPT

public
	OnFilterScriptInit()
{
	_h@s = funcidx(#GRAPHIC::"Init") != -1;
	if (_h@s)
	{
		return CallLocalFunction(#GRAPHIC::"Init", "");
	}
    return CallLocalFunction("lib_OnFilterScriptInit", "");
}

#if defined _ALS_OnFilterScriptInit
    #undef OnFilterScriptInit
#else
    #define _ALS_OnFilterScriptInit
#endif
#define OnFilterScriptInit lib_OnFilterScriptInit
forward lib_OnFilterScriptInit();

#else // not a filterscript

public
	OnGameModeInit()
{
	_h@s = funcidx(#GRAPHIC::"Init") != -1;
	if (_h@s)
	{
		return CallLocalFunction(#GRAPHIC::"Init", "");
	}
	if (funcidx("lib_OnGameModeInit") != -1)
	{
		return CallLocalFunction("lib_OnGameModeInit", "");
	}
	return 1;
}

#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit lib_OnGameModeInit
forward lib_OnGameModeInit();

#endif

// _@EOS