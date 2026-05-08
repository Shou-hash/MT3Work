#include <Novice.h>

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

Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3& scale)
{
	Matrix4x4 result = {};
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;
	return result;
}

Vector3 Transform(const Vector3& normal, const Matrix4x4& matrix)
{
	Vector3 result;
	result.x = normal.x * matrix.m[0][0] + normal.y * matrix.m[1][0] + normal.z * matrix.m[2][0];
	result.y = normal.x * matrix.m[0][1] + normal.y * matrix.m[1][1] + normal.z * matrix.m[2][1];
	result.z = normal.x * matrix.m[0][2] + normal.y * matrix.m[1][2] + normal.z * matrix.m[2][2];
	return result;
}

static const int kRowHeight = 20;

void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label)
{
	Novice::ScreenPrintf(x, y, "%s: (%.2f, %.2f, %.2f)", label, vector.x, vector.y, vector.z);
}

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
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Vector3 translate = { 4.1f, 2.6f, 0.8f };
	Vector3 scale = { 1.5f, 5.2f, 7.3f };
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Vector3 point = { 2.3f, 3.8f, 1.4f };
	Matrix4x4 transformMatrix = {
		1.0f,2.0f,3.0f,4.0f,
		3.0f,1.0f,1.0f,2.0f,
		1.0f,4.0f,2.0f,3.0f,
		2.0f,2.0f,1.0f,3.0f
	};

	Vector3 transformedPoint = Transform(point, transformMatrix);

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

		VectorScreenPrintf(0, 0, transformedPoint, "transformedPoint");
		MatrixScreenPrintf(0, kRowHeight, transformMatrix, "transformMatrix");
		MatrixScreenPrintf(0, kRowHeight * 6, scaleMatrix, "scaleMatrix");

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
