#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "LC1C_12_ショウ_ズーウェン";

struct Vector3
{
	float x;
	float y;
	float z;
};

struct Matrix4x4
{
	float m[4][4];
};

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
	Matrix4x4 r = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k) {
				sum += a.m[i][k] * b.m[k][j];
			}
			r.m[i][j] = sum;
		}
	}
	return r;
}

Matrix4x4 MakeRotateXMatrix(float radian)
{
	Matrix4x4 rotateX = {};
	rotateX.m[0][0] = 1.0f;
	rotateX.m[1][1] = cosf(radian);
	rotateX.m[1][2] = -sinf(radian);
	rotateX.m[2][1] = sinf(radian);
	rotateX.m[2][2] = cosf(radian);
	rotateX.m[3][3] = 1.0f;
	return rotateX;
}

Matrix4x4 MakeRotateYMatrix(float radian)
{
	Matrix4x4 rotateY = {};
	rotateY.m[0][0] = cosf(radian);
	rotateY.m[0][2] = sinf(radian);
	rotateY.m[1][1] = 1.0f;
	rotateY.m[2][0] = -sinf(radian);
	rotateY.m[2][2] = cosf(radian);
	rotateY.m[3][3] = 1.0f;
	return rotateY;
}

Matrix4x4 MakeRotateZMatrix(float radian)
{
	Matrix4x4 rotateZ = {};
	rotateZ.m[0][0] = cosf(radian);
	rotateZ.m[0][1] = -sinf(radian);
	rotateZ.m[1][0] = sinf(radian);
	rotateZ.m[1][1] = cosf(radian);
	rotateZ.m[2][2] = 1.0f;
	rotateZ.m[3][3] = 1.0f;
	return rotateZ;
}

static const int kRowHeight = 20;

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label)
{
	Novice::ScreenPrintf(x, y, "%s:", label);
	for (int i = 0; i < 4; ++i) {
		Novice::ScreenPrintf(x, y + (i + 1) * 20, "%d: %.2f, %.2f, %.2f, %.2f",
			i, matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 rotate = { 4.0f, 1.0f, 2.0f };
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));


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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		MatrixScreenPrintf(0, 0, rotateXMatrix, "rotateXMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5, rotateYMatrix, "rotateYMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5 * 2, rotateZMatrix, "rotateZMatrix");
		MatrixScreenPrintf(0, kRowHeight * 5 * 3, rotateXYZMatrix, "rotateXYZMatrix");

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
