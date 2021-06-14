//===========================================================================
//
//  ��Ȩ�����ߣ� ���¹����㽭��ѧ�������ѧ�뼼��ѧԺ
//                       CAD&CG�����ص�ʵ����
//               xgliu@cad.zju.edu.cn
//  ����޸ģ�2019��2��28�� 
//            ����˿ؼ�����ɫ���������ã�
//            �Լ�������ɫ�����ӣ��ں���demoGuiALL.c��drawButtons�����
//  ����޸ģ�2019��2��26�� 
//            �������ʾ�ı��༭��ʾ
//            ����˶�����ʾ
//            �����textbox �ı�����ؼ�
//            ���˲˵�����
//            �� uiGetInput Ϊ uiGetMouse,uiGetKey,uiGetKeyboard
//  ����޸ģ�2019��2��18��
//  ���δ�����2018��4�£�����<<�������ר��>>�γ̽�ѧ
//
//===========================================================================

#ifndef ____ui_button_h______
#define ____ui_button_h______

#include <stdint.h> // import uintptr_t and uint64_t since C99
#include <stdbool.h>

/* �ؼ���ɫ */
typedef struct ItemColor {
	char frame[32];
	char label[32];
	char hoverFrame[32];
	char hoverLabel[32];
	bool  fill;
} ItemColor;

enum {
    GUI_NONE  = 0,  // û���κ��¼�
    GUI_CLICK = 1,  // ����ڿؼ��ؼ����
    GUI_HOVER = 2,  // ��괦���ڿؼ���
    GUI_PRESS = 3,  // ����ڿؼ��ϰ�ť����
    GUI_UP    = 4,  // ����ڿؼ��ϰ�ţ̌��
    GUI_FOCUS = 5,  // �ؼ��۽�
    GUI_TYPE  = 6,  // �ı�������
    GUI_ACCEPT = 7, // �ı����������
};


//===========================================================================
//
//   [L:16-1][N:16-1]
//        XOR
//   [F:32 --------1]
// 
// Generate a *fake* unique ID for gui controls at compiling/run time
//
#define GenUUID(N) (((__LINE__<<16) | (N&0xFFFF)) \
                    ^ ((uintptr_t)&__FILE__) \
                    ^ ((uint64_t)(uint64_t*)__DATE__) \
                    ^ ((uint64_t)(uint64_t*)__TIME__))
//
// GenUUID(0) will give a unique ID at each source code line. 
// If you need one UI ID per line, just call GenUUID with 0
//
//               GenUUID(0)
//
// But, two GenUUID(0) calls at the same line will give the same ID.
//
// So, in a while/for loop body, GenUUID(0) will give you the same ID.
// In this case, you need call GenUUID with different N parameter: 
//
//               GenUUID(N)
//
//===========================================================================

/* ��������	initGUI
 *
 * ���ܣ���ʼ������
 *
 * �÷����ڴ��ڴ�������������֮�����
 */
void initGUI();

/* ��������	uiGetMouse
 *			uiGetKeyboard
 *			uiGetChar
 *
 * ���ܣ�����û������/����/�ı�������
 *
 * �÷��������/����/�ı�����Ļص�������ʹ�� 
 *
 *		void MouseEventProcess(int x, int y, int button, int event)
 *		{
 *			uiGetMouse(x,y,button,event); 
 *			...��������...
 *		}
 *
 *		void KeyboardEventProcess(int key, int event)
 *		{
 *			uiGetKeyboard(key,event); 
 *			...��������...
 *		}
 *
 *		void CharEventProcess(char ch)
 *		{
 *			uiGetChar(ch); 
 *			...��������...
 *		}
 */
void uiGetMouse(int x, int y, int button, int event);
void uiGetKeyboard(int key, int event);
void uiGetChar(int ch);


/* 
 * ��������button
 *
 * ���ܣ���ʾһ����ť��button��
 *
 * �÷���if( button(GenUUID(0),x,y,w,h,label) ) {
 *           ִ����䣬��Ӧ�û����¸ð�ť
 *       }
 *
 *   id  - Ψһ��
 *   x,y - λ��
 *   w,h - ��Ⱥ͸߶�
 *   label - ��ť�ϵ����ֱ�ǩ
 *
 * ����ֵ
 *   0 - �û�û�е�������²��ͷţ���ť  
 *   1 - ����˰�ť
 */
int button(int id, double x, double y, double w, double h, char *label);


