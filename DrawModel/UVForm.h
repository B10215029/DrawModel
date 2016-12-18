#pragma once
#include "OpenGLEventTrigger.h"
#include "UVPanel.h"

namespace DrawModel {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// UVForm ���K�n
	/// </summary>
	public ref class UVForm : public System::Windows::Forms::Form
	{
	public:
		UVForm(void)
		{
			InitializeComponent();
			//
			//TODO:  �b���[�J�غc�禡�{���X
			//
			uvPanel = new UVPanel();
			gcnew OpenGLEventTrigger(panel1, timer1, uvPanel);
		}

	protected:
		/// <summary>
		/// �M������ϥΤ����귽�C
		/// </summary>
		~UVForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^  panel1;
	protected:
	private: System::Windows::Forms::Timer^  timer1;
	private: System::ComponentModel::IContainer^  components;

	protected:

	private:
		/// <summary>
		/// �]�p�u��һݪ��ܼơC
		/// </summary>
		UVPanel* uvPanel;


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
			// UVForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->panel1);
			this->Name = L"UVForm";
			this->Text = L"UVForm";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &UVForm::UVForm_FormClosing);
			this->VisibleChanged += gcnew System::EventHandler(this, &UVForm::UVForm_VisibleChanged);
			this->ResumeLayout(false);

		}
#pragma endregion
	public: void SetModelPart(ModelPart* part) {
		uvPanel->part = part;
	}
	private: System::Void UVForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
		e->Cancel = true;
		this->Hide();
	}
	private: System::Void UVForm_VisibleChanged(System::Object^  sender, System::EventArgs^  e) {
		timer1->Enabled = this->Visible;
	}
	};
}
