#include <Novice.h>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <imgui.h>
#include <algorithm> // std::min, std::max, std::clamp 用
#define _USE_MATH_DEFINES

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

struct AABB
{
	Vector3 min;
	Vector3 max;
};

// 【新規追加】OBBの構造体定義
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

// 内積
float Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// ベクトルの長さを計算
float Length(const Vector3& v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

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

// 【新規追加】OBBと球の衝突判定関数
bool IsCollision(const OBB& obb, const Sphere& sphere)
{
	// OBBの中心から球の中心へのベクトル
	Vector3 centerInOBBLocal = Subtract(sphere.center, obb.center);

	// OBBの各局所軸に投影して、ローカル空間上での座標を取り出す
	Vector3 closestPointInLocal{
		Dot(centerInOBBLocal, obb.orientations[0]),
		Dot(centerInOBBLocal, obb.orientations[1]),
		Dot(centerInOBBLocal, obb.orientations[2])
	};

	// ローカル空間上でAABBと同じように[-size, size]の範囲にクランプする
	closestPointInLocal.x = std::clamp(closestPointInLocal.x, -obb.size.x, obb.size.x);
	closestPointInLocal.y = std::clamp(closestPointInLocal.y, -obb.size.y, obb.size.y);
	closestPointInLocal.z = std::clamp(closestPointInLocal.z, -obb.size.z, obb.size.z);

	// クランプしたローカル座標を、もう一度ワールド空間の座標系に戻す
	Vector3 closestPointWorld = obb.center;
	closestPointWorld = Add(closestPointWorld, Multiply(closestPointInLocal.x, obb.orientations[0]));
	closestPointWorld = Add(closestPointWorld, Multiply(closestPointInLocal.y, obb.orientations[1]));
	closestPointWorld = Add(closestPointWorld, Multiply(closestPointInLocal.z, obb.orientations[2]));

	// ワールド空間での最近接点と球の中心との距離を測定
	float distance = Length(Subtract(closestPointWorld, sphere.center));

	// 半径以下なら衝突
	return distance <= sphere.radius;
}

// 【新規追加】OBBの描画関数
void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	// ローカル空間での8頂点
	Vector3 localVertices[8] = {
		{ -obb.size.x, -obb.size.y, -obb.size.z },
		{  obb.size.x, -obb.size.y, -obb.size.z },
		{ -obb.size.x,  obb.size.y, -obb.size.z },
		{  obb.size.x,  obb.size.y, -obb.size.z },
		{ -obb.size.x, -obb.size.y,  obb.size.z },
		{  obb.size.x, -obb.size.y,  obb.size.z },
		{ -obb.size.x,  obb.size.y,  obb.size.z },
		{  obb.size.x,  obb.size.y,  obb.size.z }
	};

	Vector3 screenVertices[8];
	for (int i = 0; i < 8; ++i)
	{
		// ローカルからワールド空間へ変換
		Vector3 worldPos = obb.center;
		worldPos = Add(worldPos, Multiply(localVertices[i].x, obb.orientations[0]));
		worldPos = Add(worldPos, Multiply(localVertices[i].y, obb.orientations[1]));
		worldPos = Add(worldPos, Multiply(localVertices[i].z, obb.orientations[2]));

		Vector3 ndc = Transform(worldPos, viewProjectionMatrix);
		screenVertices[i] = Transform(ndc, viewportMatrix);
	}

	// 各辺をつなぐ
	Novice::DrawLine(int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y), color);
	Novice::DrawLine(int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[3].x), int(screenVertices[3].y), color);
	Novice::DrawLine(int(screenVertices[3].x), int(screenVertices[3].y), int(screenVertices[2].x), int(screenVertices[2].y), color);
	Novice::DrawLine(int(screenVertices[2].x), int(screenVertices[2].y), int(screenVertices[0].x), int(screenVertices[0].y), color);

	Novice::DrawLine(int(screenVertices[4].x), int(screenVertices[4].y), int(screenVertices[5].x), int(screenVertices[5].y), color);
	Novice::DrawLine(int(screenVertices[5].x), int(screenVertices[5].y), int(screenVertices[7].x), int(screenVertices[7].y), color);
	Novice::DrawLine(int(screenVertices[7].x), int(screenVertices[7].y), int(screenVertices[6].x), int(screenVertices[6].y), color);
	Novice::DrawLine(int(screenVertices[6].x), int(screenVertices[6].y), int(screenVertices[4].x), int(screenVertices[4].y), color);

	Novice::DrawLine(int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[4].x), int(screenVertices[4].y), color);
	Novice::DrawLine(int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[5].x), int(screenVertices[5].y), color);
	Novice::DrawLine(int(screenVertices[2].x), int(screenVertices[2].y), int(screenVertices[6].x), int(screenVertices[6].y), color);
	Novice::DrawLine(int(screenVertices[3].x), int(screenVertices[3].y), int(screenVertices[7].x), int(screenVertices[7].y), color);
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

