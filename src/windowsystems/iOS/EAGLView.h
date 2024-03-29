/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an EAGL view.

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface EAGLView : UIView <UITextFieldDelegate>
{
	
@private
	/* The pixel dimensions of the backbuffer */
	GLint backingWidth;
	GLint backingHeight;
	
    CADisplayLink *displayLink;
	EAGLContext *context;
	
	/* OpenGL names for the renderbuffer and framebuffers used to render to this view */
	GLuint viewRenderbuffer, viewFramebuffer;
	
	/* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
	GLuint depthRenderbuffer;
	
	/* Textfield */
	UITextField *textField;
@public
	bool app_started;
	bool displayLinkAttached;
	int frameInterval;
}

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView:(CADisplayLink*)link;
- (void)swapBuffers;
- (void)beginKeyboardHandling;
- (void)terminateKeyboardHandling;
- (BOOL)isKeyboardActive;
- (void)keyboardWasShown:(id)sender;
- (void)keyboardWasHidden:(id)sender;
- (BOOL)textFieldShouldReturn:(UITextField*)aTextField;
- (void)deviceOrientationDidChange:(id)sender;
- (void)applicationDidBecomeActive:(UIApplication*)app;
- (void)applicationWillResignActive:(UIApplication*)app;
- (void)setUpdateInterval:(int)interval;

@end
