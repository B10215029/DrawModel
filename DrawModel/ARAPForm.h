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
	/// ARAPForm ���K�n
	/// </summary>
	public ref class ARAPForm : public System::Windows::Forms::Form
	{
	public:
		ARAPForm(void)
		{
			InitializeComponent();
			//
			//TODO:  �b���[�J�غc�禡�{���X
			//
			aRAPPanel = new ARAPPanel();
			gcnew OpenGLEventTrigger(panel1, timer1, aRAPPanel);
		}

	protected:
		/// <summary>
		/// �M������ϥΤ����귽�C
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
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel1;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel2;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::CheckBox^  checkBox1;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	private: System::Windows::Forms::SaveFileDialog^  saveFileDialog1;
	private: System::Windows::Forms::CheckBox^  checkBox2;
	protected:

	private:
		/// <summary>
		/// �]�p�u��һݪ��ܼơC
		/// </summary>
		ARAPPanel* aRAPPanel;


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
			this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->tableLayoutPanel2 = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->saveFileDialog1 = (gcnew System::Windows::Forms::SaveFileDialog());
			this->checkBox2 = (gcnew System::Windows::Forms::CheckBox());
			this->tableLayoutPanel1->SuspendLayout();
			this->tableLayoutPanel2->SuspendLayout();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->panel1->Location = System::Drawing::Point(4, 4);
			this->panel1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(339, 318);
			this->panel1->TabIndex = 0;
			// 
			// timer1
			// 
			this->timer1->Interval = 30;
			// 
			// tableLayoutPanel1
			// 
			this->tableLayoutPanel1->ColumnCount = 2;
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
				133)));
			this->tableLayoutPanel1->Controls->Add(this->panel1, 0, 0);
			this->tableLayoutPanel1->Controls->Add(this->tableLayoutPanel2, 1, 0);
			this->tableLayoutPanel1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanel1->Location = System::Drawing::Point(0, 0);
			this->tableLayoutPanel1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
			this->tableLayoutPanel1->RowCount = 1;
			this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 100)));
			this->tableLayoutPanel1->Size = System::Drawing::Size(480, 326);
			this->tableLayoutPanel1->TabIndex = 1;
			// 
			// tableLayoutPanel2
			// 
			this->tableLayoutPanel2->ColumnCount = 1;
			this->tableLayoutPanel2->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				100)));
			this->tableLayoutPanel2->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
				27)));
			this->tableLayoutPanel2->Controls->Add(this->button1, 0, 0);
			this->tableLayoutPanel2->Controls->Add(this->checkBox1, 0, 1);
			this->tableLayoutPanel2->Controls->Add(this->checkBox2, 0, 2);
			this->tableLayoutPanel2->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanel2->Location = System::Drawing::Point(351, 4);
			this->tableLayoutPanel2->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tableLayoutPanel2->Name = L"tableLayoutPanel2";
			this->tableLayoutPanel2->RowCount = 5;
			this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 20)));
			this->tableLayoutPanel2->Size = System::Drawing::Size(125, 318);
			this->tableLayoutPanel2->TabIndex = 1;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(4, 4);
			this->button1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(100, 29);
			this->button1->TabIndex = 0;
			this->button1->Text = L"Ū�ʵe";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &ARAPForm::button1_Click);
			// 
			// checkBox1
			// 
			this->checkBox1->AutoSize = true;
			this->checkBox1->Location = System::Drawing::Point(4, 67);
			this->checkBox1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->checkBox1->Name = L"checkBox1";
			this->checkBox1->Size = System::Drawing::Size(59, 19);
			this->checkBox1->TabIndex = 1;
			this->checkBox1->Text = L"���s";
			this->checkBox1->UseVisualStyleBackColor = true;
			this->checkBox1->CheckedChanged += gcnew System::EventHandler(this, &ARAPForm::checkBox1_CheckedChanged);
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			// 
			// checkBox2
			// 
			this->checkBox2->AutoSize = true;
			this->checkBox2->Location = System::Drawing::Point(3, 129);
			this->checkBox2->Name = L"checkBox2";
			this->checkBox2->Size = System::Drawing::Size(111, 19);
			this->checkBox2->TabIndex = 2;
			this->checkBox2->Text = L"mapping to 3d";
			this->checkBox2->UseVisualStyleBackColor = true;
			this->checkBox2->CheckedChanged += gcnew System::EventHandler(this, &ARAPForm::checkBox2_CheckedChanged);
			// 
			// ARAPForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 15);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(480, 326);
			this->Controls->Add(this->tableLayoutPanel1);
			this->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->Name = L"ARAPForm";
			this->Text = L"ARAPForm";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &ARAPForm::ARAPForm_FormClosing);
			this->VisibleChanged += gcnew System::EventHandler(this, &ARAPForm::ARAPForm_VisibleChanged);
			this->tableLayoutPanel1->ResumeLayout(false);
			this->tableLayoutPanel2->ResumeLayout(false);
			this->tableLayoutPanel2->PerformLayout();
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
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			aRAPPanel->ReadAnimation((const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(openFileDialog1->FileName)).ToPointer());
		}
	}
private: System::Void checkBox1_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	if (checkBox1->Checked) {
		if (saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			aRAPPanel->StartOutputAnimation((const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(saveFileDialog1->FileName)).ToPointer());
		}
	}
	else {
		aRAPPanel->EndOutputAnimation();
	}
}
private: System::Void checkBox2_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	ARAPPanel::fastMapping = checkBox2->Checked;
}
};
}
