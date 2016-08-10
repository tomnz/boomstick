#include "color.h"

Color Wheel(uint8_t wheelPos) {
//  WheelPos = 255 - WheelPos;

	if(wheelPos < 85) {
		return Color(255 - wheelPos * 3, 0, wheelPos * 3);
	} else if(wheelPos < 170) {
		wheelPos -= 85;
		return Color(0, wheelPos * 3, 255 - wheelPos * 3);
	} else {
		wheelPos -= 170;
		return Color(wheelPos * 3, 255 - wheelPos * 3, 0);
	}
}
