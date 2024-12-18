#include <windows.h>
#include <commdlg.h>  // для диалогового окна 
#include <vector>
#include <map> 



HBITMAP LoadBMP(const wchar_t* filename);
void RemoveBackground(HBITMAP hBitmap);
void DrawImg(HDC hdc, HBITMAP hBitmap, int x, int y);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HBITMAP g_hBitmap = NULL;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    WNDCLASS wc = {}; //класс окна
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"BitmapWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    
    HWND hwnd = CreateWindowEx(0, L"BitmapWindowClass", L"Прога 2",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL); //окно

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {

        CreateWindow(L"BUTTON", L"Выберите BMP файл", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 150, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE), NULL);

    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) {
            OPENFILENAME bmp_file; //окно выбора файла
            wchar_t szFile[260] = {};
            ZeroMemory(&bmp_file, sizeof(bmp_file));
            bmp_file.lStructSize = sizeof(bmp_file);
            bmp_file.hwndOwner = hwnd;
            bmp_file.lpstrFilter = L"Bitmap Files\0*.BMP\0";
            bmp_file.lpstrFile = szFile;
            bmp_file.nMaxFile = sizeof(szFile);
            bmp_file.lpstrDefExt = L"bmp";

            if (GetOpenFileName(&bmp_file)) {
                g_hBitmap = LoadBMP(szFile);
                if (g_hBitmap) {
                    RemoveBackground(g_hBitmap); 
                    InvalidateRect(hwnd, NULL, TRUE);  
                }
            }
        }
        break;
    }
    case WM_PAINT: {
        if (g_hBitmap) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            BITMAP bmp;
            GetObject(g_hBitmap, sizeof(BITMAP), &bmp); //размер bmp

            RECT rect;
            GetClientRect(hwnd, &rect);
            int x = (rect.right - bmp.bmWidth) / 2;  //координаты
            int y = (rect.bottom - bmp.bmHeight) ; 

            DrawImg(hdc, g_hBitmap, x, y);

            EndPaint(hwnd, &ps);
        }
        break;
    }
    case WM_SIZE: {
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }
    case WM_DESTROY: {
        if (g_hBitmap) {
            DeleteObject(g_hBitmap);
        }
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

HBITMAP LoadBMP(const wchar_t* filename) {

    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return hBitmap;
}

void RemoveBackground(HBITMAP hBitmap) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);


    int width = bmp.bmWidth; //исходные размеры
    int height = bmp.bmHeight;

    HDC hdcMem = CreateCompatibleDC(NULL);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    std::vector<COLORREF> pixels(width * height);
    GetBitmapBits(hBitmap, width * height * sizeof(COLORREF), pixels.data());

    std::map<COLORREF, int> colorFrequency;
    for (const auto& pixel : pixels) {
        colorFrequency[pixel]++;
    }


    COLORREF backgroundColor = RGB(255, 255, 255); 
    int maxCount = 0;

    for (const auto& entry : colorFrequency) { //поиск фона
        if (entry.second > maxCount) {
            backgroundColor = entry.first;
            maxCount = entry.second;
        }
    }

    for (int i = 0; i < width * height; ++i) { //удаление фона
        if (pixels[i] == backgroundColor) {
            pixels[i] = RGB(255, 255, 255);  
        }
    }

    SetBitmapBits(hBitmap, width * height * sizeof(COLORREF), pixels.data());

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
}

void DrawImg(HDC hdc, HBITMAP hBitmap, int x, int y) {

    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
}
