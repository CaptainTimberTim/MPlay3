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
CreateGLTexture(loaded_bitmap Bitmap, b32 DoSameColorFormat = false)
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
        
        case colorFormat_Alpha:
        {
            BitmapColorFormat = GL_ALPHA;
        } break;
        
        InvalidDefaultCase
    }
    
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    if(DoSameColorFormat) glTexImage2D(GL_TEXTURE_2D, 0, BitmapColorFormat, Bitmap.Width, Bitmap.Height, 0, BitmapColorFormat, 
                                       GL_UNSIGNED_BYTE, Bitmap.Pixels);
    else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap.Width, Bitmap.Height, 0, BitmapColorFormat, 
                      GL_UNSIGNED_BYTE, Bitmap.Pixels);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    return ++ID; // ZII:: We increment texIDs on creation to have 0 be default
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

inline u32
DecodeAndCreateGLTexture(arena_allocator *Arena, loaded_bitmap Bitmap)
{
    loaded_bitmap IconBitmap = DecodeIcon(Arena, Bitmap.Width, Bitmap.Height, 
                                          (u8 *)Bitmap.Pixels, Bitmap.Pitch); // Abusing pitch for memory size
    u32 Result = CreateGLTexture(IconBitmap);
    FreeMemory(Arena, IconBitmap.Pixels);
    
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
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

inline void
DecodeAndUpdateGLTexture(arena_allocator *Arena, loaded_bitmap Bitmap, GLuint TexID)
{
    loaded_bitmap IconBitmap = DecodeIcon(Arena, Bitmap.Width, Bitmap.Height, 
                                          (u8 *)Bitmap.Pixels, Bitmap.Pitch); // Abusing pitch for memory size
    UpdateGLTexture(IconBitmap, TexID);
    FreeMemory(Arena, IconBitmap.Pixels);
}

inline void
DeleteGLTexture(u32 GLID)
{
    GLID -= 1;
    glDeleteTextures(1, &GLID);
}

inline void
ConvertToGLSpace(window_info WindowDims, render_entry *Entry, v3 *Result)
{
    v2 CurD = V2(WindowDims.CurrentDim.Dim);
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

internal void
DisplayBufferInWindow(HDC DeviceContext, window_info Window, render_entry_list *EntryList, v4 BGColor)
{
    glClearColor(BGColor.r, BGColor.g, BGColor.b, BGColor.a);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    UpdateEntryList(EntryList);
    
    for(u32 EntryID = 0; EntryID < EntryList->EntryCount; EntryID++)
    {
        render_entry *RenderEntry = EntryList->Entries + EntryID;
        if(!RenderEntry->Render) continue;
        
        if(RenderEntry->Scissor != NULL)
        {
            glEnable(GL_SCISSOR_TEST);
            v3 V[4];
            ApplyTransform(RenderEntry->Scissor->ID, V);
            glScissor((i32)V[0].x, (i32)V[0].y, (i32)(V[2].x - V[0].x), (i32)(V[2].y - V[0].y));
        }
        
        switch(RenderEntry->Type)
        {
            case renderType_2DBitmap: 
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, RenderEntry->TexID-1); // ZII:: We increment texIDs on creation to have 0 be default
                
                glBegin(GL_QUADS);
                v3 V[4];
                ConvertToGLSpace(Window, RenderEntry, V);
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
                ConvertToGLSpace(Window, RenderEntry, V);
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
                glBegin(GL_QUADS);
                
                u32 GLID = 0;
                For(RenderEntry->Text->Count)
                {
                    render_entry *TextEntry = RenderEntry->Text->RenderEntries+It;
                    if(!TextEntry->Render) continue;
                    
                    Assert(TextEntry->TexID != 0);
                    if(GLID != TextEntry->TexID) // Only swtich Texture if actually needed.
                    {
                        GLID = TextEntry->TexID;
                        glEnd();
                        glBindTexture(GL_TEXTURE_2D, TextEntry->TexID-1);
                        glBegin(GL_QUADS);
                    }
                    
                    v3 V[4];
                    ConvertToGLSpace(Window, TextEntry, V);
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
        if(RenderEntry->Scissor != NULL)
            glDisable(GL_SCISSOR_TEST);
    }
}
