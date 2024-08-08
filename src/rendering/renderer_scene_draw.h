#ifndef RENDERER_SCENE_DRAW_H
#define RENDERER_SCENE_DRAW_H

class RendererSceneDraw {
public:
	static RendererSceneDraw &singleton() {
		static RendererSceneDraw instance;
		return instance;
	}

	RendererSceneDraw(RendererSceneDraw const &) = delete;
	void operator=(RendererSceneDraw const &) = delete;

private:
	RendererSceneDraw() {}

public:
	void draw();
};

typedef RendererSceneDraw RSD;

#endif // !RENDERER_SCENE_DRAW_H
