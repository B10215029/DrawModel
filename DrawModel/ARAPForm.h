#pragma once
#include "OpenGLEventTrigger.h"
#include "ARAPPanel.h"

namespace DrawModel {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// ARAPForm 的摘要
	/// </summary>
	public ref class ARAPForm : public System::Windows::Forms::Form
	{
	public:
		ARAPForm(void)
		{
			InitializeComponent();
			//
			//TODO:  在此加入建構函式程式碼
			//
			aRAPPanel = new ARAPPanel();
			gcnew OpenGLEventTrigger(panel1, timer1, aRAPPanel);
		}

	protected:
		/// <summary>
		/// 清除任何使用中的資源。
		/// </summary>
		~ARAPForm()
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
		/// 設計工具所需的變數。
		/// </summary>
		ARAPPanel* aRAPPanel;


#pragma region Windows Form Designer generated code
		/// <summary>
		/// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器修改
		/// 這個方法的內容。
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
			// ARAPForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->panel1);
			this->Name = L"ARAPForm";
			this->Text = L"ARAPForm";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &ARAPForm::ARAPForm_FormClosing);
			this->VisibleChanged += gcnew System::EventHandler(this, &ARAPForm::ARAPForm_VisibleChanged);
			this->ResumeLayout(false);

		}
#pragma endregion
	public: void SetModelPart(ModelPart* part) {
		aRAPPanel->SetPart(part);
	}
	private: System::Void ARAPForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
		e->Cancel = true;
		this->Hide();
	}
	private: System::Void ARAPForm_VisibleChanged(System::Object^  sender, System::EventArgs^  e) {
		timer1->Enabled = this->Visible;
	}
	};
}
