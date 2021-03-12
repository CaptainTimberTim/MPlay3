#include "GL_TD.h"
#include "Renderer_TD.h"
#include "GameBasics_TD.h"
// NOTE:: The windows guy is called: Raymond Chen
// See his blog post on fullscreen stuff!
#define glMultMatrixf(m) Assert(false);

internal void 
InitOpenGL(HWND Window, b32 DoBackfaceCulling = true)
{
    HDC WindowDC = GetDC(Window);
    
    // Describing what pixel format we wanna use
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    // Ask for the best fitting pixel format
    i32 SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    // Get descriptor for the pixel format we got back
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    // Set the pixel format we got with all its descriptions
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    if(wglMakeCurrent(WindowDC, OpenGLRC))
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH);
        glDepthFunc(GL_LESS);
        if(DoBackfaceCulling)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW); // GL_CCW for counter clock-wise
        }
    }
    else InvalidCodePath; // Should never really happen
    
    ReleaseDC(Window, WindowDC);
}

internal void 
DrawGroundGrid()
{
    r32 GridSize = 10.0f;
    u32 GridSegments = 20;
    r32 GridStep = GridSize / (r32)GridSegments;
    r32 GridOrigin = -GridSize*0.5f;
    
	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 1.0f);
	For (GridSegments + 1)
	{
		r32 itpos = GridOrigin + GridStep*(r32)It;
		glVertex3f(itpos, -1.0f, GridOrigin);
		glVertex3f(itpos, -1.0f, GridOrigin + GridSize);
        
		glVertex3f(GridOrigin, -1.0f, itpos);
		glVertex3f(GridOrigin + GridSize, -1.0f, itpos);
        
	}
	glEnd();
}

inline void
GLMatrixMultiply(m4 M)
{
    Transpose(&M);
#undef glMultMatrixf
    glMultMatrixf(M.E);
#define glMultMatrixf(m) Assert(false); 
}

inline void 
ReshapeGLWindow(renderer *Renderer)
{
    glViewport(0, 0, Renderer->Window.CurrentDim.Width, Renderer->Window.CurrentDim.Height);
    
    /*
    camera *Camera = &Renderer->Camera;
    Camera->Projection = Identity();
    Perspective(&Camera->Projection, Camera->FOVY, Renderer->Window.CurrentAspect, 
                Camera->ClippingPlanes.x, Camera->ClippingPlanes.y);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLMatrixMultiply(Renderer->Camera.Projection);
    
    glMatrixMode(GL_MODELVIEW);*/
}

inline u32
CreateGLTexture(loaded_bitmap Bitmap)
{
    u32 ID;
    
    Assert(Bitmap.WasLoaded);
    Assert(Bitmap.Pixels);
    
    u32 BitmapColorFormat = 0;
    switch(Bitmap.ColorFormat)
    {
        case colorFormat_RGB:
        {
            BitmapColorFormat = GL_RGB;
        } break;
        
        case colorFormat_RGBA:
        {
            BitmapColorFormat = GL_RGBA;
        } break;
        
        InvalidDefaultCase
    }
    
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap.Width, Bitmap.Height, 0, BitmapColorFormat, 
                 GL_UNSIGNED_BYTE, Bitmap.Pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    return ++ID;
}

inline u32
LoadAndCreateGLTexture(u8 *Path)
{
    loaded_bitmap NewTexBitmap = LoadImage_STB(Path);
    u32 Result = CreateGLTexture(NewTexBitmap);
    FreeImage_STB(NewTexBitmap);
    
    return Result;
}

inline u32
LoadAndCreateGLTexture(u8 *Path, u8 *Filename)
{
    NewLocalString(PathName, 260, Path);
    AppendStringToCompound(&PathName, Filename);
    loaded_bitmap NewTexBitmap = LoadImage_STB(PathName.S);
    u32 Result = CreateGLTexture(NewTexBitmap);
    FreeImage_STB(NewTexBitmap);
    
    return Result;
}

inline u32
DecodeAndCreateGLTexture(u32 DataSize, u8 *Data)
{
    loaded_bitmap NewTexBitmap = LoadImage_STB({DataSize, Data});
    u32 Result = CreateGLTexture(NewTexBitmap);
    FreeImage_STB(NewTexBitmap);
    
    return Result;
}

