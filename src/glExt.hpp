
#pragma once

// 
// OpenGL拡張
//

// リンクするライブラリの指示(Windows)
#if defined (_MSC_VER)
#ifdef DEBUG
#pragma comment(lib, "glew32sd.lib")
#else
#pragma comment(lib, "glew32s.lib")
#endif
#endif


namespace {

#if defined (_MSC_VER)

// 初期化
// false: 拡張機能は使えない
bool initGlExt() {
  auto result_code = glewInit();
  return result_code == GLEW_OK;
}

// 画面更新がモニタと同期可能か調べる(OpenGLの初期化後に呼び出す事)
bool isVsyncSwap() {
  const char* ext = (const char*)glGetString(GL_EXTENSIONS);

  return std::strstr(ext, "WGL_EXT_swap_control") ? true : false;
}

// モニタとの同期間隔を設定
// sync インターバル間隔(0で同期しない)
bool VsyncSwapInterval(int sync) {
  typedef bool (APIENTRY* PFNWGLSWAPINTERVALFARPROC)( int );
  
  PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT
    = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

  return (wglSwapIntervalEXT) ? wglSwapIntervalEXT(sync) : false;
}

#else

// OSX, iOSでは必ず使える
bool initGlExt() { return true; }
bool isVsyncSwap() { return true; }
bool VsyncSwapInterval(int sync) { return true; }

#endif

}
