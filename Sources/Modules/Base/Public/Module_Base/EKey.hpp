/*
 *	Better than Nothing
 *	Copyright (C) 2026 kakesinfo
 *
 *	Modified for SDL3 Migration
 */

#pragma once

#include <cstdint>

namespace BetterThanNothing
{
	enum class EKey : int16_t
	{
		Unknown = 0,

		A = 4,
		B = 5,
		C = 6,
		D = 7,
		E = 8,
		F = 9,
		G = 10,
		H = 11,
		I = 12,
		J = 13,
		K = 14,
		L = 15,
		M = 16,
		N = 17,
		O = 18,
		P = 19,
		Q = 20,
		R = 21,
		S = 22,
		T = 23,
		U = 24,
		V = 25,
		W = 26,
		X = 27,
		Y = 28,
		Z = 29,

		D1 = 30,
		D2 = 31,
		D3 = 32,
		D4 = 33,
		D5 = 34,
		D6 = 35,
		D7 = 36,
		D8 = 37,
		D9 = 38,
		D0 = 39,

		Enter = 40,
		Escape = 41,
		Tab = 43,
		Space = 44,

		CapsLock = 57,

		F1 = 58,
		F2 = 59,
		F3 = 60,
		F4 = 61,
		F5 = 62,
		F6 = 63,
		F7 = 64,
		F8 = 65,
		F9 = 66,
		F10 = 67,
		F11 = 68,
		F12 = 69,

		Right = 79,
		Left = 80,
		Down = 81,
		Up = 82,

		LeftShift = 225,
		LeftControl = 224,
		LeftAlt = 226
	};

	enum class EKeyAction : int16_t
	{
		Pressed = 0,
		Released = 1
	};
}
