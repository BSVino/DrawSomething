#include "artist.h"

#include <viewback2.h>

#include "input.h"
#include "window.h"

#include "client/ds_client.h"

void LocalArtist::HandleInput(ControlData* input)
{
	if (!m_local)
		return;

	if (!m_draw_mode)
	{
		m_local->m_looking.p -= input->m_mouse_dy * g_client_data->m_frame_time * 30;
		m_local->m_looking.y -= input->m_mouse_dx * g_client_data->m_frame_time * 30;

		if (m_local->m_looking.p > 89.9f)
			m_local->m_looking.p = 89.9f;
		if (m_local->m_looking.p < -89.9f)
			m_local->m_looking.p = -89.9f;

		m_local->m_looking.y = fmod(m_local->m_looking.y, 360.0f);
	}

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

	mat4 player_looking(m_local->m_looking);
	vec3 transformed_velocity = player_looking * velocity;

	m_local->m_position += transformed_velocity * g_client_data->m_frame_time;

	if (input->m_forward.m_down || input->m_back.m_down || input->m_left.m_down || input->m_right.m_down)
		m_draw_time = -9999;

	if (input->m_draw.m_down)
		m_draw_time = g_client_data->m_game_time + 1.5;
}

void LocalArtist::LocalThink()
{
	if (!m_local)
		return;

	float draw_mode_goal = g_client_data->m_game_time < m_draw_time;
	m_draw_mode = Approach(draw_mode_goal, m_draw_mode, g_client_data->m_frame_time * 10.0f);

	g_client_data->m_window_data->m_cursor_visible = !!draw_mode_goal;
}

