#pragma once


namespace dg {

struct MouseEvent
{
	enum Buttons
	{
		NoButton      = 0x0000,
		LeftButton    = 0x0001,
		RightButton   = 0x0002,
		MiddleButton  = 0x0004,
		BackButton    = 0x0008,
		ForwardButton = 0x0010,
	};

	int buttons;
	int x,y;
};

struct WheelEvent
{
	int delta_vertical;
	int delta_horizontal;
};


}
