//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm2 *Form2;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
UnicodeString TForm2::GetText() {
	return text;
}
void __fastcall TForm2::Button1Click(TObject *Sender)
{
	this->text = Edit1->Text;
}
//---------------------------------------------------------------------------

