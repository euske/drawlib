/*
 *  drawlib.c
 *  ウィンドウに図形を描くためのライブラリ関数。
 *  by Yusuke Shinyama
 *
 *  コンパイル方法: 
 *    C:\> cl test1.c drawlib.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

/*
 * 利用可能な関数 
 */

/* initwin(タイトル, 幅, 高さ): ウィンドウを開く。 */
extern int initwin(const char* title, int width, int height);
/* setcolor(R値, G値, B値): 現在の色を設定する。 */
extern void setcolor(int r, int g, int b);
/* pset(x座標, y座標): 指定された位置にピクセルを描画する。 */
extern void pset(int x, int y);
/* fill(x座標, y座標, 幅, 高さ): 指定された位置・大きさの矩形を描画する。 */
extern void fill(int x, int y, int w, int h);
/* clear(): 画面を消去する。 */
extern void clear(void);
/* refresh(): 現在の変更を画面に反映する。 */
extern void refresh(void);
/* sleep(ミリ秒): 指定された時間だけ待つ。 */
extern void sleep(int msec);
/* getkey(): 前回のrefresh()時に押されていたキーの番号を返す。 */
extern int getkey(void);
/* isopen(): ウィンドウが開いていれば 1 を返す。 */
extern int isopen(void);


/*
 * 内部的な関数およびデータ型。
 */

/* Screen: 描画画面をあらわす構造体。 */
typedef struct _Screen
{
    int width;
    int height;
    const char* name;
    COLORREF color;
    int key;
    int keybuf;
    
    HANDLE hThread;
    HWND hWnd;
    HBITMAP hBitmap;
    HDC hDC;
    CRITICAL_SECTION mutex;
} Screen;

/* ThreadParams: スレッド開始時の変数。 */
typedef struct _ThreadParams {
    Screen* self;
    HANDLE hEvent;
    int retval;
} ThreadParams;

/* アプリケーション全体にわたって使われる変数 */
static ATOM aScreenWindowClass = 0;
static HINSTANCE hInstance;

/* clamp(v0, v, v1): 値v が範囲におさまるよう調整。  */
static int clamp(int v0, int v, int v1)
{
    if (v < v0) {
        return v0;
    } else if (v1 < v) {
        return v1;
    } else {
        return v;
    }
}

/* ScreenSetColor: 現在の色を変更する。 */
static int ScreenSetColor(Screen* self, int r, int g, int b)
{
    r = clamp(0, r, 255);
    g = clamp(0, g, 255);
    b = clamp(0, b, 255);
    self->color = RGB(r, g, b);
    return 0;
}

/* ScreenFillRect: 描画バッファに矩形を描画する。 */
static int ScreenFillRect(Screen* self, int x, int y, int w, int h)
{
    x = clamp(0, x, self->width);
    y = clamp(0, y, self->height);
    w = clamp(0, x+w, self->width) - x;
    h = clamp(0, y+h, self->height) - y;
    if (self->hDC != NULL) {
        if (0 < w && 0 < h) {
            HBRUSH hBrush;
            RECT rect;
            EnterCriticalSection(&self->mutex);
            hBrush = CreateSolidBrush(self->color);
            SetRect(&rect, x, y, x+w, y+h);
            FillRect(self->hDC, &rect, hBrush);
            LeaveCriticalSection(&self->mutex);
        }
        return 0;
    }
    return -1;
}

/* ScreenSetPixel: 描画バッファにピクセルを描く。 */
static int ScreenSetPixel(Screen* self, int x, int y)
{
    if (self->hDC != NULL) {
        if (0 <= x && x < self->width &&
            0 <= y && y < self->height) {
            EnterCriticalSection(&self->mutex);
            SetPixel(self->hDC, x, y, self->color);
            LeaveCriticalSection(&self->mutex);
        }
        return 0;
    }
    return -1;
}

/* ScreenRepaint: 描画バッファの内容を転送する。 */
static int ScreenRepaint(Screen* self, HDC hDC)
{
    if (self->hDC != NULL) {
        EnterCriticalSection(&self->mutex);
        BitBlt(hDC, 0, 0, self->width, self->height,
               self->hDC, 0, 0, SRCCOPY);
        LeaveCriticalSection(&self->mutex);
        return 0;
    }
    return -1;
}

/* ScreenUninitBitmap: 描画バッファを解放する。 */
static int ScreenUninitBitmap(Screen* self)
{
    if (self->hBitmap != NULL) {
        DeleteObject(self->hBitmap);
        self->hBitmap = NULL;
    }
    if (self->hDC != NULL) {
        DeleteDC(self->hDC);
        self->hDC = NULL;
    }
    return 0;
}

