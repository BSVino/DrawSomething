#include "artist.h"

#include <viewback2.h>

#include "input.h"
#include "window.h"

#include "client/ds_client.h"

void LocalArtist::HandleInput(ControlData* input)
{
	if (!m_local)
		return;

	if (m_draw_mode)
	{
		if (input->m_mouse_dy || input->m_mouse_dx)
			m_draw_time = g_client_data->m_game_time + 1.5;
	}
	else
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

	if (input->m_draw.m_pressed)
	{
		if (g_client_data->m_num_strokes < MAX_STROKES)
		{
			ClientData::Stroke* stroke = &g_client_data->m_strokes[g_client_data->m_num_strokes];

			// Start a new stroke
			int first_point = 0;
			if (g_client_data->m_num_strokes)
				first_point = (stroke - 1)->m_first + (stroke - 1)->m_size;

			stroke->m_first = first_point;
			stroke->m_size = 0;

			g_client_data->m_num_strokes++;
		}
	}

	if (input->m_draw.m_down)
	{
		ClientData::Stroke* stroke = &g_client_data->m_strokes[g_client_data->m_num_strokes - 1];

		if (stroke->m_first + stroke->m_size < MAX_STROKE_POINTS)
		{
			// TODO: Don't write it if it's too close to the last one?
			// TODO: Erase the line if there's only one point when the artist lets go?

			float aspect_ratio = (float)g_client_data->m_window_data->m_width / (float)g_client_data->m_window_data->m_height;

			mat4 projection = mat4::ProjectPerspective(GetFOV(), aspect_ratio, g_client_data->m_renderer.base.m_camera_near, g_client_data->m_renderer.base.m_camera_far);
			mat4 camera = mat4::ConstructCameraView(m_local->m_position, AngleVector(m_local->m_looking), vec3(0, 0, 1));

			vec3 unprojected_0 = mat4::UnProjectPoint(projection, camera, (float)g_client_data->m_window_data->m_width, (float)g_client_data->m_window_data->m_height,
				vec3((float)g_client_data->m_window_data->m_mouse_x, (float)g_client_data->m_window_data->m_mouse_y, 0));
			vec3 unprojected_1 = mat4::UnProjectPoint(projection, camera, (float)g_client_data->m_window_data->m_width, (float)g_client_data->m_window_data->m_height,
				vec3((float)g_client_data->m_window_data->m_mouse_x, (float)g_client_data->m_window_data->m_mouse_y, 1));

			vec3 projection_direction = (unprojected_1 - unprojected_0).Normalized();

			g_client_data->m_stroke_points[stroke->m_first + stroke->m_size] = m_local->m_position + projection_direction * 0.5f;
			stroke->m_size++;
		}
	}
}

void LocalArtist::LocalThink()
{
	if (!m_local)
		return;

	float draw_mode_goal = g_client_data->m_game_time < m_draw_time;
	m_draw_mode = Approach(draw_mode_goal, m_draw_mode, g_client_data->m_frame_time * 10.0f);

	g_client_data->m_window_data->m_cursor_visible = !!draw_mode_goal;
}

float LocalArtist::GetFOV()
{
	return 90 + m_draw_mode * 20;
}
