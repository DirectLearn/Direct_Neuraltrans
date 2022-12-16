#include "input_text.h"

std::string Text::ReturnWord()
{
	if (key_up['A'] & 0x80)
	{
		input_word = "a";
		return input_word;
	}

	if (key_up['B'] & 0x80)
	{
		input_word = "b";
		return input_word;
	}

	if (key_up['C'] & 0x80)
	{
		input_word = "c";
		return input_word;
	}

	if (key_up['D'] & 0x80)
	{
		input_word = "d";
		return input_word;
	}

	if (key_up['E'] & 0x80)
	{
		input_word = "e";
		return input_word;
	}

	if (key_up['F'] & 0x80)
	{
		input_word = "f";
		return input_word;
	}

	if (key_up['G'] & 0x80)
	{
		input_word = "g";
		return input_word;
	}

	if (key_up['H'] & 0x80)
	{
		input_word = "h";
		return input_word;
	}

	if (key_up['I'] & 0x80)
	{
		input_word = "i";
		return input_word;
	}

	if (key_up['J'] & 0x80)
	{
		input_word = "j";
		return input_word;
	}

	if (key_up['K'] & 0x80)
	{
		input_word = "k";
		return input_word;
	}

	if (key_up['L'] & 0x80)
	{
		input_word = "l";
		return input_word;
	}

	if (key_up['M'] & 0x80)
	{
		input_word = "m";
		return input_word;
	}

	if (key_up['N'] & 0x80)
	{
		input_word = "n";
		return input_word;
	}

	if (key_up['O'] & 0x80)
	{
		input_word = "o";
		return input_word;
	}

	if (key_up['P'] & 0x80)
	{
		input_word = "p";
		return input_word;
	}

	if (key_up['Q'] & 0x80)
	{
		input_word = "q";
		return input_word;
	}

	if (key_up['R'] & 0x80)
	{
		input_word = "r";
		return input_word;
	}

	if (key_up['S'] & 0x80)
	{
		input_word = "s";
		return input_word;
	}

	if (key_up['T'] & 0x80)
	{
		input_word = "t";
		return input_word;
	}

	if (key_up['U'] & 0x80)
	{
		input_word = "u";
		return input_word;
	}

	if (key_up['V'] & 0x80)
	{
		input_word = "v";
		return input_word;
	}

	if (key_up['W'] & 0x80)
	{
		input_word = "w";
		return input_word;
	}

	if (key_up['X'] & 0x80)
	{
		input_word = "x";
		return input_word;
	}

	if (key_up['Y'] & 0x80)
	{
		input_word = "y";
		return input_word;
	}

	if (key_up['Z'] & 0x80)
	{
		input_word = "z";
		return input_word;
	}

	if (key_up['0'] & 0x80)
	{
		input_word = "0";
		return input_word;
	}

	if (key_up['1'] & 0x80)
	{
		input_word = "1";
		return input_word;
	}

	if (key_up['2'] & 0x80)
	{
		input_word = "2";
		return input_word;
	}

	if (key_up['3'] & 0x80)
	{
		input_word = "3";
		return input_word;
	}

	if (key_up['4'] & 0x80)
	{
		input_word = "4";
		return input_word;
	}

	if (key_up['5'] & 0x80)
	{
		input_word = "5";
		return input_word;
	}

	if (key_up['6'] & 0x80)
	{
		input_word = "6";
		return input_word;
	}

	if (key_up['7'] & 0x80)
	{
		input_word = "7";
		return input_word;
	}

	if (key_up['8'] & 0x80)
	{
		input_word = "8";
		return input_word;
	}

	if (key_up['9'] & 0x80)
	{
		input_word = "9";
		return input_word;
	}

	if (key_up[VK_SPACE] & 0x80)
	{
		input_word = " ";
		return input_word;
	}

	return "";
}


bool Text::PushReturn()
{
	if (key_up[VK_RETURN] & 0x80)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool Text::PushBackSpace()
{
	if (key_up[VK_BACK] & 0x80)
	{
		return true;
	}
	else
	{
		return false;
	}
}