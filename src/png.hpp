
#pragma once

// 
// libpngによる画像読み書き
//
// PNG_COLOR_TYPE_RGB		    RGB
// PNG_COLOR_TYPE_RGB_ALPHA	RGBA
//

#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
#include <png.h>


// リンクするライブラリの定義(Windows)
#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "libpngd.lib")
#pragma comment (lib, "zlibd.lib")
#else
#pragma comment (lib, "libpng.lib")
#pragma comment (lib, "zlib.lib")
#endif
#endif


namespace {

// TIPS:RAIIによる安全なpng_structリソース管理
class PngStruct {
	png_struct* hdl_;
	png_info*   info_;

  
public:
	PngStruct() :
		hdl_(),
		info_()
	{
		hdl_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		!hdl_ && DOUT << "Error:png_create_read_struct()" << std::endl;
		
		if (hdl_) {
			info_ = png_create_info_struct(hdl_);
			!info_ && DOUT << "Error:png_create_info_struct()" << std::endl;
		}
	}

	~PngStruct() {
		if (hdl_) png_destroy_read_struct(&hdl_, &info_, 0);
	}

	bool error() const { return !hdl_ || !info_; }
	png_struct* hdl() const { return hdl_; }
	png_info* info() const { return info_; }
};


class Png {
	enum {
    PNG_SIG_HEADER = 8,
    // PNGかどうかを判別するために読み込むデータ量

    PNG_COLOR_TYPE_NONE = -1
  };

	int type_;
	int width_;
	int height_;
	std::vector<u_char> image_;

  
public:
	Png(const std::string& path) :
		type_(PNG_COLOR_TYPE_NONE)
	{
		DOUT << "Png()" << std::endl;

    std::ifstream fstr(path, std::ios::binary);
    if (!fstr) {
      DOUT << "PNG flle open error. " << path << std::endl;
      throw;
    }

    // PNGかどうかを検証
    png_byte header[PNG_SIG_HEADER];
    fstr.read(reinterpret_cast<char*>(header), PNG_SIG_HEADER);
    if (int cmp = png_sig_cmp(header, 0, PNG_SIG_HEADER)) {
			DOUT << "Error:png_sig_cmp():" << cmp << std::endl;
			throw;
    }

    // png読み込み用ハンドル生成
		PngStruct png;
		if (png.error()) throw "Error:Can't create PngStruct.";

    // 標準的なFILE I/Oを使わないので、読み込み処理を自分で用意
		png_set_read_fn(png.hdl(), static_cast<png_voidp>(&fstr), static_cast<png_rw_ptr>(readFunc));

    // 検証したぶんをスキップしつつ、各種情報を取得
    png_set_sig_bytes(png.hdl(), PNG_SIG_HEADER);
		png_read_info(png.hdl(), png.info());

		png_uint_32 width, height;
		int depth;
		png_get_IHDR(png.hdl(), png.info(), &width, &height, &depth, &type_, 0, 0, 0);
    width_ = width;
    height_ = height;

		// どんなフォーマットもRGB8かRGBA8に収めるように準備
		if (type_ == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png.hdl());
		if (png_get_valid(png.hdl(), png.info(), PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png.hdl());
		if (depth == 16) png_set_strip_16(png.hdl());
		if (depth < 8) png_set_packing(png.hdl());

    // フォーマット更新
		png_read_update_info(png.hdl(), png.info());

		// 最終的なフォーマットを決定
		size_t row = png_get_rowbytes(png.hdl(), png.info());
		type_ = ((row / width) == 3) ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
		
		// 読込先の領域を確保し、読み込みテーブルを生成
		image_.resize(row * height);
		std::vector<png_bytep> row_pointers(height);
		for (size_t h = 0; h < height; ++h) {
			row_pointers[h] = &image_[h * row];
		}
		
		png_read_image(png.hdl(), &row_pointers[0]);
		png_read_end(png.hdl(), png.info());

		DOUT << "PNG:" << width_ << " x " << height_ << " format:" << type_ << std::endl;
	}
  
	~Png() {
		DOUT << "~Png()" << std::endl;
	}

  
  // 画像タイプ
  // PNG_COLOR_TYPE_RGB
  // PNG_COLOR_TYPE_RGB_ALPHA
  int type() const { return type_; }

  // サイズ
  int width() const { return width_; }
  int height() const { return height_; }

  // ピクセルデータ
  const u_char* image() const { return &image_[0]; }


private:
  // png読み込み時のコールバック
  static void readFunc(png_struct* hdl, png_bytep buf, png_size_t size) {
    std::ifstream* fstr = static_cast<std::ifstream*>(png_get_io_ptr(hdl));

    // TIPS:一旦void*にしてから目的のポインタにcastする
    void* p = buf;
    fstr->read(static_cast<char*>(p), size);
  }
  
};


//
// png書き出し
//
class PngWriteStruct {
	png_struct* hdl_;
	png_info*   info_;

  
public:
	PngWriteStruct() :
		hdl_(0),
		info_(0)
	{
		hdl_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		!hdl_ && DOUT << "Error:png_create_write_struct()" << std::endl;
		
		if (hdl_) {
			info_ = png_create_info_struct(hdl_);
			!info_ && DOUT << "Error:png_create_info_struct()" << std::endl;
		}
	}

	~PngWriteStruct() {
		if (hdl_) png_destroy_write_struct(&hdl_, &info_);
	}

  
	bool error() const { return !hdl_ || !info_; }
	png_struct *hdl() const { return hdl_; }
	png_info *info() const { return info_; }

};

// RGB8で書き出し
void WritePng(const std::string& path, const u_int width, const u_int height, u_char* image) {
  // 書き出しテーブルの用意
  // TIPS:上下を反転
	std::vector<png_bytep> row_pointers(height);
	for (size_t h = 0; h < height; ++h) {
		row_pointers[h] = &image[(height - h - 1) * width * 3];
	}
	
	PngWriteStruct png;
	if (png.error()) throw "Error:Can't create PngWriteStruct.";

	FILE* fp = fopen(path.c_str(), "wb");
	if (!fp) {
		DOUT << "File create error:" << path << std::endl;
		throw;
	}

	png_init_io(png.hdl(), fp);

	png_set_IHDR(png.hdl(), png.info(), width, height, 8,
							 PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png.hdl(), png.info());

	png_write_image(png.hdl(), &row_pointers[0]);
	png_write_end(png.hdl(), 0);

	fclose(fp);
}

}
