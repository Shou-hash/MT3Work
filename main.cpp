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

// OBBと線分（Segment）の衝突判定
bool IsCollision(const Segment& segment, const OBB& obb)
{
	// OBBのワールド行列を作成
	Matrix4x4 obbWorld = { {
		{ obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0.0f },
		{ obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0.0f },
		{ obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0.0f },
		{ obb.center.x,          obb.center.y,          obb.center.z,          1.0f }
	} };

	// ローカル空間への逆行列
	Matrix4x4 obbInverse = Inverse(obbWorld);

	// 始点と終点をそれぞれローカル空間に変換
	Vector3 localOrigin = Transform(segment.origin, obbInverse);
	Vector3 localEnd = Transform(Add(segment.origin, segment.diff), obbInverse);

	// ローカル空間での線の方向ベクトル
	Vector3 localDiff = Subtract(localEnd, localOrigin);

	// OBBのサイズからローカル空間での境界（min, max）を直接決定
	Vector3 boxMin = { -obb.size.x, -obb.size.y, -obb.size.z };
	Vector3 boxMax = { +obb.size.x, +obb.size.y, +obb.size.z };

	float tNear = -std::numeric_limits<float>::infinity();
	float tFar = std::numeric_limits<float>::infinity();

	// X軸のスラブ判定
	if (std::abs(localDiff.x) < 1e-6f) {
		if (localOrigin.x < boxMin.x || localOrigin.x > boxMax.x)
		{
			return false;
		}
	}
	else {
		float t1 = (boxMin.x - localOrigin.x) / localDiff.x;
		float t2 = (boxMax.x - localOrigin.x) / localDiff.x;
		tNear = (std::max)(tNear, (std::min)(t1, t2));
		tFar = (std::min)(tFar, (std::max)(t1, t2));
	}

	// Y軸のスラブ判定
	if (std::abs(localDiff.y) < 1e-6f) {
		if (localOrigin.y < boxMin.y || localOrigin.y > boxMax.y)
		{
			return false;
		}
	}
	else {
		float t1 = (boxMin.y - localOrigin.y) / localDiff.y;
		float t2 = (boxMax.y - localOrigin.y) / localDiff.y;
		tNear = (std::max)(tNear, (std::min)(t1, t2));
		tFar = (std::min)(tFar, (std::max)(t1, t2));
	}

	// Z軸のスラブ判定
	if (std::abs(localDiff.z) < 1e-6f) {
		if (localOrigin.z < boxMin.z || localOrigin.z > boxMax.z)
		{
			return false;
		}
	}
	else {
		float t1 = (boxMin.z - localOrigin.z) / localDiff.z;
		float t2 = (boxMax.z - localOrigin.z) / localDiff.z;
		tNear = (std::max)(tNear, (std::min)(t1, t2));
		tFar = (std::min)(tFar, (std::max)(t1, t2));
	}

	// 衝突区間の整合性チェック
	return (tNear <= tFar && tNear <= 1.0f && tFar >= 0.0f);
}

// 分離軸定理(SAT)に基づく、特定の軸におけるOBBの影の長さを計算するヘルパー関数
float CalculateProjectedRadius(const OBB& obb, const Vector3& axis)
{
	return std::abs(Dot(Multiply(obb.size.x, obb.orientations[0]), axis)) +
		std::abs(Dot(Multiply(obb.size.y, obb.orientations[1]), axis)) +
		std::abs(Dot(Multiply(obb.size.z, obb.orientations[2]), axis));
}

// OBBとOBBの衝突判定関数（分離軸定理を用いる）
bool IsCollision(const OBB& obb1, const OBB& obb2)
{
	// 15本の分離軸候補をリストアップ
	Vector3 axes[15];

	// 各OBBの面法線（計6本）
	axes[0] = obb1.orientations[0];
	axes[1] = obb1.orientations[1];
	axes[2] = obb1.orientations[2];
	axes[3] = obb2.orientations[0];
	axes[4] = obb2.orientations[1];
	axes[5] = obb2.orientations[2];

	// 各辺の組み合わせのクロス積（計9本）
	int index = 6;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			axes[index++] = Cross(obb1.orientations[i], obb2.orientations[j]);
		}
	}

	// 2つのOBBの中心間を結ぶベクトル
	Vector3 centerDir = Subtract(obb2.center, obb1.center);

	// すべての候補軸に対して分離しているかをテスト
	for (int i = 0; i < 15; ++i) {
		Vector3 axis = axes[i];

		// クロス積が並行になり長さが0になった軸はスキップ
		float axisLengthSq = Dot(axis, axis);
		if (axisLengthSq < 1e-6f) {
			continue;
		}

		// 単位ベクトル化
		float axisLength = std::sqrt(axisLengthSq);
		axis = Vector3{ axis.x / axisLength, axis.y / axisLength, axis.z / axisLength };

		// それぞれのOBBを軸に射影した影の長さを算出（L1, L2）
		float L1 = CalculateProjectedRadius(obb1, axis);
		float L2 = CalculateProjectedRadius(obb2, axis);

		// 影の長さの合計（sumSpan）
		float sumSpan = L1 + L2;

		// 2つの影の両端の差分（longSpan相当：中心間距離の射影成分）
		float centerDistProj = std::abs(Dot(centerDir, axis));

		// 1つでも分離しているなら衝突していない
		if (sumSpan < centerDistProj) {
			return false; // 分離している（隙間がある）ので衝突していない
		}
	}

	// どの候補軸も分離していないなら衝突している
	return true;
}

