/*
 * Example program for creating an OpenGL context with EGL for offscreen
 * rendering with a framebuffer.
 *
 *
 * The MIT License (MIT)
 * Copyright (c) 2014 Sven-Kristofer Pilz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <sstream>
#include <stdexcept>

/*
 * OpenCV for saving the render target as an image file.
 */
#include <opencv2/opencv.hpp>

/*
 * EGL headers.
 */
#include <EGL/egl.h>

/*
 * OpenGL headers.
 */
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>

using namespace std;

void assertOpenGLError(const std::string& msg) {
	GLenum error = glGetError();

	if (error != GL_NO_ERROR) {
		stringstream s;
		s << "OpenGL error 0x" << std::hex << error << " at " << msg;
		throw runtime_error(s.str());
	}
}

void assertEGLError(const std::string& msg) {
	EGLint error = eglGetError();

	if (error != EGL_SUCCESS) {
		stringstream s;
		s << "EGL error 0x" << std::hex << error << " at " << msg;
		throw runtime_error(s.str());
	}
}

int main() {
	/*
	 * EGL initialization and OpenGL context creation.
	 */
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;
	EGLint num_config;

	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assertEGLError("eglGetDisplay");
	
	eglInitialize(display, nullptr, nullptr);
	assertEGLError("eglInitialize");

	eglChooseConfig(display, nullptr, &config, 1, &num_config);
	assertEGLError("eglChooseConfig");
	
	eglBindAPI(EGL_OPENGL_API);
	assertEGLError("eglBindAPI");
	
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
	assertEGLError("eglCreateContext");

	//surface = eglCreatePbufferSurface(display, config, nullptr);
	//assertEGLError("eglCreatePbufferSurface");
	
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);
	assertEGLError("eglMakeCurrent");
	
	
	/*
	 * Create an OpenGL framebuffer as render target.
	 */
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	assertOpenGLError("glBindFramebuffer");

	
	/*
	 * Create a texture as color attachment.
	 */
	GLuint t;
	glGenTextures(1, &t);

	glBindTexture(GL_TEXTURE_2D, t);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	assertOpenGLError("glTexImage2D");
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	
	/*
	 * Attach the texture to the framebuffer.
	 */
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t, 0);
	assertOpenGLError("glFramebufferTexture2D");

	
	/*
	 * Render something.
	 */
	glClearColor(0.9, 0.8, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();

	
	/*
	 * Read the framebuffer's color attachment and save it as a PNG file.
	 */
	cv::Mat image(500, 500, CV_8UC3);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, 500, 500, GL_BGR, GL_UNSIGNED_BYTE, image.data);
	assertOpenGLError("glReadPixels");

	cv::imwrite("img.png", image);
	
	
	/*
	 * Destroy context.
	 */
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &t);
	 
	//eglDestroySurface(display, surface);
	//assertEGLError("eglDestroySurface");
	
	eglDestroyContext(display, context);
	assertEGLError("eglDestroyContext");
	
	eglTerminate(display);
	assertEGLError("eglTerminate");

	return 0;
}

