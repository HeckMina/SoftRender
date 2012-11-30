#include "prerequisite.h"
#include "SrRenderContext.h"
#include "SrHwD3D9Renderer.h"

SrRendContext* g_context;
GlobalEnvironment* gEnv;
SrHwD3D9Renderer* g_renderer;

extern "C" __declspec(dllexport) IRenderer* LoadRenderer(GlobalEnvironment* pgEnv)
{
	gEnv = pgEnv;
	g_context = gEnv->context;

	g_renderer = new SrHwD3D9Renderer();

	return g_renderer;
}

extern "C" __declspec(dllexport) void FreeRenderer()
{
	delete g_renderer;
}