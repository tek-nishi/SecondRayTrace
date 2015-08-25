
#pragma once

//
// OpenGLでのプレビュー
//

#include "appEnv.hpp"
#include "camera3D.hpp"
#include "light.hpp"
#include "modelDraw.hpp"


namespace Preview {

void setup(const std::vector<Light>& lights, const Pixel& ambient) {
  // 光源設定
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  GLfloat light_ambient[] = { GLfloat(ambient.x()),
                              GLfloat(ambient.y()),
                              GLfloat(ambient.z()),
                              1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);

  int il = 0;
  for (const auto& l : lights) {
    glEnable(GL_LIGHT0 + il);

    GLfloat light_diffuse[] = { GLfloat(l.diffuse.x()),
                                GLfloat(l.diffuse.y()),
                                GLfloat(l.diffuse.z()),
                                1.0f };
    glLightfv(GL_LIGHT0 + il, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0 + il, GL_SPECULAR, light_diffuse);

    ++il;
  }
}


void display(AppEnv& app_env,
             Camera3D& camera, const std::vector<Light>& lights,
             const Model& model) {
  app_env.setupDraw();

  auto matrix = camera(app_env.viewSize());

  glMatrixMode(GL_PROJECTION);
  glLoadMatrix(matrix.first.data());

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrix(matrix.second.data());

  // TIPS:光源の位置設定はモデリング行列を設定した後で
  int il = 0;
  for (const auto& l : lights) {
    // TIPS:wが1だと点光源
    GLfloat light_position[] = { GLfloat(l.position.x()),
                                 GLfloat(l.position.y()),
                                 GLfloat(l.position.z()),
                                 1.0f };
    glLightfv(GL_LIGHT0 + il, GL_POSITION, light_position);

    ++il;
  }
    
  modelDraw(model);
    
  app_env.update();
}

}
