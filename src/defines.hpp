
#pragma once

//
// 基本的な定義
//

// TIPS:std::cout をReleaseビルドで排除する
#ifdef DEBUG
#define DOUT std::cout
#else
#define DOUT 0 && std::cout
#endif

// TIPS:プリプロセッサを文字列として定義する
#define PREPRO_TO_STR(value) PREPRO_STR(value)
#define PREPRO_STR(value)    #value


#if 1

using Real = float;
#define glMultMatrix(...) glMultMatrixf(__VA_ARGS__)
#define glLoadMatrix(...) glLoadMatrixf(__VA_ARGS__)

#else

using Real = double;
#define glMultMatrix(...) glMultMatrixd(__VA_ARGS__)
#define glLoadMatrix(...) glLoadMatrixd(__VA_ARGS__)

#endif


// 符号無し整数の別名定義
using u_char = unsigned char;
using u_int  = unsigned int;
using u_long = unsigned long;


// Windows特有の定義
#if defined (_MSC_VER)

// 文字リテラルをutf-8に
// FIXME:パス名に英数字以外を使うと読めない
#pragma execution_character_set("utf-8")

// TIPS:std::min std::maxを使うための定義
#define NOMINMAX

// TIPS:M_PIなどの定義を有効にする
#define _USE_MATH_DEFINES

// いくつかの余計な警告を表示しないようにする
#pragma warning (disable:4244)
#pragma warning (disable:4800)
#pragma warning (disable:4996)

// GLEWのリンク形式
#define GLEW_STATIC
// GLFWのリンク形式
#define GLFW_DLL

#include <windows.h>

#endif
