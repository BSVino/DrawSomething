#include "ds_renderer.h"

#include "client/renderer/context.h"

#include "../ds_client.h"

void DSRenderer::Draw()
{
	base.ClearColor(Color(78, 188, 239, 255));
	base.ClearDepth();

	base.SetCamera(Vector(0, 0, 1.6f), AngleVector(EAngle(m_pitch, m_yaw, 0)), Vector(0, 0, 1), 90, 0.01f, 1000);

	Context c(&base);
	base.StartRendering(&c);

	c.UseShader(SHADER_MODEL);

	c.BeginRenderTriFan();
		c.Color(Color(57, 138, 175, 255));

		c.TexCoord(Vector2D(0, 1));
		c.Vertex(Vector(-10, -10, 0));
		c.TexCoord(Vector2D(1, 1));
		c.Vertex(Vector(10, -10, 0));
		c.TexCoord(Vector2D(1, 0));
		c.Vertex(Vector(10, 10, 0));
		c.TexCoord(Vector2D(0, 0));
		c.Vertex(Vector(-10, 10, 0));
	c.EndRender();

	base.FinishRendering(&c);
}
