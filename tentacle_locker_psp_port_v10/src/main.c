#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

PSP_MODULE_INFO("Tentacle Locker PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define BUF_W 512
#define SCR_W 480
#define SCR_H 272
#define GAME_SECONDS 45
#define FPS 60

typedef struct { float u,v,x,y,z; } Vertex;
typedef struct { int w,h,tw,th; unsigned char *pixels; } Texture;
typedef enum { TITLE, PLAY, GAMEOVER } State;

static unsigned int __attribute__((aligned(16))) list[262144];
static int running=1;
static State state=TITLE;
static int score=0, frame=0, side=0, character=0, closetOpen=0;
static int actionFrames=0;
static int animFrame=0;

static Texture bg={0}, closet={0}, door={0}, mainmenu={0}, openbtn={0}, leftbtn={0}, rightbtn={0};
static Texture chars[3][3];
static Texture windowFrames[3];

static int cb_exit(int a,int b,void*c){(void)a;(void)b;(void)c;running=0;return 0;}
static int cb_thread(SceSize a,void*b){(void)a;(void)b;int id=sceKernelCreateCallback("Exit",cb_exit,NULL);if(id>=0)sceKernelRegisterExitCallback(id);sceKernelSleepThreadCB();return 0;}
static void setup_callbacks(void){int t=sceKernelCreateThread("cb",cb_thread,0x11,0xFA0,0,NULL);if(t>=0)sceKernelStartThread(t,0,NULL);}

static int load_raw(Texture*t,const char*path,int w,int h,int tw,int th){
    FILE*f=fopen(path,"rb"); if(!f)return -1;
    int sz=tw*th*4; t->pixels=(unsigned char*)memalign(16,sz);
    if(!t->pixels){fclose(f);return -2;}
    if(fread(t->pixels,1,sz,f)!=(size_t)sz){fclose(f);free(t->pixels);t->pixels=NULL;return -3;}
    fclose(f); t->w=w;t->h=h;t->tw=tw;t->th=th;
    sceKernelDcacheWritebackRange(t->pixels,sz); return 0;
}
static void free_tex(Texture*t){if(t->pixels){free(t->pixels);t->pixels=NULL;}}

static void begin_frame(void){
    sceGuStart(GU_DIRECT,list);
    sceGuClearColor(0xFF202030); sceGuClear(GU_COLOR_BUFFER_BIT);
    sceGuDisable(GU_DEPTH_TEST); sceGuEnable(GU_TEXTURE_2D); sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
    sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA); sceGuTexFilter(GU_LINEAR,GU_LINEAR);
}
static void end_frame(void){sceGuFinish();sceGuSync(0,0);sceDisplayWaitVblankStart();sceGuSwapBuffers();}

static void draw_tex(Texture*t,float x,float y,float scale){
    if(!t->pixels)return;
    sceGuTexMode(GU_PSM_8888,0,0,0);
    int lw=0,lh=0;while((1<<lw)<t->tw)lw++;while((1<<lh)<t->th)lh++;
    sceGuTexImage(0,t->tw,t->th,t->tw,t->pixels);
    Vertex*v=(Vertex*)sceGuGetMemory(2*sizeof(Vertex));
    v[0]=(Vertex){0,0,x,y,0}; v[1]=(Vertex){(float)t->w,(float)t->h,x+t->w*scale,y+t->h*scale,0};
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,v);
}
static void rect(unsigned int color,float x,float y,float w,float h){
    sceGuDisable(GU_TEXTURE_2D); sceGuColor(color);
    Vertex*v=(Vertex*)sceGuGetMemory(2*sizeof(Vertex));
    v[0]=(Vertex){0,0,x,y,0};v[1]=(Vertex){0,0,x+w,y+h,0};
    sceGuDrawArray(GU_SPRITES,GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,v);sceGuEnable(GU_TEXTURE_2D);
}

static void start_game(void){state=PLAY;score=0;frame=0;side=0;character=0;closetOpen=0;actionFrames=0;animFrame=0;}
static void next_character(int dir){character=(character+dir+3)%3;animFrame=0;}

static void update(SceCtrlData*p){
    static unsigned last=0; unsigned now=p->Buttons, press=now&~last; last=now;
    if(press&PSP_CTRL_START){running=0;return;}
    if(state==TITLE){if(press&PSP_CTRL_CROSS)start_game();return;}
    if(state==GAMEOVER){if(press&PSP_CTRL_CROSS)start_game();return;}
    if(press&PSP_CTRL_LEFT){side=0;next_character(-1);}
    if(press&PSP_CTRL_RIGHT){side=1;next_character(1);}
    if(press&PSP_CTRL_CROSS)closetOpen=!closetOpen;
    if(press&PSP_CTRL_CIRCLE && !closetOpen){score++;actionFrames=FPS;}
    frame++; if(actionFrames>0)actionFrames--;
    if((frame%10)==0)animFrame=(animFrame+1)%3;
    if(frame>=GAME_SECONDS*FPS)state=GAMEOVER;
}
static void draw_hud(void){
    float remain=1.0f-(float)frame/(float)(GAME_SECONDS*FPS);if(remain<0)remain=0;
    rect(0xAA000000,8,7,464,12);rect(0xFF70D6FF,10,9,460*remain,8);
    int blocks=score>20?20:score;for(int i=0;i<blocks;i++)rect(0xFFFFD36A,8+i*12,25,9,7);
}

