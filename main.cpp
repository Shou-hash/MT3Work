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

	// 複合代入演算子（メンバ関数として定義）
	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
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

// カプセル構造体
struct Capsule {
	Segment segment;
	float radius;
};

// 平面構造体
struct Plane {
	Vector3 normal;
	float distance;
};

struct Ball {
	Vector3 position;     // ボールの位置
	Vector3 velocity;     // ボールの速度
	Vector3 acceleration; // ボールの加速度
	float mass;           // ボールの質量
	float radius;         // ボールの半径
	unsigned int color;   // ボールの色
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

// ベクトルの長さを計算
float Length(const Vector3& v)
{
	return std::sqrt(Dot(v, v));
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

// --- 演算子オーバーロードの定義（グローバル関数） ---

// 二項演算子 (Vector3)
Vector3 operator+(const Vector3& v1, const Vector3& v2) { return Add(v1, v2); }
Vector3 operator-(const Vector3& v1, const Vector3& v2) { return Subtract(v1, v2); }
Vector3 operator*(float s, const Vector3& v) { return Multiply(s, v); }
Vector3 operator*(const Vector3& v, float s) { return s * v; }
Vector3 operator/(const Vector3& v, float s) { return Multiply(1.0f / s, v); }

// ベクトルの正規化
Vector3 Normalize(const Vector3& v)
{
	float len = Length(v);
	if (len != 0.0f)
	{
		return v / len;
	}
	return v;
}

// 二項演算子 (Matrix4x4)
Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}
	return result;
}
Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}
	return result;
}
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2) { return Multiply(m1, m2); }

// 単項演算子 (Vector3)
Vector3 operator-(const Vector3& v) { return { -v.x, -v.y, -v.z }; }
Vector3 operator+(const Vector3& v) { return v; }

// --------------------------------------------------

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
	Matrix4x4 rotationMatrix = rotationXMatrix * rotationYMatrix * rotationZMatrix; // 演算子オーバーロードを使用
	Matrix4x4 translationMatrix = MakeTranslationMatrix(translation);
	return scaleMatrix * rotationMatrix * translationMatrix; // 演算子オーバーロードを使用
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

// 反射ベクトルを求める関数: r = i - 2(i・n)n
Vector3 Reflect(const Vector3& input, const Vector3& normal) {
	return input - 2.0f * Dot(input, normal) * normal;
}

// 射影ベクトルを求める関数（法線方向への射影）
Vector3 Project(const Vector3& v, const Vector3& normal) {
	return Dot(v, normal) * normal;
}

// 線形補間
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return (1.0f - t) * v1 + t * v2;
}

// グリッド描画
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivisions = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivisions);

	for (uint32_t xIndex = 0; xIndex <= kSubdivisions; ++xIndex) {
		float x = -kGridHalfWidth + (xIndex * kGridEvery);
		Vector3 startPos = { x, 0.0f, -kGridHalfWidth };
		Vector3 endPos = { x, 0.0f, kGridHalfWidth };
		Vector3 startScreen = Transform(Transform(startPos, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(endPos, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y), 0xAAAAAAFF);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivisions; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);
		Vector3 startPos = { -kGridHalfWidth, 0.0f, z };
		Vector3 endPos = { kGridHalfWidth, 0.0f, z };
		Vector3 startScreen = Transform(Transform(startPos, viewProjectionMatrix), viewportMatrix);
		Vector3 endScreen = Transform(Transform(endPos, viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y), 0xAAAAAAFF);
	}
}

