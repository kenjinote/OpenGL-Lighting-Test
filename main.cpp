#define UNICODE
#define _UNICODE
#pragma comment(linker,"/opt:nowin98")
#pragma comment(lib,"opengl32")
#pragma comment(lib,"glu32")
#pragma comment(lib,"glaux")
#pragma comment(lib,"comctl32")
#include<windows.h>
#include<commctrl.h>
#include<tchar.h>
#include<gl\gl.h>
#include"glut.h"
#include"resource.h"

extern "C" long _ftol( double );
extern "C" long _ftol2( double dblSource ) { return _ftol( dblSource ); }

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_ASPECT ((float)WINDOW_WIDTH/(float)WINDOW_HEIGHT)

HDC hDC=NULL;
HGLRC hRC=NULL;
TCHAR szClassName[]=TEXT("Window");
HWND hLightDialogWnd;
HWND hMaterialDialogWnd;
GLfloat rot[2];

static GLfloat lit_amb[4]={0.0f,0.0f,0.0f,0.0f};/* 環境光の強さ */
static GLfloat lit_dif[4]={0.0f,0.0f,0.0f,0.0f};/* 拡散光の強さ */
static GLfloat lit_spc[4]={0.0f,0.0f,0.0f,0.0f};/* 鏡面反射光の強さ */
static GLfloat lit_pos[4]={0.0f,0.0f,0.0f,1.0f};/* 光源の位置 */
static GLfloat mat_amb[4]={0.0f,0.0f,0.0f,0.0f};/* 環境光に対する反射 */
static GLfloat mat_dif[4]={0.0f,0.0f,0.0f,0.0f};/* 拡散光に対する反射 */
static GLfloat mat_spc[4]={0.0f,0.0f,0.0f,0.0f};/* 鏡面反射 */
static GLfloat mat_emi[4]={0.0f,0.0f,0.0f,0.0f};/* 発光 */
static GLfloat mat_shi[1]={0.0f};				/* 光沢 */

typedef struct {
    int nEditID;
    int nSliderID;
    GLfloat*fOutValue;
} data_t;

