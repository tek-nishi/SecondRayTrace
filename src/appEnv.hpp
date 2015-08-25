
#pragma once

//
// アプリの実行環境
//

#include "defines.hpp"
#include <iostream>
#include <array>
#include <set>
#include <functional>
#include <boost/noncopyable.hpp>
#include "glfwWindow.hpp"
#include "vector.hpp"
#include "utils.hpp"
#include "color.hpp"


namespace {

// マウスボタンの省略名を定義
enum Mouse {
  LEFT  = GLFW_MOUSE_BUTTON_LEFT,
  RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
};


class AppEnv : private boost::noncopyable {
  // 描画ウィンドウ
  GlfwWindow window_;

  // 現在のウィンドウサイズ
  Vec2f window_size_;
  
  Color bg_color_;
  
  // キー入力関連
  u_int pushed_key_;

  std::array<std::set<int>, 2> push_keys_;
  std::array<std::set<int>, 2> pull_keys_;
  std::set<int> press_keys_;
  int key_page_;

  // マウス関連
  bool  mouse_left_press_;
  bool  mouse_right_press_;
  Vec2f mouse_pos_;
  Vec2f mouse_last_pos_;
  Vec2f mouse_current_pos_;

  std::array<std::set<int>, 2> push_buttons_;
  std::array<std::set<int>, 2> pull_buttons_;
  std::set<int> press_buttons_;
  int buttons_page_;
  
  
public:
  AppEnv(const int width, const int height, const bool full_screen = false) :
    window_(width, height, true, full_screen),
    window_size_(width, height),
    bg_color_(0, 0, 0, 0),
    key_page_(0),
    mouse_left_press_(false),
    mouse_right_press_(false),
    mouse_pos_(0, 0),
    mouse_last_pos_(0, 0),
    mouse_current_pos_(0, 0),
    buttons_page_(0)
  {
    DOUT << "AppEnv()" << std::endl;

    // GLFWのハンドルに自分自身を登録
    glfwSetWindowUserPointer(window_(), this);

    // ウインドウのサイズ変更
    glfwSetWindowSizeCallback(window_(), changeWindowSize);

    // キーが押された時に呼ばれる関数を登録
    glfwSetCharCallback(window_(), createCharaInfo);
    glfwSetKeyCallback(window_(), createKeyInfo);

    // マウスイベント
    glfwSetMouseButtonCallback(window_(), mouseButtonCallback);
    glfwSetCursorPosCallback(window_(), mouseMoveCallback);

    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
  }

  ~AppEnv() {
    DOUT << "~AppEnv()" << std::endl;
  }

  
  // アプリウインドウが開いてるならtrueを返す
  bool isOpen() {
    return !glfwWindowShouldClose(window_());
  }

  // 描画準備
  void setupDraw() {
    // ウインドウの内容を指定色で消去
    glClearColor(bg_color_.red(), bg_color_.green(), bg_color_.blue(), bg_color_.alpha());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 半透明描画指示
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 深度テスト有効
		glEnable(GL_DEPTH_TEST);

    // 裏面は描画しない
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
  }

  // アプリウインドウの更新
  // 1. OpenGLの描画内容をウインドウに表示
  // 2. キーやマウスイベントのポーリング
  void update() {
    // GLFWへ描画指示
    glfwSwapBuffers(window_());

    // 入力(キー＆ボタン)の再初期化
    switchInputBuffer();
    
    glfwPollEvents();
  }

  // 入力(キー＆ボタン)の再初期化
  void flushInput() {
    switchInputBuffer();
    switchInputBuffer();
  }
  
  // 現在のウィンドウサイズを返す
  const Vec2f& viewSize() const { return window_size_; }

  // 背景色の変更
  // color 色
  void bgColor(const Color& color) { bg_color_ = color; }

  
  // 押されたキーを取得
  // 'A'とか'a'とか(押されてないときは0)
  // キーリピート付き
  u_int getPushedKey() const { return pushed_key_; }

  
  // 当該キーが押されているならtrueを返す
  // key 'A'とか'7'とか
  // SOURCE:include/GLFW/glfw3.h 271〜396
  bool isPressKey(const int key) {
    return press_keys_.find(key) != press_keys_.cend();
  }
  
  // 当該キーが押された瞬間trueを返す
  // key 'A'とか'7'とか
  // SOURCE:include/GLFW/glfw3.h 271〜396
  bool isPushKey(const int key) {
    const auto& keys = push_keys_[key_page_ ^ 1];
    return keys.find(key) != keys.cend();
  }

  // 当該キーが離された瞬間trueを返す
  bool isPullKey(const int key) {
    const auto& keys = pull_keys_[key_page_ ^ 1];
    return keys.find(key) != keys.cend();
  }

  
  // マウスのカーソル位置を返す
  const Vec2f& mousePosition() const { return mouse_current_pos_; }

