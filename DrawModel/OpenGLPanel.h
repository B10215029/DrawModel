#pragma once
#include <windows.h>

public class OpenGLPanel
{
public:
	OpenGLPanel();
	~OpenGLPanel();
	void SetPanel(HWND panelHandle);
	void BindGL();
	void ReleaseGL();

	virtual void Initialize() = 0;
	virtual void Reshape(int width, int height) = 0;
	virtual void Display() = 0;
	virtual void MouseDown(int x, int y, int button) = 0;
	virtual void MouseUp(int x, int y, int button) = 0;
	virtual void MouseMove(int x, int y) = 0;
	virtual void MouseWheel(int x, int y, int delta) = 0;
	//void Update();
	//void Resize(int width, int height);

	//void CallInitialize();
	void CallReshape(int width, int height);
	void CallDisplay();
	void CallMouseDown(int x, int y, int button);
	void CallMouseUp(int x, int y, int button);
	void CallMouseMove(int x, int y);
	void CallMouseWheel(int x, int y, int delta);

	bool isLMBDown;
	bool isRMBDown;
	bool isMMBDown;
	bool isInitialize;

	int width;
	int height;

private:
	HDC hDC;
	HGLRC hGLRC;

};
