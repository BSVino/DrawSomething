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
			BumpDrawTime();

		float window_border_acceleration = vb_const_float(vb_str("dm_border_acceleration"), 20);
		float window_border_velocity = vb_const_float(vb_str("dm_border_velocity"), 20);

		if (!input->m_draw.m_down)
		{
			float window_border = vb_const_float(vb_str("dm_border"), 60);
			float window_border_max_velocity = vb_const_float(vb_str("dm_border_max_velocity"), 4);
			if (g_client_data->m_window_data->m_mouse_x < window_border)
			{
				m_draw_mode_velocity.x = Approach(-window_border_max_velocity, m_draw_mode_velocity.x, g_client_data->m_shared.m_frame_time * window_border_acceleration);
				BumpDrawTime();
			}
			else if (g_client_data->m_window_data->m_mouse_x > g_client_data->m_window_data->m_width - window_border)
			{
				m_draw_mode_velocity.x = Approach(window_border_max_velocity, m_draw_mode_velocity.x, g_client_data->m_shared.m_frame_time * window_border_acceleration);
				BumpDrawTime();
			}
			else
				m_draw_mode_velocity.x = Approach(0, m_draw_mode_velocity.x, g_client_data->m_shared.m_frame_time * window_border_acceleration);

			if (g_client_data->m_window_data->m_mouse_y < window_border)
			{
				m_draw_mode_velocity.y = Approach(-window_border_max_velocity, m_draw_mode_velocity.y, g_client_data->m_shared.m_frame_time * window_border_acceleration);
				BumpDrawTime();
			}
			else if (g_client_data->m_window_data->m_mouse_y > g_client_data->m_window_data->m_height - window_border)
			{
				m_draw_mode_velocity.y = Approach(window_border_max_velocity, m_draw_mode_velocity.y, g_client_data->m_shared.m_frame_time * window_border_acceleration);
				BumpDrawTime();
			}
			else
				m_draw_mode_velocity.y = Approach(0, m_draw_mode_velocity.y, g_client_data->m_shared.m_frame_time * window_border_acceleration);
		}
		else
		{
			m_draw_mode_velocity.x = Approach(0, m_draw_mode_velocity.x, g_client_data->m_shared.m_frame_time * window_border_acceleration);
			m_draw_mode_velocity.y = Approach(0, m_draw_mode_velocity.y, g_client_data->m_shared.m_frame_time * window_border_acceleration);
		}

		m_local->m_looking.y -= m_draw_mode_velocity.x * g_client_data->m_shared.m_frame_time * window_border_velocity;
		m_local->m_looking.p -= m_draw_mode_velocity.y * g_client_data->m_shared.m_frame_time * window_border_velocity;
	}
	else
	{
		m_local->m_looking.p -= input->m_mouse_dy * g_client_data->m_shared.m_frame_time * 30;
		m_local->m_looking.y -= input->m_mouse_dx * g_client_data->m_shared.m_frame_time * 30;
	}

	if (m_local->m_looking.p > 89.9f)
		m_local->m_looking.p = 89.9f;
	if (m_local->m_looking.p < -89.9f)
		m_local->m_looking.p = -89.9f;

	m_local->m_looking.y = fmod(m_local->m_looking.y, 360.0f);

	vec3 velocity(0, 0, 0);

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

	m_local->m_position += transformed_velocity * g_client_data->m_shared.m_frame_time;

	if (input->m_forward.m_down || input->m_back.m_down || input->m_left.m_down || input->m_right.m_down)
		m_draw_time = -9999;

	if (input->m_draw.m_down)
		BumpDrawTime();

	if (input->m_draw.m_pressed)
	{
		g_client_data->m_buckets.AddNewStroke();

		m_draw_mode_velocity = vec2(0,0);

		// Tell the server we've started a new stroke.
		uint32 length = 2;
		TStackAllocate(uint8, contents, length);
		contents[0] = 'S'; // Stroke
		contents[1] = 'N'; // New stroke
		g_client_data->m_shared.m_host.Packet_SendCustom(contents, length);
	}

	if (input->m_draw.m_down)
	{
		float aspect_ratio = (float)g_client_data->m_window_data->m_width / (float)g_client_data->m_window_data->m_height;

		vec3 view_direction = AngleVector(m_local->m_looking);

		mat4 projection = mat4::ProjectPerspective(GetFOV(), aspect_ratio, g_client_data->m_renderer.base.m_camera_near, g_client_data->m_renderer.base.m_camera_far);
		mat4 camera = mat4::ConstructCameraView(m_local->m_position, view_direction, vec3(0, 0, 1));

		vec3 unprojected = mat4::UnProjectPoint(projection, camera, (float)g_client_data->m_window_data->m_width, (float)g_client_data->m_window_data->m_height,
			vec3((float)g_client_data->m_window_data->m_mouse_x, (float)g_client_data->m_window_data->m_mouse_y, 1));

		vec3 projection_direction = unprojected - m_local->m_position;

		vec3 projected_onto_view = view_direction.Dot(projection_direction) * view_direction;

		float projection_ratio = 0.5f / projected_onto_view.Length();

		vec3 new_point = m_local->m_position + projection_direction * projection_ratio;

		bool skip = false;

		if (g_client_data->m_local_artist.m_current_stroke.Valid())
		{
			BucketCoordinate bc = g_client_data->m_local_artist.m_current_stroke.m_bucket;
			BucketHashIndex bucket_index = g_client_data->m_buckets.m_shared.BucketHash_Find(&bc);

			if (bucket_index != TInvalid(BucketHashIndex))
			{
				BucketHeader* bucket = &g_client_data->m_buckets.m_shared.m_buckets_hash[bucket_index];
				StrokeInfo* stroke = &bucket->m_strokes[g_client_data->m_local_artist.m_current_stroke.m_stroke_index];

				TAssert(!stroke->m_next.Valid());

				vec3 previous_point = bucket->m_verts[stroke->m_first_vertex + stroke->m_num_verts-1];

				if ((new_point - previous_point).LengthSqr() < 0.000001f)
					skip = true;
			}
		}

		if (!skip)
		{
			g_client_data->m_buckets.AddPointToStroke(&new_point);

			// Tell the server we've drawn a new point.
			uint32 length = 2 + sizeof(vec3);
			TStackAllocate(uint8, contents, length);
			contents[0] = 'S'; // Stroke
			contents[1] = 'P'; // New point in the current stroke
			*(vec3*)&contents[2] = new_point;
			g_client_data->m_shared.m_host.Packet_SendCustom(contents, length);
		}
	}

	if (input->m_draw.m_released)
	{
		g_client_data->m_buckets.EndStroke();

		// Tell the server we've drawn a new point.
		uint32 length = 2;
		TStackAllocate(uint8, contents, length);
		contents[0] = 'S'; // Stroke
		contents[1] = 'E'; // End the stroke
		g_client_data->m_shared.m_host.Packet_SendCustom(contents, length);
	}
}

void LocalArtist::BumpDrawTime()
{
	m_draw_time = g_client_data->m_shared.m_game_time + 1.5;
}

void LocalArtist::LocalThink()
{
	if (!m_local)
		return;

	float draw_mode_goal = g_client_data->m_shared.m_game_time < m_draw_time;
	m_draw_mode = Approach(draw_mode_goal, m_draw_mode, g_client_data->m_shared.m_frame_time * vb_const_float(vb_str("dm_lerp"), 10));

	g_client_data->m_window_data->m_cursor_visible = !!draw_mode_goal;
}

float LocalArtist::GetFOV()
{
	return 90 + m_draw_mode * 20;
}
