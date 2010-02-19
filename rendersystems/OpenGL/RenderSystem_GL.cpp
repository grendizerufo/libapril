#include "RenderSystem_GL.h"
#ifdef IPHONE_PLATFORM
#include <OpenGLES/ES1/gl.h>
#else
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <IL/ilut.h>

namespace April
{
	unsigned int platformLoadGLTexture(const char* name,int* w,int* h)
	{
		unsigned int texid,image,success;
		ilGenImages(1, &texid);
		ilBindImage(texid);

		std::string filename=name;
		success = ilLoadImage(name);
		if (success)
		{
			success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		}
		else throw "can't load image";
		*w=ilGetInteger(IL_IMAGE_WIDTH);
		*h=ilGetInteger(IL_IMAGE_HEIGHT);
		glGenTextures(1, &image);
		glBindTexture(GL_TEXTURE_2D, image);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), *w,*h, 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,ilGetData());
		ilDeleteImages(1, &texid);


		return image;
	}
	
	void win_mat_invert()
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		float mat[16],t;
		glGetFloatv(GL_PROJECTION_MATRIX,mat);
		t=mat[1]; mat[1]=mat[0]; mat[0]=t;
		t=mat[5]; mat[5]=-mat[4]; mat[4]=t;
		mat[13]=-mat[13];
		glLoadMatrixf(mat);
	}
	#endif
	// translation from abstract render ops to gl's render ops
	int gl_render_ops[]=
	{
		0,
		GL_TRIANGLES,      // ROP_TRIANGLE_LIST
		GL_TRIANGLE_STRIP, // ROP_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,   // ROP_TRIANGLE_FAN
		GL_LINES,          // ROP_LINE_LIST
		GL_LINE_STRIP,     // ROP_LINE_STRIP
		GL_LINE_LOOP,      // ROP_LINE_LOOP
	};



	GLTexture::GLTexture(std::string filename,bool dynamic)
	{
		mFilename=filename;
		mDynamic=dynamic;
		mTexId=0; mWidth=mHeight=0;
	}

	GLTexture::GLTexture(unsigned char* rgba,int w,int h)
	{
		mWidth=w; mHeight=h;
		mDynamic=0;
		mFilename="UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w,h, 0, GL_RGBA, GL_UNSIGNED_BYTE,rgba);

	}

	GLTexture::~GLTexture()
	{
		unload();
	}

	void GLTexture::load()
	{
		rendersys->logMessage("loading GL texture '"+mFilename+"'");
		mTexId=platformLoadGLTexture(mFilename.c_str(),&mWidth,&mHeight);
		if (!mTexId) throw mFilename+"not found";
	}

	void GLTexture::unload()
	{
		if (mTexId)
		{
			rendersys->logMessage("unloading GL texture '"+mFilename+"'");
			glDeleteTextures(1, &mTexId);
			mTexId=0;
		}
	}

	int GLTexture::getSizeInBytes()
	{
		return mWidth*mHeight*3;
	}


	void GLRenderSystem::setViewport(float w,float h,float x_offset,float y_offset)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		// iphone has inverted x & y due to landscape
		float mat[]={2/w,0,0,0,  0,-2/h,0,0,  0,0,-2,0 ,-1-x_offset*2/w,1+y_offset*2/h,1,1}; // warning: matrix is transposed
		glMultMatrixf(mat);

		glMatrixMode(GL_MODELVIEW);

	}

	GLRenderSystem::GLRenderSystem(int w,int h) :
		mTexCoordsEnabled(0), mColorEnabled(0)
	{
		glViewport(0,0,w,h);
		glClearColor(0,0,0,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		
	#ifdef IPHONE_PLATFORM
		setViewport(1152,768,0,0);
	#else
		
	#endif
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);

		
		glDisable(GL_CULL_FACE);
	}

	GLRenderSystem::~GLRenderSystem()
	{

	}

	std::string GLRenderSystem::getName()
	{
		return "OpenGL Render System";
	}

	Texture* GLRenderSystem::loadTexture(std::string filename,bool dynamic)
	{
		
		if (dynamic) rendersys->logMessage("creating dynamic GL texture '"+filename+"'");
		GLTexture* t=new GLTexture(filename,dynamic);
		if (!dynamic) t->load();
		return t;
	}

	Texture* GLRenderSystem::createTextureFromMemory(unsigned char* rgba,int w,int h)
	{
		
		rendersys->logMessage("creating user-defined GL texture");
		GLTexture* t=new GLTexture(rgba,w,h);
		return t;

	}

	void GLRenderSystem::setTexture(Texture* t)
	{
		GLTexture* glt=(GLTexture*) t;
		if (t == 0) glBindTexture(GL_TEXTURE_2D,0);
		else
		{
			if (glt->mTexId == 0 && glt->isDynamic())
			{
				glt->load();
			}
			glBindTexture(GL_TEXTURE_2D,glt->mTexId);
		}
	}

	void GLRenderSystem::clear(bool color,bool depth)
	{
		GLbitfield mask=0;
		if (color) mask |= GL_COLOR_BUFFER_BIT;
		if (depth) mask |= GL_DEPTH_BUFFER_BIT;
		glClear(mask);
		
	}

	void GLRenderSystem::setIdentityTransform()
	{
		glLoadIdentity();
	}

	void GLRenderSystem::translate(float x,float y)
	{
		glTranslatef(x,y,0);
	}

	void GLRenderSystem::rotate(float angle)
	{
		glRotatef(angle,0,0,1);
	}

	void GLRenderSystem::scale(float s)
	{
		glScalef(s,s,s);
	}
				  
	void GLRenderSystem::pushTransform()
	{
		glPushMatrix();
	}

	void GLRenderSystem::popTransform()
	{
		glPopMatrix();
	}

	void GLRenderSystem::setBlendMode(BlendMode mode)
	{
		if (mode == ALPHA_BLEND || mode == DEFAULT)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (mode == ADD)
		{
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		}
	}

	void GLRenderSystem::render(int renderOp,TexturedVertex* v,int nVertices)
	{
		if (!mTexCoordsEnabled) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=true; }
		if (mColorEnabled) { glColor4f(1,1,1,mAlphaMultiplier); glDisableClientState(GL_COLOR_ARRAY); mColorEnabled=false; }

		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*) v+3*sizeof(float));

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);

	}

	void GLRenderSystem::render(int renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)
	{
		if (!mTexCoordsEnabled) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=true; }
		if (!mColorEnabled) { mColorEnabled=true; glDisableClientState(GL_COLOR_ARRAY); }    
		glColor4f(r,g,b,a*mAlphaMultiplier);
		
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*) v+3*sizeof(float));


		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(int renderOp,PlainVertex* v,int nVertices)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (mColorEnabled) { glColor4f(1,1,1,mAlphaMultiplier); glDisableClientState(GL_COLOR_ARRAY); mColorEnabled=false; }

		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(int renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (!mColorEnabled) { mColorEnabled=true; glDisableClientState(GL_COLOR_ARRAY);}

		glColor4f(r,g,b,a*mAlphaMultiplier);
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(int renderOp,ColoredVertex* v,int nVertices)
	{
		if (mTexCoordsEnabled) { glBindTexture(GL_TEXTURE_2D,0); glDisableClientState(GL_TEXTURE_COORD_ARRAY); mTexCoordsEnabled=false; }
		if (!mColorEnabled) { mColorEnabled=true; }
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertex), v);
		glColor4f(1,1,1,mAlphaMultiplier);
		glColorPointer(4, GL_UNSIGNED_BYTE,sizeof(ColoredVertex), (char*) v+3*sizeof(float));

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier=value;
		glColor4f(1,1,1,value);
	}

	void GLRenderSystem::presentFrame()
	{
		glutSwapBuffers();
	}
	
	void GLRenderSystem::enterMainLoop()
	{
		try
		{ glutMainLoop(); }
		catch (void*) {}
	}
	
	bool GLRenderSystem::triggerUpdate(float time_increase)
	{
		return mUpdateCallback(time_increase);
	}

	void gl_draw()
	{
		static unsigned long x=GetTickCount();
		float k=(GetTickCount()-x)/1000.0f;
		x=GetTickCount();
		if (!((GLRenderSystem*) rendersys)->triggerUpdate(x)) throw "done";
		rendersys->presentFrame();
	}

	void createGLRenderSystem(int w,int h,bool fullscreen,std::string title)
	{
		const char *argv[] = {"program"};
		int argc=1;
		glutInit(&argc,(char**) argv);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
		int _w=glutGet(GLUT_SCREEN_WIDTH);
		int _h=glutGet(GLUT_SCREEN_HEIGHT);
		glutInitWindowPosition(_w/2-w/2,_h/2-h/2);
		glutInitWindowSize(w,h);
		glutCreateWindow(title.c_str());
		HWND hWnd = FindWindow("GLUT", title.c_str());
		SetFocus(hWnd);

		glutDisplayFunc(gl_draw);
//		glutMouseFunc(mouse_click_handler);
//		glutKeyboardFunc(keyboard_handler);
//		glutKeyboardUpFunc(keyboard_up_handler);
//		glutMotionFunc(mouse_move_handler);
		
//		glutSpecialFunc(special_handler);
		//glutSpecialFunc(arrow_keys);
//		glutIdleFunc(platform_draw);

		ilInit();
		ilutRenderer(ILUT_OPENGL);
		rendersys=new GLRenderSystem(w,h);
	}

	void destroyGLRenderSystem()
	{
		
	}
}
