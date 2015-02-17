#include "artist.h"

#include "input.h"

#include "client/ds_client.h"

void Artist::HandleInput(ControlData* input)
{
	m_looking.p -= input->m_mouse_dy * g_client_data->m_frame_time * 30;
	m_looking.y -= input->m_mouse_dx * g_client_data->m_frame_time * 30;

	if (m_looking.p > 89.9f)
		m_looking.p = 89.9f;
	if (m_looking.p < -89.9f)
		m_looking.p = -89.9f;

	m_looking.y = fmod(m_looking.y, 360.0f);

	vec3 velocity(0,0,0);

	if (input->m_forward.m_down && input->m_back.m_down)
		velocity.x = 0;
	else if (input->m_forward.m_down)
		velocity.x = 1;
	else if (input->m_back.m_down)
		velocity.x = -1;

	if (input->m_left.m_down && input->m_right.m_down)
		velocity.y = 0;
	else if (input->m_left.m_down)
		velocity.y = 1;
	else if (input->m_right.m_down)
		velocity.y = -1;

	mat4 player_looking(m_looking);
	vec3 transformed_velocity = player_looking * velocity;

	m_position += transformed_velocity * g_client_data->m_frame_time;
}

