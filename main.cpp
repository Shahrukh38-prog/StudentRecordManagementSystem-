#include <windows.h>
#include <commctrl.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace std;

// --- Data Structure ---
struct Student {
    string name;
    string rollNo;
    string fatherName;
    string caste;
    string district;
    string gpa;
};

// --- Global Variables ---
const string FILENAME = "students.txt";
vector<Student> students;
int currentIndex = -1;
bool isAddingMode = true;

// GUI Control Handles
HWND hName, hRoll, hFather, hCaste, hDistrict, hGPA;
HWND hBtnAdd, hBtnUpdate, hBtnDelete, hBtnNext;
HWND hStatus;

// --- Helper Functions ---
string GetText(HWND hEdit) {
    int len = GetWindowTextLength(hEdit) + 1;
    char* buffer = new char[len];
    GetWindowText(hEdit, buffer, len);
    string text(buffer);
    delete[] buffer;
    return text;
}

void SetText(HWND hEdit, const string& text) {
    SetWindowText(hEdit, text.c_str());
}

void EnableWindowSafe(HWND hWnd, bool enable) {
    EnableWindow(hWnd, enable ? TRUE : FALSE);
}

// --- File Handling ---
void loadFromFile() {
    students.clear();
    ifstream file(FILENAME);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string item;
        Student s;
        getline(ss, s.name, ',');
        getline(ss, s.rollNo, ',');
        getline(ss, s.fatherName, ',');
        getline(ss, s.caste, ',');
        getline(ss, s.district, ',');
        getline(ss, s.gpa, ',');
        students.push_back(s);
    }
    file.close();
}

void saveToFile() {
    ofstream file(FILENAME);
    if (!file.is_open()) {
        MessageBox(NULL, "Error: Could not save to file!", "Error", MB_ICONERROR);
        return;
    }
    for (const auto& s : students) {
        file << s.name << "," << s.rollNo << "," << s.fatherName << ","
             << s.caste << "," << s.district << "," << s.gpa << "\n";
    }
    file.close();
}

// --- GUI Logic ---
void updateGUIState() {
    if (isAddingMode) {
        EnableWindowSafe(hBtnAdd, true);
        EnableWindowSafe(hBtnUpdate, false);
        EnableWindowSafe(hBtnDelete, false);
        EnableWindowSafe(hBtnNext, true);
        SetWindowText(hStatus, "Mode: Add New Record");
    } else {
        EnableWindowSafe(hBtnAdd, false);
        EnableWindowSafe(hBtnUpdate, true);
        EnableWindowSafe(hBtnDelete, true);
        EnableWindowSafe(hBtnNext, true);
        SetWindowText(hStatus, "Mode: View/Edit Record");
    }
}

void clearInputs() {
    SetText(hName, "");
    SetText(hRoll, "");
    SetText(hFather, "");
    SetText(hCaste, "");
    SetText(hDistrict, "");
    SetText(hGPA, "");
}

void displayStudent(int index) {
    if (index >= 0 && index < (int)students.size()) {
        SetText(hName, students[index].name);
        SetText(hRoll, students[index].rollNo);
        SetText(hFather, students[index].fatherName);
        SetText(hCaste, students[index].caste);
        SetText(hDistrict, students[index].district);
        SetText(hGPA, students[index].gpa);
    } else {
        clearInputs();
    }
}

// --- Button Actions ---
void doAdd() {
    string name = GetText(hName);
    string roll = GetText(hRoll);
    
    if (name.empty() || roll.empty()) {
        MessageBox(NULL, "Name and Roll Number are required!", "Validation Error", MB_ICONWARNING);
        return;
    }

    for (const auto& s : students) {
        if (s.rollNo == roll) {
            MessageBox(NULL, "Error: A student with this Roll Number already exists!", "Error", MB_ICONERROR);
            return;
        }
    }

    Student newStudent;
    newStudent.name = name;
    newStudent.rollNo = roll;
    newStudent.fatherName = GetText(hFather);
    newStudent.caste = GetText(hCaste);
    newStudent.district = GetText(hDistrict);
    newStudent.gpa = GetText(hGPA);

    students.push_back(newStudent);
    saveToFile();
    
    MessageBox(NULL, "Student record added successfully!", "Success", MB_ICONINFORMATION);
    clearInputs();
    currentIndex = -1;
    isAddingMode = true;
    updateGUIState();
}