// 球描画
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivisions = 16;
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / (kSubdivisions);
	const float kLatEvery = std::numbers::pi_v<float> / (kSubdivisions);

	for (uint32_t latIndex = 0; latIndex < kSubdivisions; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + latIndex * kLatEvery;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivisions; ++lonIndex) {
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

// 傾斜平面描画用の4つの角を計算してワイヤーフレームを描画
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 平面の法線から適当なローカル座標系軸を作る
	Vector3 u = { 1.0f, 0.0f, 0.0f };
	if (std::abs(Dot(plane.normal, u)) > 0.99f) {
		u = { 0.0f, 0.0f, 1.0f };
	}
	Vector3 r = Normalize(Cross(plane.normal, u));
	Vector3 f = Normalize(Cross(r, plane.normal));

	// 平面上の中心点
	Vector3 center = plane.normal * plane.distance;

	// 平面の描画サイズ
	float halfSize = 2.0f;
	Vector3 p0 = center + r * halfSize + f * halfSize;
	Vector3 p1 = center - r * halfSize + f * halfSize;
	Vector3 p2 = center - r * halfSize - f * halfSize;
	Vector3 p3 = center + r * halfSize - f * halfSize;

	Vector3 s0 = Transform(Transform(p0, viewProjectionMatrix), viewportMatrix);
	Vector3 s1 = Transform(Transform(p1, viewProjectionMatrix), viewportMatrix);
	Vector3 s2 = Transform(Transform(p2, viewProjectionMatrix), viewportMatrix);
	Vector3 s3 = Transform(Transform(p3, viewProjectionMatrix), viewportMatrix);

	Novice::DrawLine(int(s0.x), int(s0.y), int(s1.x), int(s1.y), color);
	Novice::DrawLine(int(s1.x), int(s1.y), int(s2.x), int(s2.y), color);
	Novice::DrawLine(int(s2.x), int(s2.y), int(s3.x), int(s3.y), color);
	Novice::DrawLine(int(s3.x), int(s3.y), int(s0.x), int(s0.y), color);
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

	// 平面の初期化
	Plane plane;
	plane.normal = Normalize({ -0.2f, 0.9f, -0.3f });
	plane.distance = 0.0f;

	// ImGui操作用の一時角度変数（ラジアン）
	float planePitch = 0.0f; // X軸周りの回転
	float planeRoll = 0.0f;  // Z軸周りの回転

	// ボールの初期化
	Ball ball{};
	ball.position = { 0.8f, 2.5f, 0.3f };
	ball.velocity = { 0.0f, 0.0f, 0.0f };
	ball.acceleration = { 0.0f, -9.8f, 0.0f };
	ball.mass = 2.0f;
	ball.radius = 0.08f;
	ball.color = 0xFFFFFFFF; // WHITE

	// 反発係数 e
	float restitution = 0.6f;

	// アプリケーションが開始されたかどうかのフラグ
	bool isStarted = false;

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

		// deltaTimeの導入
		float deltaTime = 1.0f / 60.0f;

		// 角度から平面の法線を生成
		Matrix4x4 planeRotMatrix = rotationX(planePitch) * rotationZ(planeRoll);
		plane.normal = Transform({ 0.0f, 1.0f, 0.0f }, planeRotMatrix); // 初期法線(0,1,0)を回転

		// 2. 物理演算アップデート
		if (isStarted) {
			// 移動前の位置を保存
			Vector3 previousPosition = ball.position;

			// 重力による速度と位置の更新
			ball.velocity += ball.acceleration * deltaTime;
			ball.position += ball.velocity * deltaTime;

			// スイープ（カプセル）による平面衝突判定と埋まり戻し
			// ボールの移動軌跡（Segment）を作成
			Segment movementSegment;
			movementSegment.origin = previousPosition;
			movementSegment.diff = ball.position - previousPosition;

			// 平面と前後の位置関係（各点から平面への最短距離）を調べる
			float distPrev = Dot(previousPosition, plane.normal) - plane.distance;
			float distCurr = Dot(ball.position, plane.normal) - plane.distance;

			// 前フレームで平面の上（法線側）にいて、現フレームで平面の下に突き抜けた場合、衝突とみなす
			if (distPrev >= ball.radius && distCurr < ball.radius) {

				// 1. 衝突が起こった瞬間（ちょうど距離が radius になる瞬間）のパラメータ t を算出する
				// dist(t) = distPrev + t * (distCurr - distPrev) = radius
				float t = 0.0f;
				float denominator = distPrev - distCurr;
				if (std::abs(denominator) > 0.0001f) {
					t = (distPrev - ball.radius) / denominator;
				}
				t = std::clamp(t, 0.0f, 1.0f);

				// 衝突時の位置にボールを戻す（すり抜け・埋まりの防止）
				ball.position = Lerp(previousPosition, ball.position, t);

				// 2. 反射ベクトルの計算
				Vector3 reflected = Reflect(ball.velocity, plane.normal);

				// 3. 反発係数による減衰を法線方向だけに適用
				Vector3 projectToNormal = Project(reflected, plane.normal);
				Vector3 movingDirection = reflected - projectToNormal;

				// 法線方向の速度成分のみ反発係数 e を掛け、接線方向はそのままにする
				ball.velocity = projectToNormal * restitution + movingDirection;
			}
		}

		ImGui::Begin("Window");

		if (ImGui::Button("Reset/Start")) {
			// 初期位置と初速をリセット
			ball.position = { 0.8f, 2.5f, 0.3f };
			ball.velocity = { 0.0f, 0.0f, 0.0f };
			isStarted = true;
		}

		ImGui::Separator();
		ImGui::Text("Plane Settings");
		// スライダーで平面の角度を変更
		ImGui::SliderAngle("Plane Pitch (X-axis)", &planePitch, -45.0f, 45.0f);
		ImGui::SliderAngle("Plane Roll  (Z-axis)", &planeRoll, -45.0f, 45.0f);
		ImGui::SliderFloat("Plane Distance", &plane.distance, -1.0f, 1.0f);

		ImGui::Separator();
		ImGui::Text("Physics Settings");
		ImGui::SliderFloat("Restitution (e)", &restitution, 0.0f, 1.0f);

		ImGui::End();

		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix; // 演算子オーバーロードを使用
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// 傾斜平面を描画
		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, 0xFF8000FF); // 橙色/ピンク

		// ボール（球体）を描画
		Sphere ballSphere = { ball.position, ball.radius };
		DrawSphere(ballSphere, viewProjectionMatrix, viewportMatrix, ball.color);

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