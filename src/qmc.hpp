
#pragma once

//
// QMC
// SOURCE:https://github.com/githole/simple-pathtracer
// 

#include "defines.hpp"
#include <cmath>
#include <algorithm>


std::vector<int> prime_numbers;

void init_prime_numbers() {
	// エラトステネスの篩
	const int N = 10000000; // 10000000までの素数をテーブルに格納する（664579個）

	std::vector<char> table;
	table.resize(N + 1, 0);
		
	for (int i = 2; (i * i) <= N; i ++) {
		if (table[i] == 0) { // iが素数であった
			// ふるう
			for (int j = i + i; j <= N; j += i) table[j] = 1;
		}
	}

	// 列挙する
	for (int i = 2; i <= N; i ++) {
		if (table[i] == 0) {
			prime_numbers.push_back(i);
		}
	}
}


class Qmc {
	int j;    // Halton列のj個目の値を得る
	int ith;  // i番目のサンプル


public:
	Qmc(const int ith_) : ith(ith_) {
		j = 0;
	}

	// Halton列を得る
	// Radical inverse function 
	//   φ_b(i) = 0.d1d2d3...dk...
	// i番目のn次元Halton列 
	//   x_i = (φ_2(i), φ_3(i),..., φ_pj(i),..., φ_pn(i)), pn = n番目の素数
	//
	
	// i番目のn次元Halton列のj個目の要素を得る
	// つまりφ_pj(i)を計算する
	Real next() {
		// 素数テーブルを超える次元のサンプルを得ることはできない！
		// ロシアンルーレットがあまりにもうまくいった場合など、ここに入ることもあり得る（非常に低確率だが）
		// 先にスタックオーバーフローするかもしれない
		if (j >= prime_numbers.size()) {
			return rand01();
		}
		// 以下、φ_b(i)を計算する
		const int base = prime_numbers[j++];
    
		Real value = 0.0;
		Real inv_base = 1.0 / base;
		Real factor = inv_base;

		int i = ith;

		while (i > 0) {
			// 適当にd_kを入れ替える	
			const int d_k = reverse(i % base, base); 
			value += d_k * factor;
			i /= base;
			factor *= inv_base;
		}
		return value;
	}

  
private:
  static Real rand01() { return (Real)rand() / RAND_MAX; }
  
	int reverse(const int i, const int p) {
		if (i == 0) return i; 
		else        return p - i;	
	}
};
