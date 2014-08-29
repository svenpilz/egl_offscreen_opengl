CXXFLAGS = -std=c++11
LIBS = `pkg-config --libs --cflags egl gl opencv`

egl_opengl_test: egl_opengl_test.cpp
	g++ $(CXXFLAGS) $(LIBS) -o $@ $<

test: egl_opengl_test
	./egl_opengl_test

test_without_x11: egl_opengl_test
	EGL_PLATFORM=drm ./egl_opengl_test

clean:
	rm -f egl_opengl_test img.png
