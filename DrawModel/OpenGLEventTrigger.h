#pragma once
#include "OpenGLPanel.h"
#include <windows.h>

public ref class OpenGLEventTrigger
{
public:
	OpenGLEventTrigger(System::Windows::Forms::Panel^ panel, System::Windows::Forms::Timer^ timer, OpenGLPanel* drawer);
	~OpenGLEventTrigger();

private:
	OpenGLPanel* drawer;
	System::Windows::Forms::Panel^ panel;
	System::Windows::Forms::Timer^ timer;

private: System::Void panel_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void panel_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void panel_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void panel_MouseWheel(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void panel_Resize(System::Object^  sender, System::EventArgs^  e);
private: System::Void timer_Tick(System::Object^  sender, System::EventArgs^  e);
};