int main(int argc,char**argv){
    (void)argc;(void)argv;setup_callbacks();
    sceCtrlSetSamplingCycle(0);sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
    sceGuInit();sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,(void*)0,BUF_W);sceGuDispBuffer(SCR_W,SCR_H,(void*)0x88000,BUF_W);
    sceGuDepthBuffer((void*)0x110000,BUF_W);sceGuOffset(2048-(SCR_W/2),2048-(SCR_H/2));
    sceGuViewport(2048,2048,SCR_W,SCR_H);sceGuDepthRange(65535,0);sceGuScissor(0,0,SCR_W,SCR_H);sceGuEnable(GU_SCISSOR_TEST);
    sceGuFinish();sceGuSync(0,0);sceDisplayWaitVblankStart();sceGuDisplay(GU_TRUE);

    load_raw(&bg,"assets/psp_textures/102_background.rgba",480,272,512,512);
    load_raw(&closet,"assets/psp_textures/16_closet_background.rgba",126,319,128,512);
    load_raw(&door,"assets/psp_textures/20_closet_Door.rgba",126,319,128,512);
    load_raw(&mainmenu,"assets/psp_textures/103_mainmenu.rgba",431,514,512,1024);
    load_raw(&openbtn,"assets/psp_textures/28_OpenbtnUp.rgba",163,72,256,128);
    load_raw(&leftbtn,"assets/psp_textures/38_leftbtn_Up.rgba",61,57,64,64);
    load_raw(&rightbtn,"assets/psp_textures/10_Rightclosetbtn.rgba",34,26,64,32);
    for(int c=0;c<3;c++)for(int a=0;a<3;a++){
        char path[96];sprintf(path,"assets/psp_textures/char%d_anim%d.rgba",c+1,a+1);
        load_raw(&chars[c][a],path,181,160,256,256);
    }
    load_raw(&windowFrames[0],"assets/psp_textures/106_fucking_window_02.rgba",430,553,512,1024);
    load_raw(&windowFrames[1],"assets/psp_textures/45_fucking_window_anim02.rgba",430,553,512,1024);
    load_raw(&windowFrames[2],"assets/psp_textures/109_fucking_window_front.rgba",430,553,512,1024);

    while(running){
        SceCtrlData p;memset(&p,0,sizeof(p));sceCtrlReadBufferPositive(&p,1);update(&p);begin_frame();
        if(state==TITLE){
            rect(0xFFEF82A5,0,0,SCR_W,SCR_H);draw_tex(&mainmenu,24,8,0.50f);draw_tex(&openbtn,275,184,0.75f);
        }else if(state==PLAY){
            rect(0xFFFFA0C0,0,0,SCR_W,SCR_H);draw_tex(&bg,0,0,1.0f);
            float cx=side?260.0f:80.0f;draw_tex(&chars[character][animFrame],cx,45,1.05f);
            draw_tex(&closet,177,20,0.78f);draw_tex(&door,closetOpen?270.0f:177.0f,20,0.78f);
            draw_tex(&leftbtn,12,205,0.85f);draw_tex(&rightbtn,420,205,0.85f);draw_tex(&openbtn,190,224,0.62f);
            if(actionFrames>0){int wf=(actionFrames<20)?2:((actionFrames<40)?1:0);draw_tex(&windowFrames[wf],115,5,0.25f);}
            draw_hud();
        }else{
            rect(0xFF7C4C70,0,0,SCR_W,SCR_H);draw_tex(&mainmenu,24,8,0.50f);
            rect(0xCC000000,105,225,270,30);rect(0xFFFFD36A,115,235,(score>20?20:score)*12,10);
        }
        end_frame();
    }
    free_tex(&bg);free_tex(&closet);free_tex(&door);free_tex(&mainmenu);free_tex(&openbtn);free_tex(&leftbtn);free_tex(&rightbtn);
    for(int c=0;c<3;c++)for(int a=0;a<3;a++)free_tex(&chars[c][a]);for(int i=0;i<3;i++)free_tex(&windowFrames[i]);
    sceGuTerm();sceKernelExitGame();return 0;
}