/* 
 * ��������menuList
 *
 * ���ܣ���ʾһ��˵�
 *
 * �÷���choice = menuList(GenUUID(0),x,y,w,h,labels,n);
 *
 *   id  - �˵���Ψһ��
 *   x,y - �˵���λ��
 *   w,h - �˵���Ĵ�С
 *   wlist - �˵��б�Ŀ��
 *   labels[] - ��ǩ���֣�[0]�ǲ˵����[1...n-1]�ǲ˵��б�
 *   n   - �˵���ĸ���
 *
 * ����ֵ
 *   -1    - �û�û�е�������²��ͷţ���ť  
 *   >=0   - �û�ѡ�еĲ˵��� index ����labels[]�У�
 *
 * Ӧ�þ�����

	char * menuListFile[] = {"File",  
		"Open  | Ctrl-O",  // ��ݼ��������[Ctrl-X]��ʽ�������ַ����Ľ�β
		"Close",           
		"Exit   | Ctrl-E"};// ��ݼ��������[Ctrl-X]��ʽ�������ַ����Ľ�β

	int selection;

	selection = menuList(GenUUID(0), x, y, w, wlist, h, menuListFile, sizeof(menuListFile)/sizeof(menuListFile[0]));

	if( selection==3 )
		exit(-1); // user choose to exit

 */
int  menuList(int id, double x, double y, double w, double wlist, double h, char *labels[], int n);
// �ò˵�����ɫ����һ�����Σ�λ��(x,y),���(w,h)
// һ�� w ���ڴ��ڵĿ�ȣ�h �Ͳ˵���ĸ߶�ƥ��
void drawMenuBar(double x, double y, double w, double h); 

/* 
 * ��������textbox
 *
 * ���ܣ���ʾһ���ı��༭����ʾ�ͱ༭�ı��ַ���
 *
 * �÷���textbox(GenUUID(0),x,y,w,h,textbuf,buflen);
 *       ����
         if( textbox(GenUUID(0),x,y,w,h,textbuf,buflen) ) {
 *           �ı��ַ������༭�޸��ˣ�ִ����Ӧ���
 *       }
 *
 *   id  - Ψһ�ţ�һ����GenUUID(0), ����GenUUID��k)��k��ѭ��������
 *   x,y - �ı���λ��
 *   w,h - �ı���Ŀ�Ⱥ͸߶�
 *   textbuf - ���༭���ı��ַ�������\0��β��
 *   buflen - �ɴ洢���ı��ַ�������󳤶�
 * ����ֵ
 *   0 - �ı�û�б��༭
 *   1 - ���༭��
 *   2 - ���˻س����ı���ȷ��
 */
int textbox(int id, double x, double y, double w, double h, char textbuf[], int buflen);

/*
 * ���ÿؼ�����ɫ
 *  
 * �����͹���
 *      setButtonColors   - ���ð�ť��ɫ
 *      setMenuColors     - ���ò˵���ɫ
 *      setTextBoxColors  - ���ñ༭����ɫ    
 * ����
 *      frame/label       - �ؼ���/���ֱ�ǩ����ɫ
 *      hotFrame/hotLabel - ��껮��ʱ���ؼ���/���ֱ�ǩ����ɫ
 *      fillflag          - �Ƿ���䱳����0 - ����䣬1 - ���
 * ˵��
 *      ��ĳ����ɫ�ַ���Ϊ��ʱ����Ӧ����ɫ��������
 */
void setDefaultColor(char *frame, char *label, char *hoverFrame, char *hoverLabel, bool fill);
void setButtonColor(char *frame, char *label, char *hoverFrame, char *hoverLabel, bool fill);
void setMenuColor(char *frame, char *label, char *hoverFrame, char *hoverLabel, bool fill);
void setTextBoxColor(char *frame, char *label, char *hoverFrame, char *hoverLabel, bool fill);

ItemColor *getDefaultColor(void);
ItemColor *getMenuColor(void);
ItemColor *getTextBoxColor(void);

// ʹ��Ԥ�������ɫ���
void usePredefinedColor(int k);
void usePredefinedDefaultColor(int k);
void usePredefinedButtonColor(int k);
void usePredefinedMenuColor(int k);
void usePredefinedTexBoxColor(int k);

/* ��ֱ�� */
void drawLine(double x1, double y1, double x2, double y2);
void drawLineColor(double x1, double y1, double x2, double y2, char *color);

/* ��ʾ�ַ�����ǩ */
void drawLabel(double x, double y, char *label);
void drawLabelColor(double x, double y, char *label, char *color);

/* ��һ������ */
void drawRectangle(double x, double y, double w, double h, bool fill);
void drawRectangleColor(double x, double y, double w, double h, bool fill, char *color);

/* ��ʾ���ַ�����ǩ�ľ���
 * 
 * xalign - ��ǩ�;��εĶ��뷽ʽ
 *              'L' - ����
 *			    'R' - ����
 *              ����- ����
 *
 * fill   - 1 ���
 *          0 �����
 */
void drawBox(double x, double y, double w, double h, char *label,
        char xalign, bool fill);
void drawBoxColor(double x, double y, double w, double h, char *label,
        char xalign, bool fill,
        char *frameColor, char *labelColor);

#endif // define ____ui_button_h______