void doUpdate() {
    if (currentIndex < 0 || currentIndex >= (int)students.size()) {
        MessageBox(NULL, "No record selected to update.", "Error", MB_ICONERROR);
        return;
    }

    students[currentIndex].name = GetText(hName);
    students[currentIndex].fatherName = GetText(hFather);
    students[currentIndex].caste = GetText(hCaste);
    students[currentIndex].district = GetText(hDistrict);
    students[currentIndex].gpa = GetText(hGPA);

    saveToFile();
    MessageBox(NULL, "Student record updated successfully!", "Success", MB_ICONINFORMATION);
}

void doDelete() {
    if (currentIndex < 0 || currentIndex >= (int)students.size()) {
        MessageBox(NULL, "No record selected to delete.", "Error", MB_ICONERROR);
        return;
    }

    int msgboxID = MessageBox(NULL, "Are you sure you want to delete this record?", "Confirm Delete", MB_ICONWARNING | MB_YESNO);
    if (msgboxID == IDYES) {
        students.erase(students.begin() + currentIndex);
        saveToFile();
        MessageBox(NULL, "Record deleted successfully.", "Success", MB_ICONINFORMATION);
        
        currentIndex = -1;
        clearInputs();
        isAddingMode = true;
        updateGUIState();
    }
}

void doNext() {
    if (students.empty()) {
        MessageBox(NULL, "No records available to navigate.", "Info", MB_ICONINFORMATION);
        return;
    }

    if (isAddingMode) {
        isAddingMode = false;
        currentIndex = 0;
        displayStudent(currentIndex);
        updateGUIState();
    } else {
        currentIndex++;
        if (currentIndex >= (int)students.size()) {
            currentIndex = 0; // Loop
        }
        displayStudent(currentIndex);
    }
}

// --- Windows Procedure ---
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            // Create Labels and Inputs
            CreateWindow("STATIC", "Student Name:", WS_VISIBLE | WS_CHILD, 20, 20, 120, 25, hWnd, NULL, NULL, NULL);
            hName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 150, 20, 200, 25, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Roll Number:", WS_VISIBLE | WS_CHILD, 20, 60, 120, 25, hWnd, NULL, NULL, NULL);
            hRoll = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 150, 60, 200, 25, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Father's Name:", WS_VISIBLE | WS_CHILD, 20, 100, 120, 25, hWnd, NULL, NULL, NULL);
            hFather = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 150, 100, 200, 25, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Caste:", WS_VISIBLE | WS_CHILD, 20, 140, 120, 25, hWnd, NULL, NULL, NULL);
            hCaste = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 150, 140, 200, 25, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "District:", WS_VISIBLE | WS_CHILD, 20, 180, 120, 25, hWnd, NULL, NULL, NULL);
            hDistrict = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 150, 180, 200, 25, hWnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Student GPA:", WS_VISIBLE | WS_CHILD, 20, 220, 120, 25, hWnd, NULL, NULL, NULL);
            hGPA = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 150, 220, 200, 25, hWnd, NULL, NULL, NULL);

            // Create Buttons
            hBtnAdd = CreateWindow("BUTTON", "Add", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 20, 270, 80, 30, hWnd, (HMENU)1, NULL, NULL);
            hBtnUpdate = CreateWindow("BUTTON", "Update", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 110, 270, 80, 30, hWnd, (HMENU)2, NULL, NULL);
            hBtnDelete = CreateWindow("BUTTON", "Delete", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 200, 270, 80, 30, hWnd, (HMENU)3, NULL, NULL);
            hBtnNext = CreateWindow("BUTTON", "Next", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 290, 270, 80, 30, hWnd, (HMENU)4, NULL, NULL);

            // Status Box
            hStatus = CreateWindow("STATIC", "Mode: Add New Record", WS_VISIBLE | WS_CHILD, 20, 310, 350, 25, hWnd, NULL, NULL, NULL);

            updateGUIState();
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wp);
            switch (wmId) {
                case 1: doAdd(); break;
                case 2: doUpdate(); break;
                case 3: doDelete(); break;
                case 4: doNext(); break;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, msg, wp, lp);
    }
    return 0;


// --- WinMain (Entry Point for Windows App) ---
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
    // Load data at startup
    loadFromFile();

    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"StudentRecordClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc)) return -1;

    CreateWindowW(L"StudentRecordClass", L"Student Record Management System", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 300, 200, 420, 380, NULL, NULL, NULL, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}