inline void
UpdateGLTexture(loaded_bitmap Bitmap, GLuint TexID)
{
    glBindTexture(GL_TEXTURE_2D, TexID-1);
    
    i32 GLColorFormat = 0;
    switch(Bitmap.ColorFormat)
    {
        case colorFormat_RGBA:
        {
            GLColorFormat = GL_RGBA;
        } break;
        case colorFormat_RGB:
        {
            GLColorFormat = GL_RGB;
        } break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap.Width, Bitmap.Height, 0, GLColorFormat, GL_UNSIGNED_BYTE, Bitmap.Pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

inline void
ConvertToGLSpace(renderer *Renderer, render_entry *Entry, v3 *Result)
{
    v2 CurD = V2(Renderer->Window.CurrentDim.Dim);
    ApplyTransform(Entry, Result);
    
    For(4)
    {
        // Into -1, 1 space of GL
        Result[It].x = Result[It].x/CurD.x;
        Result[It].x = (Result[It].x*2) - 1;
        Result[It].y = Result[It].y/CurD.y;
        Result[It].y = (Result[It].y*2) - 1;
    }
}

inline b32
Render(render_entry *Entry)
{
    b32 Result = Entry->Render;
    
    while(Entry->Parent && Result)
    {
        Entry = Entry->Parent->ID;
        Result = Entry->Render;
    }
    
    return Result;
}

internal void
DisplayBufferInWindow(HDC DeviceContext, renderer *Renderer)
{
    glClearColor(Renderer->BackgroundColor.r, Renderer->BackgroundColor.g, 
                 Renderer->BackgroundColor.b, Renderer->BackgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    render_entry_list *EntryList = &Renderer->RenderEntryList;
    UpdateEntryList(EntryList);
    
    for(u32 EntryID = 0; EntryID < EntryList->EntryCount; EntryID++)
    {
        render_entry *RenderEntry = EntryList->Entries + EntryID;
        //if(!Render(RenderEntry)) continue;
        if(!RenderEntry->Render) continue;
        
        switch(RenderEntry->Type)
        {
            case renderType_2DBitmap: 
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, RenderEntry->TexID-1); // ZII:: We increment texIDs on creation to have 0 be default
                
                glBegin(GL_QUADS);
                v3 V[4];
                ConvertToGLSpace(Renderer, RenderEntry, V);
                For(4)
                {
                    v2 T = RenderEntry->TexCoords[It];
                    
                    glTexCoord2f(T.x, T.y);
                    glColor4f(RenderEntry->Color->r, RenderEntry->Color->g, RenderEntry->Color->b, RenderEntry->Transparency);
                    glVertex3fv(V[It].E);
                }
                glEnd();
                glDisable(GL_TEXTURE_2D);
            } break;
            
            case renderType_2DRectangle:
            {
                glBegin(GL_QUADS);
                v3 V[4];
                ConvertToGLSpace(Renderer, RenderEntry, V);
                For(4)
                {
                    v2 T = RenderEntry->TexCoords[It];
                    
                    glTexCoord2f(T.x, T.y);
                    glColor4f(RenderEntry->Color->r, RenderEntry->Color->g, RenderEntry->Color->b, RenderEntry->Transparency);
                    glVertex3fv(V[It].E);
                }
                glEnd();
            } break;
            
            case renderType_Text:
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, RenderEntry->TexID-1);
                glBegin(GL_QUADS);
                
                For(RenderEntry->Text->Count)
                {
                    render_entry *TextEntry = RenderEntry->Text->RenderEntries+It;
                    if(!TextEntry->Render) continue;
                    v3 V[4];
                    ConvertToGLSpace(Renderer, TextEntry, V);
                    For(4)
                    {
                        v2 T = TextEntry->TexCoords[It];
                        
                        glTexCoord2f(T.x, T.y);
                        glColor4f(TextEntry->Color->r, TextEntry->Color->g, TextEntry->Color->b, TextEntry->Transparency);
                        glVertex3fv(V[It].E);
                    }
                }
                
                glEnd();
                glDisable(GL_TEXTURE_2D);
            } break;
            
            InvalidDefaultCase;
        }
    }
}
