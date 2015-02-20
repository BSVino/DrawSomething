/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LW_MATHS_H
#define LW_MATHS_H

// For min/max
#include <algorithm>

// Generic math functions
#include <math.h>
#include <string.h>

inline float Bias(float x, float amount)
{
	static float last_amount = -1;
	static float last_exp = -1;

	if (amount == 0.5f)
		return x;

	if (last_amount != amount)
		last_exp = log(amount) * -1.4427f;	// 1/log(0.5f)

	if (x < 0)
		return 0;

	return pow(x, last_exp);
}

inline float Gain(float x, float amount)
{
	if(x < 0.5f)
		return Bias(2 * x, amount) / 2;
	else
		return 1 - Bias(2 - 2 * x, amount) / 2;
}

template <class T>
inline const T LerpValue(const T& from, const T& to, float lerp)
{
	return to * lerp + from * (1 - lerp);
}

template <class T>
inline const T LerpValue(const T& from, const T& to, float lerp, float in_lo, float in_hi)
{
	return (((lerp-in_lo) / (in_hi-in_lo)) * (to-from)) + from;
}

template <class T>
inline const T LerpValueClamped(const T& from, const T& to, float lerp, float in_lo, float in_hi)
{
	if (lerp < in_lo)
		return from;

	if (lerp > in_hi)
		return to;

	return (((lerp-in_lo) / (in_hi-in_lo)) * (to-from)) + from;
}

inline float RemapVal(float input, float in_lo, float in_hi, float out_lo, float out_hi)
{
	return (((input-in_lo) / (in_hi-in_lo)) * (out_hi-out_lo)) + out_lo;
}

inline double RemapVal(double input, double in_lo, double in_hi, double out_lo, double out_hi)
{
	return (((input-in_lo) / (in_hi-in_lo)) * (out_hi-out_lo)) + out_lo;
}

template <class T>
inline T RemapVal(T input, T in_lo, T in_hi, T out_lo, T out_hi)
{
	return (((input-in_lo) / (in_hi-in_lo)) * (out_hi-out_lo)) + out_lo;
}

inline double RemapBiased(double input, double in_lo, double in_hi, double out_lo, double out_hi, double flBias)
{
	return (Bias((float)((input - in_lo) / (in_hi - in_lo)), (float)flBias) * (out_hi - out_lo)) + out_lo;
}

template <class T>
inline void TSwap(T& l, T& r)
{
	T temp = l;
	l = r;
	r = temp;
}

inline float RemapValClamped(float input, float in_lo, float in_hi, float out_lo, float out_hi)
{
	float flReturn = RemapVal(input, in_lo, in_hi, out_lo, out_hi);

	if (out_hi < out_lo)
		TSwap(out_hi, out_lo);

	if (flReturn < out_lo)
		return out_lo;

	if (flReturn > out_hi)
		return out_hi;

	return flReturn;
}

inline double RemapValClamped(double input, double in_lo, double in_hi, double out_lo, double out_hi)
{
	double flReturn = RemapVal(input, in_lo, in_hi, out_lo, out_hi);

	if (out_hi < out_lo)
		TSwap(out_hi, out_lo);

	if (flReturn < out_lo)
		return out_lo;

	if (flReturn > out_hi)
		return out_hi;

	return flReturn;
}

template <class T>
inline T RemapValClamped(float input, float in_lo, float in_hi, const T& out_lo, const T& out_hi)
{
	float flLower = std::min(in_lo, in_hi);
	float flHigher = std::max(in_lo, in_hi);

	if (input < flLower)
		return out_lo;

	if (input > flHigher)
		return out_hi;

	return ((out_hi-out_lo) * ((input-in_lo) / (in_hi-in_lo))) + out_lo;
}

template<typename T>
T Clamp(T input, T min, T max)
{
	return (input<min) ? min : (input>max) ? max : input;
}
 
inline float Blink(float time, float length)
{
	if (fmod(time, length) > length / 2)
		return 1.0f;
	
	return 0.0f;
}

inline float Oscillate(float time, float length)
{
	return fabs(RemapVal((float)fmod(time, length), 0, length, -1, 1));
}

// Strobe: Flicker("az", GetGameTime(), 0.1f)
// Blink: Flicker("aaaaaaz", GetGameTime(), 1.0f)
// Ramp: Flicker("abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba", GetGameTime(), 1.0f)
inline float Flicker(const char* letters, float time, float length)
{
	if (!letters)
		return 0;

	size_t num_values = strlen(letters);
	if (num_values == 0)
		return 0;

	float mod_time = fmod(time, length);
	int value = (int)RemapValClamped(mod_time, 0, length, 0, (float)num_values);
	return RemapVal((float)letters[value], 'a', 'z', 0, 1);
}

inline float Approach(float goal, float input, float amount)
{
	float flDifference = goal - input;

	if (flDifference > amount)
		return input + amount;
	else if (flDifference < -amount)
		return input -= amount;
	else
		return goal;
}

inline float AngleNormalize( float a )
{
	while ( a >= 180 )
		a -= 360;

	while ( a <= -180 )
		a += 360;

	return a;
}

inline float AngleDifference( float a, float b )
{
	if (a != a || b != b)
		return 0;

	float d = a - b;

	if ( a > b )
		while ( d >= 180 )
			d -= 360;
	else
		while ( d <= -180 )
			d += 360;

	return d;
}

inline float AngleApproach(float goal, float input, float amount)
{
	float flDifference = AngleDifference(goal, input);

	if (flDifference > amount)
		return input + amount;
	else if (flDifference < -amount)
		return input -= amount;
	else
		return goal;
}

#endif
