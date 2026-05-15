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
// 透視投影行列の作成
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearClip, float farClip) {
	Matrix4x4 result = {};
	float f = 1.0f / tanf(fovY / 2.0f);
	result.m[0][0] = f / aspect;
	result.m[1][1] = f;
	result.m[2][2] = (farClip + nearClip) / (nearClip - farClip);
	result.m[2][3] = (2.0f * farClip * nearClip) / (nearClip - farClip);
	result.m[3][2] = -1.0f;
	return result;
}
// 正射影行列の作成
Matrix4x4 MakeOrthographicMatrix(float left, float right, float top, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = -2.0f / (farClip - nearClip);
	result.m[3][0] = -(right + left) / (right - left);
	result.m[3][1] = -(top + bottom) / (top - bottom);
	result.m[3][2] = -(farClip + nearClip) / (farClip - nearClip);
	result.m[3][3] = 1.0f;
	return result;
}
// ビューポート変換行列の作成
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f; // Y軸を反転
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

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

static const int kRowHeight = 20;

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label)
{
	Novice::ScreenPrintf(x, y, "%s:", label);
	for (int i = 0; i < 4; ++i) {
		Novice::ScreenPrintf(x, y + (i + 1) * kRowHeight, "%.2f, %.2f, %.2f, %.2f",
			matrix.m[i][0], matrix.m[i][1], matrix.m[i][2], matrix.m[i][3]);
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Matrix4x4 orthographicMatrix = MakeOrthographicMatrix(-10.0f, 210.0f, 130.0f, 420.0f, 400.0f, 1000.0f);

	Matrix4x4 perspectiveMatrix = MakePerspectiveFovMatrix(3.14f / 4.0f, 1280.0f / 720.0f, 400.0f, 1000.0f);

	Matrix4x4 viewportMatrix = MakeViewportMatrix(120.0f, 230.0f, 1280.0f, 720.0f, 0.0f, 1.0f);

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

		MatrixScreenPrintf(0, 0, orthographicMatrix, "Orthographic Matrix");
		MatrixScreenPrintf(0, kRowHeight * 5, perspectiveMatrix, "Perspective Matrix");
		MatrixScreenPrintf(0, kRowHeight * 10, viewportMatrix, "Viewport Matrix");

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