BOOL CALLBACK LightDialogProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static const data_t data[] = 
	{{IDC_EDIT_AMBIENT_R,IDC_SLIDER_AMBIENT_R,&lit_amb[0]},
	{IDC_EDIT_AMBIENT_G,IDC_SLIDER_AMBIENT_G,&lit_amb[1]},
	{IDC_EDIT_AMBIENT_B,IDC_SLIDER_AMBIENT_B,&lit_amb[2]},
	{IDC_EDIT_DIFFUSE_R,IDC_SLIDER_DIFFUSE_R,&lit_dif[0]},
	{IDC_EDIT_DIFFUSE_G,IDC_SLIDER_DIFFUSE_G,&lit_dif[1]},
	{IDC_EDIT_DIFFUSE_B,IDC_SLIDER_DIFFUSE_B,&lit_dif[2]},
	{IDC_EDIT_SPECULAR_R,IDC_SLIDER_SPECULAR_R,&lit_spc[0]},
	{IDC_EDIT_SPECULAR_G,IDC_SLIDER_SPECULAR_G,&lit_spc[1]},
	{IDC_EDIT_SPECULAR_B,IDC_SLIDER_SPECULAR_B,&lit_spc[2]},
	{IDC_EDIT_POSITION_X,IDC_SLIDER_POSITION_X,&lit_pos[0]},
	{IDC_EDIT_POSITION_Y,IDC_SLIDER_POSITION_Y,&lit_pos[1]},
	{IDC_EDIT_POSITION_Z,IDC_SLIDER_POSITION_Z,&lit_pos[2]}};
	static TCHAR szText[256];
	switch(msg)
	{
	case WM_HSCROLL:
		{
			data_t* p=(data_t*)GetWindowLong((HWND)lParam,GWL_USERDATA);
			if(
				p->nSliderID==IDC_SLIDER_POSITION_X||
				p->nSliderID==IDC_SLIDER_POSITION_Y||
				p->nSliderID==IDC_SLIDER_POSITION_Z
				)
			{
				*(p->fOutValue)=SendMessage(
					(HWND)lParam,TBM_GETPOS,NULL,NULL)/25.0f-2.0f;
			}
			else
			{
				*(p->fOutValue)=SendMessage(
					(HWND)lParam,TBM_GETPOS,NULL,NULL)/100.0f;
			}
			_stprintf(szText,TEXT("%.2f"),*(p->fOutValue));
			SetDlgItemText(hWnd,p->nEditID,szText);
		}
		InvalidateRect(GetParent(hWnd),0,0);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_BUTTON_DEFAULT)
		{
            lit_amb[0] = lit_amb[1] = lit_amb[2] = 0.8f;
            lit_dif[0] = lit_dif[1] = lit_dif[2] = 0.7f;
            lit_spc[0] = lit_spc[1] = lit_spc[2] = 0.0f;
            lit_pos[0] = lit_pos[1] = lit_pos[2] = 2.0f;
			for(int i=0;i<sizeof data/sizeof data_t;i++)
			{
				_stprintf(szText,TEXT("%g"),*(data[i].fOutValue));
				SetDlgItemText(hWnd,data[i].nEditID,szText);
				if(
					data[i].nSliderID==IDC_SLIDER_POSITION_X||
					data[i].nSliderID==IDC_SLIDER_POSITION_Y||
					data[i].nSliderID==IDC_SLIDER_POSITION_Z
					)
				{
					SendDlgItemMessage(
						hWnd,data[i].nSliderID,TBM_SETPOS,1,
						(INT)((*(data[i].fOutValue)+2.0f)*25.0f));
				}
				else
				{
					SendDlgItemMessage(
						hWnd,data[i].nSliderID,TBM_SETPOS,1,
						(INT)(*(data[i].fOutValue)*100.0f));
				}
			}
			InvalidateRect(GetParent(hWnd),0,0);
		}
		if(HIWORD(wParam)==EN_CHANGE)
		{
			data_t* p=(data_t*)GetWindowLong((HWND)lParam,GWL_USERDATA);
			GetWindowText((HWND)lParam,szText,256);
			*(p->fOutValue)=_tcstod(szText,0);
			if(
				p->nSliderID==IDC_SLIDER_POSITION_X||
				p->nSliderID==IDC_SLIDER_POSITION_Y||
				p->nSliderID==IDC_SLIDER_POSITION_Z
				)
			{
				SendDlgItemMessage(
					hWnd,p->nSliderID,TBM_SETPOS,1,
					(INT)((*(p->fOutValue)+2.0f)*25.0f));
			}
			else
			{
				SendDlgItemMessage(
					hWnd,p->nSliderID,TBM_SETPOS,1,
					(INT)(*(p->fOutValue)*100.0f));
			}
			InvalidateRect(GetParent(hWnd),0,0);
		}
		break;
	case WM_INITDIALOG:
		{
			for(int i=0;i<sizeof data/sizeof data_t;i++)
			{
				SetWindowLong(
					GetDlgItem(hWnd,data[i].nEditID),
					GWL_USERDATA,(long)&data[i]);
				SetWindowLong(
					GetDlgItem(hWnd,data[i].nSliderID),
					GWL_USERDATA,(long)&data[i]);
			}
			PostMessage(hWnd,WM_COMMAND,IDC_BUTTON_DEFAULT,0);
		}
		return 1;
	case WM_CLOSE:
		DestroyWindow(hLightDialogWnd);
		hLightDialogWnd=0;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK MaterialDialogProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static const data_t data[] = 
	{{IDC_EDIT_AMBIENT_R,IDC_SLIDER_AMBIENT_R,&mat_amb[0]},
	{IDC_EDIT_AMBIENT_G,IDC_SLIDER_AMBIENT_G,&mat_amb[1]},
	{IDC_EDIT_AMBIENT_B,IDC_SLIDER_AMBIENT_B,&mat_amb[2]},
	{IDC_EDIT_DIFFUSE_R,IDC_SLIDER_DIFFUSE_R,&mat_dif[0]},
	{IDC_EDIT_DIFFUSE_G,IDC_SLIDER_DIFFUSE_G,&mat_dif[1]},
	{IDC_EDIT_DIFFUSE_B,IDC_SLIDER_DIFFUSE_B,&mat_dif[2]},
	{IDC_EDIT_SPECULAR_R,IDC_SLIDER_SPECULAR_R,&mat_spc[0]},
	{IDC_EDIT_SPECULAR_G,IDC_SLIDER_SPECULAR_G,&mat_spc[1]},
	{IDC_EDIT_SPECULAR_B,IDC_SLIDER_SPECULAR_B,&mat_spc[2]},
	{IDC_EDIT_EMISSION_R,IDC_SLIDER_EMISSION_R,&mat_emi[0]},
	{IDC_EDIT_EMISSION_G,IDC_SLIDER_EMISSION_G,&mat_emi[1]},
	{IDC_EDIT_EMISSION_B,IDC_SLIDER_EMISSION_B,&mat_emi[2]},
	{IDC_EDIT_SHININESS,IDC_SLIDER_SHININESS,&mat_shi[0]}};
	static TCHAR szText[256];
	switch(msg)
	{
	case WM_HSCROLL:
		{
			data_t* p=(data_t*)GetWindowLong((HWND)lParam,GWL_USERDATA);
			if(p->nSliderID==IDC_SLIDER_SHININESS)
			{
				*(p->fOutValue)=SendMessage(
					(HWND)lParam,TBM_GETPOS,NULL,NULL)/100.0f*128.0f;
			}
			else
			{
				*(p->fOutValue)=SendMessage(
					(HWND)lParam,TBM_GETPOS,NULL,NULL)/100.0f;
			}
			_stprintf(szText,TEXT("%.2f"),*(p->fOutValue));
			SetDlgItemText(hWnd,p->nEditID,szText);
		}
		InvalidateRect(GetParent(hWnd),0,0);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_BUTTON_DEFAULT)
		{
            mat_amb[0] = 0.5f;
            mat_amb[1] = 0.471098f;
            mat_amb[2] = 0.367052f;
            mat_dif[0] = 0.678431f; // 現在の値の取得
            mat_dif[1] = 0.639216f; // 現在の値の取得
            mat_dif[2] = 0.498039f; // 現在の値の取得
            mat_spc[0] = 0.0f; // 現在の値の取得
            mat_spc[1] = 0.0f; // 現在の値の取得
            mat_spc[2] = 0.0f; // 現在の値の取得
            mat_emi[0] = 0.0f;
            mat_emi[1] = 0.0f;
            mat_emi[2] = 0.0f;
            mat_shi[0] = 0.5f*128.0f;
			for(int i=0;i<sizeof data/sizeof data_t;i++)
			{
				_stprintf(szText,TEXT("%g"),*(data[i].fOutValue));
				SetDlgItemText(hWnd,data[i].nEditID,szText);

				if(data[i].nSliderID==IDC_SLIDER_SHININESS)
				{
					SendDlgItemMessage(
						hWnd,data[i].nSliderID,TBM_SETPOS,1,
						(INT)(*(data[i].fOutValue)/128.0f*100.0f));
				}
				else
				{
					SendDlgItemMessage(
						hWnd,data[i].nSliderID,TBM_SETPOS,1,
						(INT)(*(data[i].fOutValue)*100.0f));
				}
			}
			InvalidateRect(GetParent(hWnd),0,0);
		}
		if(HIWORD(wParam)==EN_CHANGE)
		{
			data_t* p=(data_t*)GetWindowLong((HWND)lParam,GWL_USERDATA);
			GetWindowText((HWND)lParam,szText,256);
			*(p->fOutValue)=_tcstod(szText,0);
			if(p->nSliderID==IDC_SLIDER_SHININESS)
			{
				SendDlgItemMessage(
					hWnd,p->nSliderID,TBM_SETPOS,1,
					(INT)(*(p->fOutValue)/128.0f*100.0f));
			}
			else
			{
				SendDlgItemMessage(
					hWnd,p->nSliderID,TBM_SETPOS,1,
					(INT)(*(p->fOutValue)*100.0f));
			}
			InvalidateRect(GetParent(hWnd),0,0);
		}
		break;
	case WM_INITDIALOG:
		{
			for(int i=0;i<sizeof data/sizeof data_t;i++)
			{
				SetWindowLong(
					GetDlgItem(hWnd,data[i].nEditID),
					GWL_USERDATA,(long)&data[i]);
				SetWindowLong(
					GetDlgItem(hWnd,data[i].nSliderID),
					GWL_USERDATA,(long)&data[i]);
			}
			PostMessage(hWnd,WM_COMMAND,IDC_BUTTON_DEFAULT,0);
		}
		return 1;
	case WM_CLOSE:
		DestroyWindow(hMaterialDialogWnd);
		hMaterialDialogWnd=0;
		return TRUE;
	}
	return FALSE;
}

