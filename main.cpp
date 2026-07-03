#include <Novice.h>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <imgui.h>
#include <algorithm> // std::min, std::max 用
#define _USE_MATH_DEFINES

const char kWindowTitle[] = "LC1C_12_ショウ_ズーウェン_OBB_Line";

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

struct Sphere
{
	Vector3 center;
	float radius;
};

struct Segment
{
	Vector3 origin;
	Vector3 diff;
};

// OBBの構造体定義
struct OBB
{
	Vector3 center;          // 中心点
	Vector3 orientations[3]; // 座標軸（方向単位ベクトル）[0]:X軸, [1]:Y軸, [2]:Z軸
	Vector3 size;            // 各軸の半分の長さ（拡縮）
};

// ベクトルの足し算
Vector3 Add(const Vector3& v1, const Vector3& v2)
{
	return Vector3{ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

// ベクトルの引き算
Vector3 Subtract(const Vector3& v1, const Vector3& v2)
{
	return Vector3{ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

// ベクトルのスカラー倍
Vector3 Multiply(float scalar, const Vector3& v)
{
	return Vector3{ scalar * v.x, scalar * v.y, scalar * v.z };
}

// 内積計算
float Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// 外積計算
Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
	return Vector3{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	};
}

// 逆行列計算
Matrix4x4 Inverse(const Matrix4x4& m)
{
	Matrix4x4 result = {};
	float a[4][8] = { 0 };
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			a[i][j] = m.m[i][j];
		}
		a[i][4 + i] = 1.0f;
	}
	for (int i = 0; i < 4; ++i) {
		float pivot = a[i][i];
		if (pivot == 0.0f) { continue; }
		for (int j = 0; j < 8; ++j) {
			a[i][j] /= pivot;
		}
		for (int k = 0; k < 4; ++k) {
			if (i != k) {
				float factor = a[k][i];
				for (int j = 0; j < 8; ++j) {
					a[k][j] -= factor * a[i][j];
				}
			}
		}
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = a[i][4 + j];
		}
	}
	return result;
}

// 座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix)
{
	Vector3 result;
	result.x = (vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0]);
	result.y = (vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1]);
	result.z = (vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2]);
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	if (w != 0.0f)
	{
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	return result;
}

// 行列の掛け算
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

Matrix4x4 identity() {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = (i == j) ? 1.0f : 0.0f;
		}
	}
	return result;
}

