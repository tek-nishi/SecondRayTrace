
#pragma once

//
// テクスチャを名前で管理する
//

#include "defines.hpp"
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "texture.hpp"
#include "fileUtil.hpp"


namespace {

class TexMng {
public:
	typedef std::shared_ptr<Texture> TexPtr;

  
private:
	std::map<std::string, TexPtr> tex_obj_;

  
public:
	TexMng() {
		DOUT << "TexMng()" << std::endl;
	}
  
	~TexMng() {
		DOUT << "~TexMng()" << std::endl;
	}

  
	TexPtr read(const std::string& path) {
		std::string name = getFilename(path);
		auto it = tex_obj_.find(name);
    if (it != tex_obj_.end()) {
      return it->second;
    }

    // 見つからない場合はテクスチャを読み込んでコンテナに格納する
    DOUT << "texmng read: " << path << std::endl;
    TexPtr obj(std::make_shared<Texture>(path));

    // shared_ptrなので、emplaceである必要性は低い
    tex_obj_.insert(std::map<std::string, TexPtr>::value_type(name, obj));

    return obj;
	}

	TexPtr get(const std::string& name) {
		auto it = tex_obj_.find(name);
    if (it != tex_obj_.end()) {
      return it->second;
    }

    // FIXME:見つからない時に空のshared_ptrを返すのはよくない
    DOUT << "No texture obj:" << name << std::endl;
    return TexPtr();
	}
};

}