  // 当該ボタンが押されているならtrueを返す
  // button Mouse::LEFT
  //        Mouse::Right
  bool isPressButton(const int button) {
    return press_buttons_.find(button) != press_buttons_.cend();
  }
  
  // 当該ボタンが押された瞬間trueを返す
  // button Mouse::LEFT
  //        Mouse::RIGHT
  bool isPushButton(const int button) {
    const auto& buttons = push_buttons_[buttons_page_ ^ 1];
    return buttons.find(button) != buttons.cend();
  }

  // 当該ボタンが離された瞬間trueを返す
  // button Mouse::LEFT
  //        Mouse::RIGHT
  bool isPullButton(const int button) {
    const auto& buttons = pull_buttons_[buttons_page_ ^ 1];
    return buttons.find(button) != buttons.cend();
  }
  
  
private:
  // キーが押された時に呼び出される
  static void createCharaInfo(GLFWwindow* window, const u_int chara) {
    auto* const obj = static_cast<AppEnv*>(glfwGetWindowUserPointer(window));
    
    obj->pushed_key_ = chara;
  }

  static void createKeyInfo(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
    auto* const obj = static_cast<AppEnv*>(glfwGetWindowUserPointer(window));

    // キーのpush,press,pull情報を生成
    switch (action) {
    case GLFW_PRESS:
      obj->push_keys_[obj->key_page_].insert(std::set<int>::value_type(key));
      obj->press_keys_.insert(std::set<int>::value_type(key));
      break;

    case GLFW_RELEASE:
      obj->pull_keys_[obj->key_page_].insert(std::set<int>::value_type(key));
      obj->press_keys_.erase(key);
      break;
    }
  }

  static void changeWindowSize(GLFWwindow* window, const int width, const int height) {
    // ウインドウサイズの変更をアプリに伝える
    auto* const obj = static_cast<AppEnv*>(glfwGetWindowUserPointer(window));
    obj->window_size_ << width, height;

    glViewport(0, 0, width, height);
  }


  static void mouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods) {
    auto* const obj = static_cast<AppEnv*>(glfwGetWindowUserPointer(window));

    // ボタン入力情報を生成
    switch (action) {
    case GLFW_PRESS:
      obj->push_buttons_[obj->buttons_page_].insert(std::set<int>::value_type(button));
      obj->press_buttons_.insert(std::set<int>::value_type(button));
      break;

    case GLFW_RELEASE:
      obj->pull_buttons_[obj->buttons_page_].insert(std::set<int>::value_type(button));
      obj->press_buttons_.erase(button);
      break;
    }
    
    double x_pos, y_pos;
    glfwGetCursorPos(obj->window_(), &x_pos, &y_pos);

#if 0
    obj->mouse_current_pos_ = screenPosition(Vec2f(x_pos, y_pos), obj->window_size_);
    // TIPS:Yは上下が逆
    obj->mouse_current_pos_.y() = -obj->mouse_current_pos_.y();
#endif
    
    // マウスのボタンが押されたのをアプリに伝える
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      obj->mouse_left_press_ = (action == GLFW_PRESS) ? true : false;
      obj->mouse_pos_ << x_pos, y_pos;
      break;

    case GLFW_MOUSE_BUTTON_RIGHT:
      obj->mouse_right_press_ = (action == GLFW_PRESS) ? true : false;
      obj->mouse_pos_ << x_pos, y_pos;
      break;
    }
  }

  static void mouseMoveCallback(GLFWwindow* window, const double x_pos, const double y_pos) {
    auto* const obj = static_cast<AppEnv*>(glfwGetWindowUserPointer(window));

    
    obj->mouse_current_pos_ = screenPosition(Vec2f(x_pos, y_pos), obj->window_size_);

    // TIPS:Yは上下が逆
    obj->mouse_current_pos_.y() = -obj->mouse_current_pos_.y();

    if (!obj->mouse_left_press_ && !obj->mouse_right_press_) return;

    // ボタンが押されている状態でのマウスカーソルの移動をアプリに伝える
    obj->mouse_last_pos_ = obj->mouse_pos_;
    obj->mouse_pos_ << x_pos, y_pos;
  }

  // 画面中央が(0, 0)の座標を計算
  static Vec2f screenPosition(const Vec2f& pos, const Vec2f& window) {
    return pos - window / 2;
  }

  // 入力バッファを切り替える
  void switchInputBuffer() {
    pushed_key_ = 0;
    key_page_ ^= 1;

    push_keys_[key_page_].clear();
    pull_keys_[key_page_].clear();

    buttons_page_ ^= 1;

    push_buttons_[buttons_page_].clear();
    pull_buttons_[buttons_page_].clear();
  }
  
};

}
