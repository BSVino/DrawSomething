#include "ds_renderer.h"

#include "client/renderer/context.h"

#include "../ds_client.h"

void DSRenderer::Draw()
{
	base.ClearColor(Color(78, 188, 239, 255));
	base.ClearDepth();

	base.SetCamera(Vector(0, 0, 10), Vector(1, 0, 0), Vector(0, 0, 1), 90, 5, 1000);

	Context c(&base);
	base.StartRendering(&c);

	c.UseShader(SHADER_MODEL);

	c.BeginRenderTriFan();
		c.Color(Color(128, 128, 128, 255));

		c.TexCoord(Vector2D(0, 1));
		c.Vertex(Vector(-300, -300, 0));
		c.TexCoord(Vector2D(1, 1));
		c.Vertex(Vector(300, -300, 0));
		c.TexCoord(Vector2D(1, 0));
		c.Vertex(Vector(300, 300, 0));
		c.TexCoord(Vector2D(0, 0));
		c.Vertex(Vector(-300, 300, 0));
	c.EndRender();

	base.FinishRendering(&c);
}
