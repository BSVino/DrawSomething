#pragma once

#include "artist.h"

// Properties of the artist that live only on the client.
struct LocalArtist
{
	Artist* m_local;

	double m_draw_time; // Stay in draw mode until this time.
	float m_draw_mode;

	LocalArtist()
		: m_draw_mode(0), m_draw_time(-9999)
	{
		m_local = NULL;
	}
	
	void HandleInput(struct ControlData* input);
	void LocalThink(); // Think that only the local player does.
};
