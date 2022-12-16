#pragma once
#include "Source/DirectX/DirectX.h"

class Text
{
private:
	BYTE keybuf[256], keybuf_old[256], key_down[256], key_up[256];
	std::string input_word;
public:
	std::string ReturnWord();

	void Update_Key()
	{
		for (int i = 0; i < 256; i++)
		{
			keybuf_old[i] = keybuf[i];
		}

		GetKeyboardState(keybuf);

		for (int i = 0; i < 256; i++)
		{
			int key_xor = keybuf[i] ^ keybuf_old[i];
			key_down[i] = key_xor & keybuf[i];
			key_up[i] = key_xor & keybuf_old[i];
		}
	}

	bool PushReturn();
	bool PushBackSpace();
};