BOOL InitGL(GLvoid)
{
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	return TRUE;
}

VOID DrawGLScene()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0f,WINDOW_ASPECT,2.0f,10.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glTranslatef(0.0f,0.0f,-3.0f);
	glRotatef(rot[0],1.0f,0.0f,0.0f);
	glRotatef(rot[1],0.0f,1.0f,0.0f);

	glLightfv(GL_LIGHT0,GL_AMBIENT,lit_amb);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,lit_dif);
	glLightfv(GL_LIGHT0,GL_SPECULAR,lit_spc);
	glLightfv(GL_LIGHT0,GL_POSITION,lit_pos);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	/* 質感を設定 */
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,mat_amb);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,mat_dif);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,mat_spc);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,mat_emi);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,mat_shi);

	glutSolidTeapot(0.6);
	
	glFlush();
}

void rotate(int ox,int nx,int oy,int ny)
{
	int dx=ox-nx;
	int dy=ny-oy;
	rot[0]+=(dy*180.0f)/500.0f;
	rot[1]-=(dx*180.0f)/500.0f;
#define clamp(x) x=x>360.0f?x-360.0f:x<-360.0f?x+=360.0f:x
	clamp(rot[0]);
	clamp(rot[1]);
#undef clamp
}

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static PIXELFORMATDESCRIPTOR pfd={
		sizeof(PIXELFORMATDESCRIPTOR),1,PFD_DRAW_TO_WINDOW|
			PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,PFD_TYPE_RGBA,
			32,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,PFD_MAIN_PLANE,0,0,0,0};
	GLuint PixelFormat;
	static BOOL bCapture=0;
	static int oldx,oldy,x,y;
	switch(msg)
	{
	case WM_CREATE:
		InitCommonControls();

		if(!(hDC=GetDC(hWnd)))
		{
			MessageBox(hWnd,
				TEXT("Can't Create A GL Device Context."),
				0,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}
		if(!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
		{
			MessageBox(hWnd,
				TEXT("Can't Find A Suitable PixelFormat."),
				0,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}
		if(!SetPixelFormat(hDC,PixelFormat,&pfd))
		{
			MessageBox(hWnd,
				TEXT("Can't Set The PixelFormat."),
				0,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}
		if(!(hRC=wglCreateContext(hDC)))
		{
			MessageBox(hWnd,
				TEXT("Can't Create A GL Rendering Context."),
				0,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}
		if(!wglMakeCurrent(hDC,hRC))
		{
			MessageBox(hWnd,
				TEXT("Can't Activate The GL Rendering Context."),
				0,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}
		if(!InitGL())
		{
			MessageBox(hWnd,
				TEXT("Initialization Failed."),
				0,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}
		hLightDialogWnd=CreateDialog(
			((LPCREATESTRUCT)lParam)->hInstance,
			MAKEINTRESOURCE(IDD_LIGHT_DIALOG),hWnd,LightDialogProc
		);
		hMaterialDialogWnd=CreateDialog(
			((LPCREATESTRUCT)lParam)->hInstance,
			MAKEINTRESOURCE(IDD_MATERIAL_DIALOG),hWnd,MaterialDialogProc
		);
		break;
	case WM_MOVE:
		{
			RECT rect1,rect2;
			GetWindowRect(hWnd,&rect1);
			SetWindowPos(
				hLightDialogWnd,0,rect1.right,rect1.top,0,0,
				SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			GetWindowRect(hLightDialogWnd,&rect2);
			SetWindowPos(
				hMaterialDialogWnd,0,rect2.right,rect2.top,0,0,
				SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		x=LOWORD(lParam);
		y=HIWORD(lParam);
		bCapture=1;
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		bCapture=0;
		break;
	case WM_MOUSEMOVE:
		if(bCapture)
		{
			oldx=x;
			oldy=y;
			x=LOWORD(lParam);
			y=HIWORD(lParam);
			if(x&1<<15)x-=(1<<16);
			if(y&1<<15)y-=(1<<16);
			rotate(oldx,x,oldy,y);
			InvalidateRect(hWnd,0,0);
		}
		break;
	case WM_PAINT:
		DrawGLScene();
		SwapBuffers(hDC);
		ValidateRect(hWnd,NULL);
		break;
	case WM_DESTROY:
		if(hRC)
		{
			if(!wglMakeCurrent(NULL,NULL))
			{
				MessageBox(hWnd,
					TEXT("Release Of DC And RC Failed."),
					0,MB_OK|MB_ICONINFORMATION);
			}
			if(!wglDeleteContext(hRC))
			{
				MessageBox(hWnd,
					TEXT("Release Rendering Context Failed."),
					0,MB_OK|MB_ICONINFORMATION);
			}
		}
		if(hDC)
		{
			ReleaseDC(hWnd,hDC);
		}
		PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPreInst,
				   LPSTR pCmdLine,int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass={0,WndProc,0,0,hInstance,0,
		LoadCursor(0,IDC_ARROW),0,0,szClassName};
	RegisterClass(&wndclass);
	RECT rect={0,0,WINDOW_WIDTH,WINDOW_HEIGHT};
	AdjustWindowRect(&rect,
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|
		WS_CLIPSIBLINGS|WS_CLIPCHILDREN,FALSE);
	HWND hWnd=CreateWindow(
		szClassName,TEXT("OpenGL Lighting Test"),
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_CLIPSIBLINGS|
		WS_CLIPCHILDREN,CW_USEDEFAULT,0,rect.right-rect.left,
		rect.bottom-rect.top,0,0,hInstance,0);
    ShowWindow(hWnd,nCmdShow);
    UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		if(hLightDialogWnd&&
			IsDialogMessage(hLightDialogWnd,&msg))continue;
		if(hMaterialDialogWnd&&
			IsDialogMessage(hMaterialDialogWnd,&msg))continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    return msg.wParam;
}