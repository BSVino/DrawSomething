#include "artist.h"

#include "input.h"

void Artist::HandleInput(ControlData* input)
{
	m_looking.p -= input->m_mouse_dy;
	m_looking.y -= input->m_mouse_dx;

	if (m_looking.p > 89.9f)
		m_looking.p = 89.9f;
	if (m_looking.p < -89.9f)
		m_looking.p = -89.9f;

	m_looking.y = fmod(m_looking.y, 360.0f);
}

