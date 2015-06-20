#pragma once

#include "artist.h"

// Properties of the artist that live only on the client.
struct LocalArtist
{
	Artist* m_local;

	StrokeCoordinate m_current_stroke;

	double m_draw_time; // Stay in draw mode until this time.
	float m_draw_mode;
	vec2 m_draw_mode_velocity; // For when the user puts the mouse at the edges.

	LocalArtist()
		: m_draw_mode(0), m_draw_time(-9999)
	{
		m_local = NULL;
		m_draw_mode_velocity = vec2(0, 0);
	}
	
	void HandleInput(struct ControlData* input);
	void BumpDrawTime();
	void LocalThink(); // Think that only the local player does.

	float GetFOV();
};
