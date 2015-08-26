//
// 二回目のレイトレース
//

#include "defines.hpp"
#include <cassert>
#include <future>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <deque>
#include "appEnv.hpp"
#include "json.hpp"
#include "sceneLoader.hpp"
#include "preview.hpp"
#include "pathtrace.hpp"
#include "os.hpp"
#include "bvh.hpp"
#include "hdri.hpp"


std::shared_ptr<Pathtrace::RenderInfo> createRenderInfo(const picojson::value& params,
                                                        const Os& os,
                                                        const int window_width, const int window_height,
                                                        const std::vector<GLint>& viewport,
                                                        const Scene& scene,
                                                        const std::vector<std::vector<int> >& perm_table) {

  auto info = std::make_shared<Pathtrace::RenderInfo>(window_width, window_height,
                                                      viewport,

                                                      scene.camera,
                                                      scene.ambient,
                                                      scene.lights,
                                                      scene.model,

                                                      os.documentPath() + "res/" + params.at("environment").get<std::string>(),
                                                      perm_table,

                                                      int(params.at("subpixel_num").get<double>()),
                                                      int(params.at("sample_num").get<double>()),
                                                      int(params.at("recursive_depth").get<double>()),

                                                      params.at("focal_distance").get<double>(),
                                                      params.at("lens_radius").get<double>(),
    
                                                      params.at("exposure").get<double>());

  return info;
}


int main() {
  // OS依存実装
  Os os;
  
  // 設定をJsonから読み込む
  auto params = Json::read(os.documentPath() + "res/params.json");
  
  const int window_width  = params.at("window_width").get<double>();
  const int window_height = params.at("window_height").get<double>();

  // プレビュー環境作成
  AppEnv app_env{ window_width, window_height };

  auto scene = SceneLoader::load(os.documentPath() + "res/" + params.at("path").get<std::string>());

#ifdef DEBUG
  // Cheetah3Dが書き出すColladaはIORを含んでいないので、検証用に設定
  if (params.contains("ior_value")) {
    auto& model = scene.model;
    auto& materials = model.material();
    Real ior_value = params.at("ior_value").get<double>();
    for (auto& m : materials) {
      m.ior(ior_value);
    }
  }
#endif

  // 書き出し先(フォルダ作成)
  std::string save_path{ os.documentPath() + "progress" };
  Os::createDirecrory(save_path);

  // レンダリング結果の格納先
  auto row_image = std::make_shared<std::vector<u_char> >(window_height * window_width * 3);
  std::fill(row_image->begin(), row_image->end(), 255);

  // posToWorldで使うviewportの準備
  std::vector<GLint> viewport{ 0, 0, window_width, window_height };

  // Halton列で使うシャッフル列の生成
  std::vector<std::vector<int> > perm_table = faurePermutation(100);

  // レンダリングに必要な情報を生成
  auto info = createRenderInfo(params,
                               os,
                               window_width, window_height,
                               viewport,
                               scene,
                               perm_table);

  // カメラの内部行列を生成
  //   posToWorldで使う
  info->camera(Vec2f{ window_width, window_height });

  // OpenGLでのプレビュー
  Preview::setup(scene.lights, scene.ambient);
  // プレビュー時にカメラを変更するのでコピーしておく
  Camera3D preview_camera = scene.camera;
  
  const std::chrono::seconds wait_time(int(params.at("wait_time").get<double>()));

  // Raytraceスレッド開始
  std::packaged_task<bool()> task(std::bind(Pathtrace::render,
                                            row_image, info));

  auto future = task.get_future();
  std::thread render_thread{ std::move(task) };
  render_thread.detach();

  auto render_begin = std::chrono::steady_clock::now();

  int png_index = 1;

  while (1) {
    if (!app_env.isOpen()) break;

    Preview::display(app_env,
                     preview_camera, scene.lights, scene.model);

    // 動作完了待ち
    auto result = future.wait_for(wait_time);
    if (result == std::future_status::ready) {
      // 結果を空読み
      future.get();

      WritePng(save_path + "/completion.png",
               window_width, window_height,
               &(*row_image)[0]);

      // 所要時間を計算
      auto current = std::chrono::steady_clock::now();
      auto end = std::chrono::duration_cast<std::chrono::milliseconds>(current - render_begin);

      std::cout << "Render time (sec):" << end.count() / 1000.0f << std::endl;

      break;
    }

    {
      // 途中経過を出力
      std::ostringstream path;
      path << save_path << "/" << std::setw(2) << std::setfill('0') << png_index << ".png";

      DOUT << "progress:" << path.str() << std::endl;

      WritePng(path.str(),
               window_width, window_height,
               &(*row_image)[0]);

      png_index += 1;
    }
  }
}
