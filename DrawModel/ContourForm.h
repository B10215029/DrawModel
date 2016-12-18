#pragma once
#include "OpenGLEventTrigger.h"
#include "ContourPanel.h"

namespace DrawModel {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// ContourForm ���K�n
	/// </summary>
	public ref class ContourForm : public System::Windows::Forms::Form
	{
	public:
		ContourForm(void)
		{
			InitializeComponent();
			//
			//TODO:  �b���[�J�غc�禡�{���X
			//
			contourPanel = new ContourPanel();
			gcnew OpenGLEventTrigger(panel1, timer1, contourPanel);
		}

	protected:
		/// <summary>
		/// �M������ϥΤ����귽�C
		/// </summary>
		~ContourForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::Timer^  timer1;
	private: System::ComponentModel::IContainer^  components;
	protected:

	private:
		/// <summary>
		/// �]�p�u��һݪ��ܼơC
		/// </summary>
		ContourPanel* contourPanel;


#pragma region Windows Form Designer generated code
		/// <summary>
		/// �����]�p�u��䴩�һݪ���k - �ФŨϥε{���X�s�边�ק�
		/// �o�Ӥ�k�����e�C
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->panel1->Location = System::Drawing::Point(0, 0);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(284, 261);
			this->panel1->TabIndex = 0;
			// 
			// ContourForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->panel1);
			this->Name = L"ContourForm";
			this->Text = L"ContourForm";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &ContourForm::ContourForm_FormClosing);
			this->VisibleChanged += gcnew System::EventHandler(this, &ContourForm::ContourForm_VisibleChanged);
			this->ResumeLayout(false);

		}
#pragma endregion
	public: void SetModelPart(ModelPart* part) {
		contourPanel->part = part;
	}
	private: System::Void ContourForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
		e->Cancel = true;
		this->Hide();
	}
	private: System::Void ContourForm_VisibleChanged(System::Object^  sender, System::EventArgs^  e) {
		timer1->Enabled = this->Visible;
	}
	};
}
