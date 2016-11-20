#include "OpenGLPanel.h"
#include <iostream>
#include <GL\glew.h>

OpenGLPanel::OpenGLPanel()
{
	hDC = NULL;
	hGLRC = NULL;
	isLMBDown = false;
	isRMBDown = false;
	isMMBDown = false;
	isInitialize = false;
}

OpenGLPanel::~OpenGLPanel()
{
	if (hGLRC) {
		wglDeleteContext(hGLRC);
	}
}

void OpenGLPanel::SetPanel(HWND panelHandle)
{
	// initial OpenGL
	hDC = GetDC(panelHandle);
	hGLRC = NULL;

	// Set pixel format
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int cpf = ChoosePixelFormat(hDC, &pfd);
	if (!cpf) {
		std::cerr << "Choose Pixel Format Error\n";
		return;
	}

	SetPixelFormat(hDC, cpf, &pfd);

	hGLRC = wglCreateContext(hDC);
	if (!hGLRC) {
		std::cerr << "Create Render Context Error\n";
		return;
	}

	if (!wglMakeCurrent(hDC, hGLRC)) {
		hGLRC = NULL;
		std::cerr << "wglMakeCurrent Error\n";
		return;
	}
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		return;
	}
	//std::clog << (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(panel->Name)).ToPointer() << " create context ok" << std::endl;
	std::clog << "OpenGL Version: " << (char *)(glGetString(GL_VERSION)) << std::endl;

	Initialize();
	isInitialize = true;
	wglMakeCurrent(NULL, NULL);
}

void OpenGLPanel::BindGL()
{
	wglMakeCurrent(hDC, hGLRC);
}

void OpenGLPanel::ReleaseGL()
{
	wglMakeCurrent(NULL, NULL);
}

//void OpenGLPanel::Update()
//{
//	wglMakeCurrent(hDC, hGLRC);
//	Display();
//	SwapBuffers(hDC);
//	wglMakeCurrent(NULL, NULL);
//}

//void OpenGLPanel::Resize(int width, int height)
//{
//	wglMakeCurrent(hDC, hGLRC);
//	Reshape(width, height);
//	wglMakeCurrent(NULL, NULL);
//}

//void OpenGLPanel::CallInitialize()
//{
//	Initialize();
//}

void OpenGLPanel::CallReshape(int width, int height)
{
	this->width = width;
	this->height = height;
	wglMakeCurrent(hDC, hGLRC);
	Reshape(width, height);
	wglMakeCurrent(NULL, NULL);
}

void OpenGLPanel::CallDisplay()
{
	wglMakeCurrent(hDC, hGLRC);
	Display();
	SwapBuffers(hDC);
	wglMakeCurrent(NULL, NULL);
}

void OpenGLPanel::CallMouseDown(int x, int y, int button)
{
	switch (button)
	{
	case 0:
		isLMBDown = true;
		break;
	case 1:
		isRMBDown = true;
		break;
	case 2:
		isMMBDown = true;
		break;
	default:
		break;
	}
	MouseDown(x, y, button);
}

void OpenGLPanel::CallMouseUp(int x, int y, int button)
{
	switch (button)
	{
	case 0:
		isLMBDown = false;
		break;
	case 1:
		isRMBDown = false;
		break;
	case 2:
		isMMBDown = false;
		break;
	default:
		break;
	}
	MouseUp(x, y, button);
}

void OpenGLPanel::CallMouseMove(int x, int y)
{
	MouseMove(x, y);
}

void OpenGLPanel::CallMouseWheel(int x, int y, int delta)
{
	MouseWheel(x, y, delta);
}