/* ScreenInitBitmap: 描画バッファを初期化する。 */
static int ScreenInitBitmap(Screen* self, HDC hDC)
{
    self->hDC = CreateCompatibleDC(hDC);
    if (self->hDC != NULL) {
        self->hBitmap = CreateCompatibleBitmap(
            hDC, self->width, self->height);
        if (self->hBitmap != NULL) {
            SelectObject(self->hDC, self->hBitmap);
            ScreenSetColor(self, 255, 255, 255);
            return ScreenFillRect(self, 0, 0, self->width, self->height);
        }
    }
    /* エラーが発生した場合、資源を解放する。 */
    ScreenUninitBitmap(self);
    return -1;
}

/* screenWndProc: イベント処理関数。 */
static LRESULT CALLBACK screenWndProc(
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    /* イベントの種類によって分岐する。 */
    switch (uMsg) {
    case WM_PAINT:
        /* 再描画イベントが来たら描画する。 */
        {
            Screen* self = (Screen*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (self != NULL) {
                PAINTSTRUCT ps;
                HDC hDC = BeginPaint(hWnd, &ps);
                ScreenRepaint(self, hDC);
                EndPaint(hWnd, &ps);
            }
        }
        return 0;

    case WM_ERASEBKGND:
        /* 背景は描画しない。 */
        return 1;

    case WM_KEYDOWN:
        {
            Screen* self = (Screen*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (self != NULL) {
		self->keybuf = wParam;
	    }
	}
	return 0;
	
    case WM_KEYUP:
        {
            Screen* self = (Screen*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (self != NULL) {
		self->keybuf = 0;
	    }
	}
	return 0;
        
    case WM_CREATE:
        /* ウィンドウ作成時に、描画バッファを初期化する。 */
        {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            Screen* self = (Screen*)(cs->lpCreateParams);
            if (self != NULL) {
                HDC hDC = GetDC(hWnd);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)self);
                ScreenInitBitmap(self, hDC);
                ReleaseDC(hWnd, hDC);
            }
        }
        return 0;
        
    case WM_CLOSE:
        /* ウィンドウ消去時に、描画バッファを解放する。 */
        {
            Screen* self = (Screen*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (self != NULL) {
                ScreenUninitBitmap(self);
            }
        }
        DestroyWindow(hWnd);
        return 0;
        
    case WM_DESTROY:
        /* ウィンドウが閉じられた場合、スレッドを終了する。 */
        PostQuitMessage(0);
        return 0;
        
    default:
        /* それ以外のイベント。 */
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

/* InitWndClass: ウィンドウクラスを初期化する。 */
static int InitWndClass()
{
    if (aScreenWindowClass == 0) {
        /* ウィンドウクラスが未登録であれば登録する。 */
        static WNDCLASS klass;
        hInstance = GetModuleHandle(NULL);
        ZeroMemory(&klass, sizeof(klass));
        klass.lpfnWndProc = screenWndProc;
        klass.hInstance = hInstance;
        klass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        klass.hCursor = LoadCursor(NULL, IDC_ARROW);
        klass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        klass.lpszClassName = "screenWindowClass";
        aScreenWindowClass = RegisterClass(&klass);
    }
    return 0;
}

/* screenMainProc: スレッドのメイン関数。 */
static DWORD WINAPI screenMainProc(LPVOID params0)
{
    MSG msg;
    RECT bounds;
    DWORD dwStyle = (WS_CAPTION | WS_BORDER | WS_SYSMENU);
    ThreadParams* params = (ThreadParams*)params0;
    Screen* self = params->self;
    
    /* ウィンドウを作成する。 */
    SetRect(&bounds, 0, 0, self->width, self->height);
    AdjustWindowRect(&bounds, dwStyle, FALSE);
    self->hWnd = CreateWindow(
        (LPCSTR)aScreenWindowClass,
        self->name, dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right-bounds.left, bounds.bottom-bounds.top,
        NULL, NULL, hInstance, self);
    
    /* 作成したウィンドウを表示する。 */
    if (self->hWnd != NULL) {
        ShowWindow(self->hWnd, SW_SHOWNORMAL);
        UpdateWindow(self->hWnd);
        params->retval = 0;
    }
    SetEvent(params->hEvent);
    
    /* イベントループ。 */
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    self->hWnd = NULL;
    return msg.wParam;
}

/* ScreenUninitialize: 画面を解放する。 */
static int ScreenUninitialize(Screen* self)
{
    if (self->hThread != NULL) {
        WaitForSingleObject(self->hThread, INFINITE);
        CloseHandle(self->hThread);
        self->hThread = NULL;
    }
    return 0;
}

/* ScreenInitialize: 画面を初期化する。 */
static int ScreenInitialize(Screen* self, int width, int height, const char* name)
{
    ThreadParams params;
    ZeroMemory(self, sizeof(*self));
    self->width = width;
    self->height = height;
    self->name = name;
    self->color = RGB(0, 0, 0);
    
    InitializeCriticalSection(&self->mutex);
    params.self = self;
    params.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    params.retval = -1;
    if (params.hEvent != NULL) {
        /* イベント処理用のスレッドを作成。 */
        self->hThread = CreateThread(NULL, 0, screenMainProc, &params, 0, NULL);
        if (self->hThread != NULL) {
            /* 準備完了まで待つ。 */
            WaitForSingleObject(params.hEvent, INFINITE);
        }
        CloseHandle(params.hEvent);
    }
    
    return params.retval;
}


/*
 * ライブラリAPI
 */
static Screen* SCR1 = NULL;
static void terminate(void);

/* initwin(タイトル, 幅, 高さ): ウィンドウを開く。 */
int initwin(const char* title, int width, int height)
{
    if (SCR1 == NULL) {
        InitWndClass();
        SCR1 = (Screen*) malloc(sizeof(Screen));
        if (SCR1 != NULL) {
            atexit(terminate);
            return ScreenInitialize(SCR1, width, height, title);
        }
    }
    return -1;
}

/* terminate(): プログラム終了時の処理。 */
void terminate(void)
{
    if (SCR1 != NULL) {
        ScreenUninitialize(SCR1);
        SCR1 = NULL;
    }
}  

/* setcolor(R値, G値, B値): 現在の色を設定する。 */
void setcolor(int r, int g, int b)
{
    if (SCR1 != NULL) {
        ScreenSetColor(SCR1, r, g, b);
    }
}

/* pset(x座標, y座標): 指定された位置にピクセルを描画する。 */
void pset(int x, int y)
{
    if (SCR1 != NULL) {
        ScreenSetPixel(SCR1, x, y);
    }
}

/* fill(x座標, y座標, 幅, 高さ): 指定された位置・大きさの矩形を描画する。 */
void fill(int x, int y, int w, int h)
{
    if (SCR1 != NULL) {
        ScreenFillRect(SCR1, x, y, w, h);
    }
}

/* clear(): 画面を消去する。 */
void clear()
{
    if (SCR1 != NULL) {
        ScreenFillRect(SCR1, 0, 0, SCR1->width, SCR1->height);
    }
}

/* refresh(): 現在の変更を画面に反映する。 */
void refresh(void)
{
    if (SCR1 != NULL) {
        InvalidateRect(SCR1->hWnd, NULL, FALSE);
        UpdateWindow(SCR1->hWnd);
	SCR1->key = SCR1->keybuf;
    }
}

/* sleep(ミリ秒): 指定された時間だけ待つ。 */
void sleep(int msec)
{
    Sleep(msec);
}

/* getkey(): 前回のrefresh()時に押されていたキーの番号を返す。 */
int getkey()
{
    if (SCR1 != NULL) {
	return SCR1->key;
    }
    return -1;
}

/* isopen(): ウィンドウが開いていれば 1 を返す。 */
int isopen(void)
{
    if (SCR1 != NULL) {
	return (SCR1->hWnd != NULL);
    }
    return -1;
}    


/*
 * テスト用プログラム
 */
void test1(void)
{
    int x, y;
    initwin("test1", 400, 300);
    for (x = 0; x < 255; x++) {
        for (y = 0; y < 255; y++) {
            setcolor(x, 255, 255-y);
            pset(x, y);
        }
    }
    refresh();
}

void test2(void)
{
    int x, y, vy;
    initwin("test2", 600, 400);
    y = 0;
    vy = 1;
    for (x = 0; x < 500; x++) {
        setcolor(255, x % 256, 0);
        fill(x, y, 100, 100);
        if (y <= 0) {
            vy = 1;
        } else if (300 <= y) {
            vy = -1;
        }
        y = y + vy*4;
        refresh();
        sleep(20);
    }
}

void test3(void)
{
    int x, y, k;
    initwin("test3", 400, 300);
    x = 100;
    y = 100;
    setcolor(0, 0, 255);
    while (isopen()) {
	k = getkey();
	if (k == VK_LEFT) {
	    x = x - 1;
	} else if (k == VK_RIGHT) {
	    x = x + 1;
	} else if (k == VK_UP) {
	    y = y - 1;
	} else if (k == VK_DOWN) {
	    y = y + 1;
	}
        fill(x, y, 25, 25);
        refresh();
        sleep(20);
    }
}