constexpr float kPi = std::numbers::pi_v<float>;

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivisions = 20;
	const float kLonEvery = 2.0f * kPi / (kSubdivisions);
	const float kLatEvery = kPi / (kSubdivisions);

	for (uint32_t latIndex = 0; latIndex < kSubdivisions; ++latIndex)
	{
		float lat = -kPi / 2.0f + latIndex * kLatEvery;

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

			Vector3 aScreen = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 bScreen = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 cScreen = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

			Novice::DrawLine(int(aScreen.x), int(aScreen.y), int(bScreen.x), int(bScreen.y), color);
			Novice::DrawLine(int(aScreen.x), int(aScreen.y), int(cScreen.x), int(cScreen.y), color);
		}
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

	// 球の初期設定
	Sphere sphere = { {0.0f, 0.0f, 2.0f}, 0.5f };

	// OBBの初期設定
	OBB obb;
	obb.center = { 0.0f, 0.0f, 0.0f };
	obb.size = { 0.5f, 0.5f, 0.5f };
	// 方向ベクトルの初期値（単位行列と同じ状態）
	obb.orientations[0] = { 1.0f, 0.0f, 0.0f };
	obb.orientations[1] = { 0.0f, 1.0f, 0.0f };
	obb.orientations[2] = { 0.0f, 0.0f, 1.0f };

	// ImGui用のOBB回転角度（オイラー角）
	Vector3 obbRotate = { 0.0f, 0.0f, 0.0f };

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

		// ImGuiでOBBと球のパラメータを変更できるようにする
		ImGui::Separator();
		ImGui::Text("OBB");
		ImGui::DragFloat3("OBB Center", &obb.center.x, 0.01f);
		ImGui::DragFloat3("OBB Size", &obb.size.x, 0.01f);
		// 【要望対応】ImGuiで角度を変更できるようにコントロールを追加
		ImGui::DragFloat3("OBB Rotate (Rad)", &obbRotate.x, 0.01f);

		ImGui::Separator();
		ImGui::Text("Sphere");
		ImGui::DragFloat3("Sphere Center", &sphere.center.x, 0.01f);
		ImGui::DragFloat("Sphere Radius", &sphere.radius, 0.01f);

		ImGui::End();

		// 入力された角度情報（obbRotate）から回転行列を作成し、OBBの方向ベクトル（軸）を更新する
		Matrix4x4 obbRotMat = Multiply(rotationX(obbRotate.x), Multiply(rotationY(obbRotate.y), rotationZ(obbRotate.z)));
		obb.orientations[0] = { obbRotMat.m[0][0], obbRotMat.m[0][1], obbRotMat.m[0][2] }; // X軸の向き
		obb.orientations[1] = { obbRotMat.m[1][0], obbRotMat.m[1][1], obbRotMat.m[1][2] }; // Y軸の向き
		obb.orientations[2] = { obbRotMat.m[2][0], obbRotMat.m[2][1], obbRotMat.m[2][2] }; // Z軸の向き

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

		// 衝突状態によって色を変える処理
		uint32_t color = 0xFFFFFFFF; // 通常時は白色
		if (IsCollision(obb, sphere))
		{
			color = 0xFF0000FF; // 衝突時は赤色
		}

		// OBBと球を描画する
		DrawOBB(obb, viewProjectionMatrix, viewportMatrix, color);
		DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, 0xFFFFFFFF);

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