// OBBの描画関数
void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
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
		Vector3 worldPos = obb.center;
		worldPos = Add(worldPos, Multiply(localVertices[i].x, obb.orientations[0]));
		worldPos = Add(worldPos, Multiply(localVertices[i].y, obb.orientations[1]));
		worldPos = Add(worldPos, Multiply(localVertices[i].z, obb.orientations[2]));

		Vector3 ndc = Transform(worldPos, viewProjectionMatrix);
		screenVertices[i] = Transform(ndc, viewportMatrix);
	}

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

// 線分（Segment）の描画関数
void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	Vector3 start = segment.origin;
	Vector3 end = Add(segment.origin, segment.diff);

	Vector3 startScreen = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
	Vector3 endScreen = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

	Novice::DrawLine(int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y), color);
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

	// OBB1の初期設定（スライドの初期値）
	OBB obb1;
	obb1.center = { 0.0f, 0.0f, 0.0f };
	obb1.size = { 0.83f, 0.26f, 0.24f };
	obb1.orientations[0] = { 1.0f, 0.0f, 0.0f };
	obb1.orientations[1] = { 0.0f, 1.0f, 0.0f };
	obb1.orientations[2] = { 0.0f, 0.0f, 1.0f };

	// OBB2の初期設定（スライドの初期値）
	OBB obb2;
	obb2.center = { 0.9f, 0.66f, 0.78f };
	obb2.size = { 0.5f, 0.37f, 0.5f };
	obb2.orientations[0] = { 1.0f, 0.0f, 0.0f };
	obb2.orientations[1] = { 0.0f, 1.0f, 0.0f };
	obb2.orientations[2] = { 0.0f, 0.0f, 1.0f };

	// ImGui用の各OBB回転角度（オイラー角）
	Vector3 rotate1 = { 0.0f, 0.0f, 0.0f };
	Vector3 rotate2 = { -0.05f, -2.49f, 0.15f };

	// 線分（Segment）の初期設定
	Segment segment;
	segment.origin = { -0.8f, -0.3f, 0.0f };
	segment.diff = { 0.5f, 0.5f, 0.5f };

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

		// ImGui設定 - OBB1
		ImGui::Separator();
		ImGui::Text("OBB1");
		ImGui::DragFloat3("OBB1 Center", &obb1.center.x, 0.01f);
		ImGui::DragFloat3("OBB1 Size", &obb1.size.x, 0.01f);
		ImGui::DragFloat3("OBB1 Rotate (Rad)", &rotate1.x, 0.01f);

		// ImGui設定 - OBB2
		ImGui::Separator();
		ImGui::Text("OBB2");
		ImGui::DragFloat3("OBB2 Center", &obb2.center.x, 0.01f);
		ImGui::DragFloat3("OBB2 Size", &obb2.size.x, 0.01f);
		ImGui::DragFloat3("OBB2 Rotate (Rad)", &rotate2.x, 0.01f);

		// ImGui設定 - Segment
		ImGui::Separator();
		ImGui::Text("Segment");
		ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.01f);

		ImGui::End();

		// OBB1の回転と軸更新
		Matrix4x4 obb1RotMat = Multiply(rotationX(rotate1.x), Multiply(rotationY(rotate1.y), rotationZ(rotate1.z)));
		obb1.orientations[0] = { obb1RotMat.m[0][0], obb1RotMat.m[0][1], obb1RotMat.m[0][2] };
		obb1.orientations[1] = { obb1RotMat.m[1][0], obb1RotMat.m[1][1], obb1RotMat.m[1][2] };
		obb1.orientations[2] = { obb1RotMat.m[2][0], obb1RotMat.m[2][1], obb1RotMat.m[2][2] };

		// OBB2の回転と軸更新
		Matrix4x4 obb2RotMat = Multiply(rotationX(rotate2.x), Multiply(rotationY(rotate2.y), rotationZ(rotate2.z)));
		obb2.orientations[0] = { obb2RotMat.m[0][0], obb2RotMat.m[0][1], obb2RotMat.m[0][2] };
		obb2.orientations[1] = { obb2RotMat.m[1][0], obb2RotMat.m[1][1], obb2RotMat.m[1][2] };
		obb2.orientations[2] = { obb2RotMat.m[2][0], obb2RotMat.m[2][1], obb2RotMat.m[2][2] };

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

		// 衝突状態によって色を変える
		uint32_t obb1Color = 0xFFFFFFFF; // 通常時：白
		uint32_t obb2Color = 0xFFFFFFFF; // 通常時：白

		// OBB1とOBB2の衝突判定
		if (IsCollision(obb1, obb2))
		{
			obb1Color = 0xFF0000FF; // 衝突時：赤
			obb2Color = 0xFF0000FF; // 衝突時：赤
		}

		// 描画
		DrawOBB(obb1, viewProjectionMatrix, viewportMatrix, obb1Color);
		DrawOBB(obb2, viewProjectionMatrix, viewportMatrix, obb2Color);

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