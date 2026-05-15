#include <Novice.h>

const char kWindowTitle[] = "LC1C_12_ショウ_ズーウェン";

struct Vector2
{
	float x;
	float y;
	float z;
};

struct Matrix4x4
{
	float m[4][4];
};

// 行列の加算
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}

// 行列の減算
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}

// 行列の乗算
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return result;
}

// 単位行列の作成 (Inverseより先に定義する必要があります)
Matrix4x4 MakeIdentity4x4()
{
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
	{
		result.m[i][i] = 1.0f;
	}
	return result;
}

// 逆行列の計算 (掃き出し法 / ガウス・ジョルダン法)
Matrix4x4 Inverse(const Matrix4x4& m)
{
	// 4x8 の作業用行列を拡張（左側に元の行列、右側に単位行列を配置）
	float mat[4][8];
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			mat[i][j] = m.m[i][j];
			mat[i][j + 4] = (i == j) ? 1.0f : 0.0f;
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		// 部分ピボット選択（絶対値が最大の行を探す）
		int maxRow = i;
		float maxVal = mat[i][i] >= 0 ? mat[i][i] : -mat[i][i];
		for (int k = i + 1; k < 4; ++k)
		{
			float val = mat[k][i] >= 0 ? mat[k][i] : -mat[k][i];
			if (val > maxVal)
			{
				maxVal = val;
				maxRow = k;
			}
		}

		// 必要に応じて行を入れ替える
		if (maxRow != i)
		{
			for (int j = 0; j < 8; ++j)
			{
				float temp = mat[i][j];
				mat[i][j] = mat[maxRow][j];
				mat[maxRow][j] = temp;
			}
		}

		float pivot = mat[i][i];
		// ピボットが0の場合は逆行列が存在しないため、単位行列を返して安全に抜ける
		if (pivot == 0.0f)
		{
			return MakeIdentity4x4();
		}

		// ピボット行をピボットの値で割る（注目要素を1にする）
		for (int j = 0; j < 8; ++j)
		{
			mat[i][j] /= pivot;
		}

		// 他の行の該当列を0にする
		for (int k = 0; k < 4; ++k)
		{
			if (k != i)
			{
				float factor = mat[k][i];
				for (int j = 0; j < 8; ++j)
				{
					mat[k][j] -= factor * mat[i][j];
				}
			}
		}
	}

	// 右側の4x4部分（逆行列）を結果にコピーして返す
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result.m[i][j] = mat[i][j + 4];
		}
	}
	return result;
}

// 転置行列の計算 (行と列の入れ替え)
Matrix4x4 Transpose(const Matrix4x4& m)
{
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			// 元の行列の[j][i]を、結果の[i][j]に代入
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

static const int kRowHeight = 20;
static const int kColumnWidth = 60;

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label)
{
	Novice::ScreenPrintf(x, y, "%s", label);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			// 行列の表示はラベルの分(1行分)下にずらすため (i + 1) * kRowHeight とする
			Novice::ScreenPrintf(x + j * kColumnWidth, y + (i + 1) * kRowHeight, "%6.2f", matrix.m[i][j]);
		}
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Matrix4x4 m1 = {
		3.2f,0.7f,9.6f,4.4f,
		5.5f,1.3f,7.8f,2.1f,
		6.4f,8.0f,2.6f,1.0f,
		0.5f,7.2f,5.1f,3.3f
	};

	Matrix4x4 m2 = {
		1.2f,4.5f,6.7f,8.9f,
		2.3f,5.6f,7.8f,9.0f,
		3.4f,6.7f,8.9f,1.2f,
		4.5f,7.8f,9.0f,2.3f
	};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 resultAdd = Add(m1, m2);
		Matrix4x4 resultMultiply = Multiply(m1, m2);
		Matrix4x4 resultSubtract = Subtract(m1, m2);
		Matrix4x4 InverseM1 = Inverse(m1);
		Matrix4x4 InverseM2 = Inverse(m2);
		Matrix4x4 TransposeM1 = Transpose(m1);
		Matrix4x4 TransposeM2 = Transpose(m2);
		Matrix4x4 Identity = MakeIdentity4x4();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, resultAdd, "Add");
		MatrixScreenPrintf(0, kRowHeight * 5, resultSubtract, "Subtract");
		MatrixScreenPrintf(0, kRowHeight * 5 * 2, resultMultiply, "Multiply");
		MatrixScreenPrintf(0, kRowHeight * 5 * 3, InverseM1, "InverseM1");
		MatrixScreenPrintf(0, kRowHeight * 5 * 4, InverseM2, "InverseM2");
		MatrixScreenPrintf(kColumnWidth * 5, 0, TransposeM1, "TransposeM1");
		MatrixScreenPrintf(kColumnWidth * 5, kRowHeight * 5, TransposeM2, "TransposeM2");
		MatrixScreenPrintf(kColumnWidth * 5, kRowHeight * 5 * 2, Identity, "Identity");

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