Matrix4x4 rotationX(float angle) {
	Matrix4x4 result = identity();
	result.m[0][0] = 1.0f;
	result.m[1][1] = cosf(angle);
	result.m[1][2] = sinf(angle);
	result.m[2][1] = -sinf(angle);
	result.m[2][2] = cosf(angle);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 rotationY(float angle) {
	Matrix4x4 result = identity();
	result.m[0][0] = cosf(angle);
	result.m[0][2] = -sinf(angle);
	result.m[1][1] = 1.0f;
	result.m[2][0] = sinf(angle);
	result.m[2][2] = cosf(angle);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 rotationZ(float angle) {
	Matrix4x4 result = identity();
	result.m[0][0] = cosf(angle);
	result.m[0][1] = sinf(angle);
	result.m[1][0] = -sinf(angle);
	result.m[1][1] = cosf(angle);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeTranslationMatrix(const Vector3& translation) {
	Matrix4x4 result = identity();
	result.m[3][0] = translation.x;
	result.m[3][1] = translation.y;
	result.m[3][2] = translation.z;
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translation) {
	Matrix4x4 scaleMatrix = { {
		{scale.x, 0.0f, 0.0f, 0.0f},
		{0.0f, scale.y, 0.0f, 0.0f},
		{0.0f, 0.0f, scale.z, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
		} };
	Matrix4x4 rotationXMatrix = rotationX(rotate.x);
	Matrix4x4 rotationYMatrix = rotationY(rotate.y);
	Matrix4x4 rotationZMatrix = rotationZ(rotate.z);
	Matrix4x4 rotationMatrix = Multiply(rotationXMatrix, Multiply(rotationYMatrix, rotationZMatrix));
	Matrix4x4 translationMatrix = MakeTranslationMatrix(translation);
	return Multiply(scaleMatrix, Multiply(rotationMatrix, translationMatrix));
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearClip, float farClip) {
	Matrix4x4 result = {};
	float f = 1.0f / tanf(fovY / 2.0f);
	result.m[0][0] = f / aspect;
	result.m[1][1] = f;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	return result;
}

Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;
	return result;
}

// 分離軸定理(SAT)に基づく、特定の軸におけるOBBの影の長さを計算するヘルパー関数
float CalculateProjectedRadius(const OBB& obb, const Vector3& axis)
{
	return std::abs(Dot(Multiply(obb.size.x, obb.orientations[0]), axis)) +
		std::abs(Dot(Multiply(obb.size.y, obb.orientations[1]), axis)) +
		std::abs(Dot(Multiply(obb.size.z, obb.orientations[2]), axis));
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivisions = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivisions);

	for (uint32_t xIndex = 0; xIndex <= kSubdivisions; ++xIndex)
	{
		float x = -kGridHalfWidth + (xIndex * kGridEvery);
		Vector3 startPos = { x, 0.0f, -kGridHalfWidth };
		Vector3 endPos = { x, 0.0f, kGridHalfWidth };
		Vector3 startScreen = Transform(Transform(startPos, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(endPos, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y), 0xAAAAAAFF);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivisions; ++zIndex)
	{
		float z = -kGridHalfWidth + (zIndex * kGridEvery);
		Vector3 startPos = { -kGridHalfWidth, 0.0f, z };
		Vector3 endPos = { kGridHalfWidth, 0.0f, z };
		Vector3 startScreen = Transform(Transform(startPos, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(endPos, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y), 0xAAAAAAFF);
	}
}

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivisions = 20;
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / (kSubdivisions);
	const float kLatEvery = std::numbers::pi_v<float> / (kSubdivisions);

	for (uint32_t latIndex = 0; latIndex < kSubdivisions; ++latIndex)
	{
		float lat = -std::numbers::pi_v<float> / 2.0f + latIndex * kLatEvery;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivisions; ++lonIndex)
		{
			float lon = lonIndex * kLonEvery;

			Vector3 a, b, c;
			a.x = sphere.center.x + sphere.radius * cosf(lat) * cosf(lon);
			a.y = sphere.center.y + sphere.radius * sinf(lat);
			a.z = sphere.center.z + sphere.radius * cosf(lat) * sinf(lon);

			b.x = sphere.center.x + sphere.radius * cosf(lat) * cosf(lon + kLonEvery);
			b.y = sphere.center.y + sphere.radius * sinf(lat);
			b.z = sphere.center.z + sphere.radius * cosf(lat) * sinf(lon + kLonEvery);

			c.x = sphere.center.x + sphere.radius * cosf(lat + kLatEvery) * cosf(lon);
			c.y = sphere.center.y + sphere.radius * sinf(lat + kLatEvery);
			c.z = sphere.center.z + sphere.radius * cosf(lat + kLatEvery) * sinf(lon);

			// ViewProjection行列でNDC座標に変換
			Vector3 aNdc = Transform(a, viewProjectionMatrix);
			Vector3 bNdc = Transform(b, viewProjectionMatrix);
			Vector3 cNdc = Transform(c, viewProjectionMatrix);

			// Viewport行列でスクリーン座標に変換
			Vector3 aScreen = Transform(aNdc, viewportMatrix);
			Vector3 bScreen = Transform(bNdc, viewportMatrix);
			Vector3 cScreen = Transform(cNdc, viewportMatrix);

			// 緯線（横方向）の描画
			Novice::DrawLine(
				int(aScreen.x), int(aScreen.y),
				int(bScreen.x), int(bScreen.y),
				color
			);

			// 経線（縦方向）の描画
			Novice::DrawLine(
				int(aScreen.x), int(aScreen.y),
				int(cScreen.x), int(cScreen.y),
				color
			);
		}
	}
}

// 実装に必要な線形補間関数
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t)
{
	return Add(Multiply(1.0f - t, v1), Multiply(t, v2));
}

// 2次ベジェ曲線の描画関数
void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
	const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	const int kSubdivisions = 32; // 曲線の分割数
	Vector3 previousScreenPos = {};

	for (int i = 0; i <= kSubdivisions; ++i)
	{
		float t = float(i) / float(kSubdivisions);

		// 制御点p0, p1を線形補間
		Vector3 p0p1 = Lerp(controlPoint0, controlPoint1, t);
		// 制御点p1, p2を線形補間
		Vector3 p1p2 = Lerp(controlPoint1, controlPoint2, t);
		// 補間点p0p1, p1p2をさらに線形補間
		Vector3 p = Lerp(p0p1, p1p2, t);

		// スクリーン座標に変換
		Vector3 ndc = Transform(p, viewProjectionMatrix);
		Vector3 screenPos = Transform(ndc, viewportMatrix);

		// 最初の点以外は前の点と線で結ぶ
		if (i > 0)
		{
			Novice::DrawLine(int(previousScreenPos.x), int(previousScreenPos.y), int(screenPos.x), int(screenPos.y), color);
		}
		previousScreenPos = screenPos;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	const int kWindowWidth = 1280;
	const int kWindowHeight = 720;
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// カメラ変数の初期化
	Vector3 cameraTranslate = { 0.0f, 2.5f, -10.0f };
	Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };

	// 実装例の初期値
	Vector3 transLates[3] = {
		{ 0.2f, 1.0f, 0.0f },
		{ 0.4f, 0.0f, 0.0f },
		{ 0.3f, 1.0f, 0.0f }
	};

	Vector3 rotates[3] = {
		{ 0.0f, 0.0f, -6.8f },
		{ 0.0f, 0.0f, -1.4f },
		{ 0.0f, 0.0f, 0.0f }
	};

	Vector3 scales[3] = {
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f }
	};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		float cameraSpeed = 0.05f;
		if (keys[DIK_W]) { cameraTranslate.z += cameraSpeed; }
		if (keys[DIK_S]) { cameraTranslate.z -= cameraSpeed; }
		if (keys[DIK_A]) { cameraTranslate.x -= cameraSpeed; }
		if (keys[DIK_D]) { cameraTranslate.x += cameraSpeed; }

		float rotateSpeed = 0.02f;
		if (keys[DIK_UP]) { cameraRotate.x += rotateSpeed; }
		if (keys[DIK_DOWN]) { cameraRotate.x -= rotateSpeed; }
		if (keys[DIK_LEFT]) { cameraRotate.y -= rotateSpeed; }
		if (keys[DIK_RIGHT]) { cameraRotate.y += rotateSpeed; }

		ImGui::Begin("Window");

		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);

		// ImGui設定 - コントロールポイントの調整
		ImGui::Separator();
		ImGui::Text("Control Points");

		ImGui::DragFloat3("transLates 0", &transLates[0].x, 0.01f);
		ImGui::DragFloat3("rotates 0", &rotates[0].x, 0.01f);
		ImGui::DragFloat3("scales 0", &scales[0].x, 0.01f);

		ImGui::DragFloat3("transLates 1", &transLates[1].x, 0.01f);
		ImGui::DragFloat3("rotates 1", &rotates[1].x, 0.01f);
		ImGui::DragFloat3("scales 1", &scales[1].x, 0.01f);

		ImGui::DragFloat3("transLates 2", &transLates[2].x, 0.01f);
		ImGui::DragFloat3("rotates 2", &rotates[2].x, 0.01f);
		ImGui::DragFloat3("scales 2", &scales[2].x, 0.01f);

		ImGui::End();

		// 各ノードのローカル行列（LocalMatrix）を計算
		Matrix4x4 localMatrix0 = MakeAffineMatrix(scales[0], rotates[0], transLates[0]);
		Matrix4x4 localMatrix1 = MakeAffineMatrix(scales[1], rotates[1], transLates[1]);
		Matrix4x4 localMatrix2 = MakeAffineMatrix(scales[2], rotates[2], transLates[2]);

		// 親子関係に基づきワールド行列（WorldMatrix）を計算
		// W_s = L_s (肩)
		// W_e = L_e * W_s (肘)
		// W_h = L_h * W_e (手)
		Matrix4x4 worldMatrix0 = localMatrix0;
		Matrix4x4 worldMatrix1 = Multiply(localMatrix1, worldMatrix0);
		Matrix4x4 worldMatrix2 = Multiply(localMatrix2, worldMatrix1);

		// 各関節のワールド空間上の中心座標を取得
		Vector3 position0 = { worldMatrix0.m[3][0], worldMatrix0.m[3][1], worldMatrix0.m[3][2] };
		Vector3 position1 = { worldMatrix1.m[3][0], worldMatrix1.m[3][1], worldMatrix1.m[3][2] };
		Vector3 position2 = { worldMatrix2.m[3][0], worldMatrix2.m[3][1], worldMatrix2.m[3][2] };

		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 各関節をそれぞれの色（肩: 赤, 肘: 緑, 手: 青）の球で描画 (半径 0.05f 程度が見やすいため調整)
		DrawSphere({ position0, 0.05f }, viewProjectionMatrix, viewportMatrix, 0xFF0000FF); // 赤
		DrawSphere({ position1, 0.05f }, viewProjectionMatrix, viewportMatrix, 0x00FF00FF); // 緑
		DrawSphere({ position2, 0.05f }, viewProjectionMatrix, viewportMatrix, 0x0000FFFF); // 青

		// 肩-肘、肘-手の間に線を引く
		Vector3 screenPos0 = Transform(Transform(position0, viewProjectionMatrix), viewportMatrix);
		Vector3 screenPos1 = Transform(Transform(position1, viewProjectionMatrix), viewportMatrix);
		Vector3 screenPos2 = Transform(Transform(position2, viewProjectionMatrix), viewportMatrix);

		Novice::DrawLine(int(screenPos0.x), int(screenPos0.y), int(screenPos1.x), int(screenPos1.y), 0xFFFFFFFF);
		Novice::DrawLine(int(screenPos1.x), int(screenPos1.y), int(screenPos2.x), int(screenPos2.y), 0xFFFFFFFF);

		///
		/// ↑描画処理ここまで
		///

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}