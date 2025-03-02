﻿//---------------------------------------------------------------------------
#include <vcl.h>
#include <vector>

#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"

#include <Clipbrd.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

//---------------------------------------------------------------------------
using namespace std;


#include <string>

#include <boost/filesystem/operations.hpp>
#include <fstream>
namespace bfs = boost::filesystem;

class File {
	bfs::path path;
	bool is_directory = false;
    bool onEdit = false;
	UnicodeString tempText;

	public:
	File(bfs::path path) : path(path) {}
	bfs::path getPath() {
		return this->path;
	}
	bool getOnEdit() {
		return this->onEdit;
	}
	void setIsDirectory() {
        this->is_directory = true;
	}
	void setOnEdit(bool r) {
        this->onEdit = r;
	}

	void setTempText(UnicodeString t) {
        this->tempText = t;
	}
	UnicodeString getTempText() {
        return this->tempText;
	}
	bool getIsDirectory() {
        this->is_directory;
    }
};


std::vector<TFileName> filenames;
std::vector<File> paths;
std::vector<File> tabPaths; // class - path, onEdit
bfs::path ProjectPath;
bool onEdit = false;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
RichEditMain->Lines->Clear();
}





void __fastcall TForm1::CheckCursorPosition(){
	int cursorPos = RichEditMain->SelStart;
	int lineIndex = RichEditMain->Perform(EM_LINEFROMCHAR, cursorPos, 0); // Индекс строки
	int charIndex = cursorPos - RichEditMain->Perform(EM_LINEINDEX, lineIndex, 0); // Позиция в строке
	//Application->MessageBox(IntToStr(lineIndex + 1).c_str(), IntToStr(charIndex + 1).c_str(), MB_OK);

	StatusBar1->Panels->Items[1]->Text = IntToStr(lineIndex + 1) + ":" + IntToStr(charIndex + 1);

}
//---------------------------------------------------------------------------
void AddDirectoryToTreeView(TTreeView *TreeView, TTreeNode *ParentNode, const bfs::path &DirPath) {
    if (!bfs::exists(DirPath) || !bfs::is_directory(DirPath)) {
        return;
	}
	for (const auto &entry : bfs::directory_iterator(DirPath)) {
		paths.push_back(File(entry));

		if (bfs::is_directory(entry)) {
			paths[paths.size() - 1].setIsDirectory();
            TTreeNode *DirNode = TreeView->Items->AddChild(ParentNode, entry.path().filename().wstring().c_str());
            
            AddDirectoryToTreeView(TreeView, DirNode, entry.path());
        } else if (bfs::is_regular_file(entry)) {

            TreeView->Items->AddChild(ParentNode, entry.path().filename().wstring().c_str());
        }
    }
}
void __fastcall TForm1::Open1Click(TObject *Sender)
{
	tabPaths.clear();
    paths.clear();
	TabControl1->Tabs->Clear();
	TreeView1->Items->Clear();
	int RC = OpenDialogFolder->Execute();
	if (RC) {
		RichEditMain->Lines->LoadFromFile(OpenDialogFolder->FileName);
		filenames.push_back(OpenDialogFolder->FileName);

		ProjectPath = OpenDialogFolder->FileName.c_str();
		TabControl1->Tabs->Append(ProjectPath.filename().c_str());
		tabPaths.push_back(File(ProjectPath));
        tabPaths[TabControl1->TabIndex].setTempText(RichEditMain->Lines->Text);
		TabControl1->TabIndex = (TabControl1->Tabs->Capacity - 1);
		ProjectPath = ProjectPath.parent_path().append("\\");

		TTreeNode * node = NULL;

		AddDirectoryToTreeView(TreeView1, NULL, ProjectPath);
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TabControl1Change(TObject *Sender)
{
	auto current = TabControl1->TabIndex;
	// RichEditMain->Lines->LoadFromFile(tabPaths[current].getPath().c_str());
	RichEditMain->Lines->Clear();
	ShowMessage(tabPaths[current].getTempText());
	RichEditMain->Lines->Text = (tabPaths[current].getTempText());
}
//---------------------------------------------------------------------------



void __fastcall TForm1::TreeView1Change(TObject *Sender, TTreeNode *Node)
{
	auto curr = TreeView1->Selected;
	int ind = curr->AbsoluteIndex;
	if (curr->HasChildren) {
		return;
	}
	auto filename = TreeView1->Selected->Text;
	bool res = TabControl1->Tabs->Contains(filename);

	if (res) {
		//ShowMessage("true!");

		TabControl1->TabIndex = TabControl1->Tabs->IndexOf(filename);
		//RichEditMain->Lines->LoadFromFile(tabPaths[TabControl1->TabIndex].getPath().c_str());

		RichEditMain->Lines->Clear();
		RichEditMain->Lines->Text = ((tabPaths[TabControl1->TabIndex].getTempText()));

		tabPaths[TabControl1->TabIndex].setTempText(RichEditMain->Lines->Text);
		return;
	}
	else {
		//ShowMessage("false!");

		TabControl1->Tabs->Append(filename);
		TabControl1->TabIndex = (TabControl1->Tabs->Capacity - 1);
		tabPaths.push_back(paths[ind]);

		RichEditMain->Lines->LoadFromFile(tabPaths[TabControl1->TabIndex].getPath().c_str());
		tabPaths[TabControl1->TabIndex].setTempText(RichEditMain->Lines->Text);

		return;
    }


}
//---------------------------------------------------------------------------



void __fastcall TForm1::TabControl1MouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
	if (Button == mbRight) {
		int TabIndex = TabControl1->IndexOfTabAt(X, Y);
		TabControl1->TabIndex = TabIndex;       // i don't know need this or not
		if (TabIndex != -1) {
			PopupMenuTabs->Popup(Mouse->CursorPos.x, Mouse->CursorPos.y);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N1Click(TObject *Sender)
{
	int ind = TabControl1->TabIndex;
	tabPaths.erase(tabPaths.begin() + ind);
	TabControl1->Tabs->Delete(ind);
	RichEditMain->Lines->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RichEditMainKeyPress(TObject *Sender, System::WideChar &Key)
{
	CheckCursorPosition();
	int index = TabControl1->TabIndex;



// maybe in this func add hotkeys???
	if (!tabPaths[index].getOnEdit()) {
		auto str = TabControl1->Tabs->Strings[index];

		str += L"*";
		TabControl1->Tabs->Strings[index] = str;
	}
	// расположение команд ниже проверь, а то хз
	tabPaths[index].setOnEdit(true);
	tabPaths[index].setTempText(RichEditMain->Lines->Text);

	// and save temp text

}
//---------------------------------------------------------------------------

UnicodeString GetName() {
	Form2->ShowModal();
	UnicodeString text;
	while(text != NULL) {
		text = Form2->GetText();
    }
	Form2->Close();
    return text;
}


void __fastcall TForm1::RichEditMainClick(TObject *Sender)
{
	CheckCursorPosition();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RichEditMainChange(TObject *Sender)
{
	CheckCursorPosition();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RichEditMainKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	CheckCursorPosition();

    int index = TabControl1->TabIndex;
	if ((Shift.Contains(ssCtrl)) && (Key == 'S' || Key == 's') && index != -1) {
		//ShowMessage("Ctrl+S нажата!");
		//Key = 0; // Чтобы предотвратить дальнейшую обработку клавиши

		if (tabPaths[index].getOnEdit) {
		   RichEditMain->Lines->SaveToFile(tabPaths[index].getPath().c_str());

		   TabControl1->Tabs->Strings[index] = tabPaths[index].getPath().filename().c_str();
		   ShowMessage(tabPaths[index].getPath().filename().c_str()); // почему оно меняется только если showMessage?
		   TabControl1->Repaint();
		}
		tabPaths[index].setOnEdit(false);

    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TreeView1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
	if (Button == mbRight) {
		int TabIndex = TreeView1->Selected->AbsoluteIndex;
		TreeView1->Selected = TreeView1->Items->Item[TabIndex];       // i don't know need this or not

		if (TabIndex != -1) {
			PopupMenuFiles->Popup(Mouse->CursorPos.x, Mouse->CursorPos.y);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N3Click(TObject *Sender)
{
	tabPaths.clear();
	RichEditMain->Lines->Clear();
	TabControl1->Tabs->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N2Click(TObject *Sender)
{
	int ind = TabControl1->TabIndex;

	File temp = tabPaths[ind];

	TabControl1->Tabs->Clear();
    TabControl1->Tabs->Append(temp.getPath().filename().c_str());

	tabPaths.clear();
    tabPaths.push_back(temp);
	RichEditMain->Lines->Clear();
	RichEditMain->Lines->Text = ((tabPaths[0].getTempText()));
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N4Click(TObject *Sender)
{
	int ind = TabControl1->TabIndex;
	Clipboard()->AsText = tabPaths[ind].getPath().c_str();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N5Click(TObject *Sender)
{
	int ind = TabControl1->TabIndex;
	Clipboard()->AsText = tabPaths[ind].getPath().relative_path().c_str();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N12Click(TObject *Sender)
{
	int ind = TreeView1->Selected->AbsoluteIndex;
	Clipboard()->AsText = paths[ind].getPath().c_str();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N13Click(TObject *Sender)
{
	int ind = TreeView1->Selected->AbsoluteIndex;
	Clipboard()->AsText = paths[ind].getPath().relative_path().c_str();
}

//---------------------------------------------------------------------------


void __fastcall TForm1::N15Click(TObject *Sender)
{
// sometimes an error occured, where prog don't open a file, which not exists
	auto node = TreeView1->Selected;

	bool res, isDir = bfs::is_directory(paths[node->AbsoluteIndex].getPath());
	if (isDir){
		res = bfs::remove_all(paths[node->AbsoluteIndex].getPath()); 
		TreeView1->Items->Delete(node);
	}
	else
	{
		res = bfs::remove(paths[node->AbsoluteIndex].getPath()); 

		int index = TabControl1->Tabs->IndexOf(paths[node->AbsoluteIndex].getPath().filename().c_str());
		ShowMessage(IntToStr(index));

		if (index != 0) {
            TabControl1->Tabs->Delete(index);
			TreeView1->Items->Delete(node);
			RichEditMain->Lines->Clear();
			RichEditMain->Lines->Text = (tabPaths[TabControl1->TabIndex].getTempText());
		}
		else {
			TabControl1->Tabs->Delete(index);
			TreeView1->Items->Delete(node);
			RichEditMain->Lines->Clear();

		}
		/*
		TabControl1->Tabs->Delete(index);
		TreeView1->Items->Delete(node);
		RichEditMain->Lines->Clear()
		RichEditMain->Lines->Text = (tabPaths[TabControl1->TabIndex].getTempText());
                    */
		// what if no tabs??
    }
		

	// RichEdit line insertion error (bcs of chinese symbols as i assume) wrong file

	// if isDir condition


	paths.erase(paths.begin() + node->AbsoluteIndex); // need this for folder and childs? 
	

	

}
//---------------------------------------------------------------------------

int GetNodeAbsoluteIndex(TTreeView* TreeView, TTreeNode* NodeToFind) {
    if (!TreeView || !NodeToFind) {
        return -1; // Некорректные параметры
    }

    int index = 0;
    TTreeNode* currentNode = TreeView->Items->GetFirstNode();

	while (currentNode) {
        if (currentNode == NodeToFind) {
            return index; // Возвращаем абсолютный индекс
        }
        currentNode = currentNode->GetNext(); // Переходим к следующему узлу
        index++;
    }

    return -1; // Узел не найден
}

void __fastcall TForm1::N7Click(TObject *Sender)
{
	auto node = TreeView1->Selected;
	auto fileSelected = paths[node->AbsoluteIndex];
	bfs::path path = fileSelected.getPath();

//	UnicodeString namefile = GetName(); // get done bcs not working
	//bfs::path file(namefile.c_str());

	bfs::path file("text.txt");

	ShowMessage(fileSelected.getPath().c_str());
    // refactor

	if (bfs::is_directory(fileSelected.getPath())) {    // later maybe get rid of isDirectory in class
		ShowMessage("true");
		path /= file;
		ShowMessage(path.string().c_str());
		std::ofstream ofs{path.string()};       // КИТАЙСКИЕ СИМВОЛЫ ЭТО ЧТО ЗА но в редакторе обычно

		File f(path);

		// если файл уже есть, то создается 2 копия в treeview
		TTreeNode* nodeFile = TreeView1->Items->AddChild(node, file.string().c_str());
		int indexFile = GetNodeAbsoluteIndex(TreeView1, nodeFile);
		paths.insert(paths.begin() + indexFile, f);
	}
	else {
		ShowMessage("false");
		path = path.parent_path();
		path /= file;
		ShowMessage(path.string().c_str());

		std::ofstream ofs{path.string()};       // КИТАЙСКИЕ СИМВОЛЫ ЭТО ЧТО ЗА  но в редакторе обычно

		File f(path);

		TTreeNode* nodeFile = TreeView1->Items->Add(node, file.string().c_str());
		int indexFile = GetNodeAbsoluteIndex(TreeView1, nodeFile);
		paths.insert(paths.begin() + indexFile, f);
    }
//	File addedFile();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::N8Click(TObject *Sender)
{
	auto node = TreeView1->Selected;
	auto fileSelected = paths[node->AbsoluteIndex];
	bfs::path path = fileSelected.getPath();
	bfs::path folderName("testFolder");


	if (bfs::is_directory(fileSelected.getPath())) {
		bfs::path f(path / folderName);
		bfs::create_directory(f);
		ShowMessage(f.string().c_str());

		File folder(f);
		folder.setIsDirectory();

		TTreeNode* nodeFolder = TreeView1->Items->AddChild(node, folderName.string().c_str());
		nodeFolder->HasChildren = true;
		int indexFile = GetNodeAbsoluteIndex(TreeView1, nodeFolder);
		paths.insert(paths.begin() + indexFile, folder);

	}
	else {
		bfs::path f(path.parent_path() / folderName);
		bfs::create_directory(f);
		ShowMessage(f.string().c_str());

		File folder(f);

		TTreeNode* nodeFolder = TreeView1->Items->Add(node, folderName.string().c_str());
		nodeFolder->HasChildren = true;
		int indexFile = GetNodeAbsoluteIndex(TreeView1, nodeFolder);
		paths.insert(paths.begin() + indexFile, folder);
    }

}
//---------------------------------------------------------------------------

