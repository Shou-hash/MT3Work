#include <Novice.h>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <imgui.h>
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

struct Line
{
	Vector3 origin;
	Vector3 diff;
};

struct Ray
{
	Vector3 origin;
	Vector3 diff;
};

struct Segment
{
	Vector3 origin;
	Vector3 diff;
};

struct Plane
{
	Vector3 normal;
	float distance;
};

// ★追加
struct Triangle
{
	Vector3 vertices[3];
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

Vector3 Project(const Vector3& v1, const Vector3& v2)
{
	// v2の長さの二乗（ドット積）
	float sqrMagV2 = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;

	// ゼロ除算の防止（v2がゼロベクトルの場合はゼロベクトルを返す）
	if (sqrMagV2 < 1e-6f)
	{
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}

	// v1 と v2 の内積
	float dot = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

	// 射影倍率を計算して v2 に掛ける
	float t = dot / sqrMagV2;
	return Vector3{ v2.x * t, v2.y * t, v2.z * t };
}

// クランプ用のヘルパー関数（標準関数の代わり、または std::clamp でも可）
float Clamp(float value, float min, float max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment)
{
	// segment.diff は「始点から終点へのベクトル(AB)」そのもの
	Vector3 ab = segment.diff;

	// 始点から対象の点へのベクトル(AP)
	Vector3 ap = Subtract(point, segment.origin);

	// ABの長さの二乗
	float sqrMagAB = ab.x * ab.x + ab.y * ab.y + ab.z * ab.z;

	// 始点と終点が同じ（点）ベクトルの場合は、始点を返す
	if (sqrMagAB < 1e-6f)
	{
		return segment.origin;
	}

	// 内積 (AP ・ AB)
	float dot = ap.x * ab.x + ap.y * ab.y + ap.z * ab.z;

	// 投影比率 t を計算し、0.0 〜 1.0 の間にクランプ
	float t = dot / sqrMagAB;
	t = Clamp(t, 0.0f, 1.0f);

	// 最近傍点を計算 (A + t * AB)
	return Add(segment.origin, Vector3{ ab.x * t, ab.y * t, ab.z * t });
}

Matrix4x4 Inverse(const Matrix4x4& m)
{
	Matrix4x4 result = {};
	float a[4][8] = { 0 };

	// 拡大係数行列の作成（左側に元の行列、右側に単位行列）
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			a[i][j] = m.m[i][j];
		}
		a[i][4 + i] = 1.0f;
	}

	// ガウス・ジョルダンの消去法（掃き出し法）
	for (int i = 0; i < 4; ++i) {
		float pivot = a[i][i];
		if (pivot == 0.0f) continue; // ゼロ除算回避（本来は行の入れ替えが必要ですが簡易化）

		// ピボット行をピボットで割る
		for (int j = 0; j < 8; ++j) {
			a[i][j] /= pivot;
		}

		// ピボット列の他の行を0にする
		for (int k = 0; k < 4; ++k) {
			if (i != k) {
				float factor = a[k][i];
				for (int j = 0; j < 8; ++j) {
					a[k][j] -= factor * a[i][j];
				}
			}
		}
	}

	// 右側の単位行列だった部分が逆行列になる
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

	// 平行移動（m[3][*]）を足し合わせ、最後にwで割る
	result.x = (vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0]);
	result.y = (vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1]);
	result.z = (vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2]);
	// w成分の計算（透視投影の除算に必要）
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
	Matrix4x4 result;
	// スケーリング行列の作成
	Matrix4x4 scaleMatrix = { {
		{scale.x, 0.0f, 0.0f, 0.0f},
		{0.0f, scale.y, 0.0f, 0.0f},
		{0.0f, 0.0f, scale.z, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
		} };
	// 回転行列の作成
	Matrix4x4 rotationXMatrix = rotationX(rotate.x);
	Matrix4x4 rotationYMatrix = rotationY(rotate.y);
	Matrix4x4 rotationZMatrix = rotationZ(rotate.z);
	Matrix4x4 rotationMatrix = Multiply(rotationXMatrix, Multiply(rotationYMatrix, rotationZMatrix));
	// 平行移動行列の作成
	Matrix4x4 translationMatrix = MakeTranslationMatrix(translation);
	// アフィン変換行列の計算
	result = Multiply(scaleMatrix, Multiply(rotationMatrix, translationMatrix));
	return result;
}

// 透視投影行列の作成
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

// 正射影行列の作成
Matrix4x4 MakeOrthographicMatrix(float left, float right, float top, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = {};
	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = -(right + left) / (right - left);
	result.m[3][1] = -(top + bottom) / (top - bottom);
	result.m[3][2] = -nearClip / (farClip - nearClip);
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

bool IsCollision(const Segment& segment, const Plane& plane)
{
	// 1. 平面の法線ベクトルと線分の方向ベクトルの内積を計算（垂直かどうかの判定用）
	float dot = plane.normal.x * segment.diff.x + plane.normal.y * segment.diff.y + plane.normal.z * segment.diff.z;

	// ゼロ除算の防止（内積がほぼゼロ = 平面と線分が平行な場合は交差しない）
	if (std::fabsf(dot) < 1e-6f)
	{
		return false;
	}

	// 2. 平面の方程式から交点までのパラメータ t を計算
	// t = (distance - (origin ・ normal)) / (diff ・ normal)
	float dotOrigin = segment.origin.x * plane.normal.x + segment.origin.y * plane.normal.y + segment.origin.z * plane.normal.z;
	float t = (plane.distance - dotOrigin) / dot;

	// 3. t が 0.0f 〜 1.0f の間であれば線分が平面と交差している
	if (t >= 0.0f && t <= 1.0f)
	{
		return true;
	}

	return false;
}

Vector3 Perpendicular(const Vector3& vector)
{
	if (vector.x != 0.0f || vector.y != 0.0f)
	{
		return { -vector.y , vector.x ,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

// ベクトルの実数倍（スライド内のMultiply用）
Vector3 PlaneMultiply(float scalar, const Vector3& v)
{
	return Vector3{ scalar * v.x, scalar * v.y, scalar * v.z };
}

// ベクトルの長さを計算
float Length(const Vector3& v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// ベクトルの正規化（長さを1にする）
Vector3 Normalize(const Vector3& v)
{
	float len = Length(v);
	if (len < 1e-6f)
	{
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}
	return Vector3{ v.x / len, v.y / len, v.z / len };
}

// クロス積（外積）の計算
Vector3 Cross(const Vector3& v1, const Vector3& v2)
{
	return Vector3{
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	};
}

// ドット積（内積）の計算（★追加）
float Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// ★追加：三角形と線分の衝突判定関数
bool IsCollision(const Triangle& triangle, const Segment& segment)
{
	// 三角形の各頂点からエッジベクトルを計算
	Vector3 v01 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 v02 = Subtract(triangle.vertices[2], triangle.vertices[0]);

	// 三角形の法線を計算して正規化
	Vector3 normal = Normalize(Cross(v01, v02));

	// 1. 三角形が乗っている平面と線分の交差判定を行う
	float distance = Dot(triangle.vertices[0], normal);
	float dot = Dot(normal, segment.diff);

	// 平面と線分が平行な場合は交差しない
	if (std::fabsf(dot) < 1e-6f)
	{
		return false;
	}

	// 交点までのパラメータ t を計算
	float dotOrigin = Dot(segment.origin, normal);
	float t = (distance - dotOrigin) / dot;

	// 線分の範囲外に交点がある場合は衝突していない
	if (t < 0.0f || t > 1.0f)
	{
		return false;
	}

	// 平面上の交点 p を計算
	Vector3 p = Add(segment.origin, Vector3{ segment.diff.x * t, segment.diff.y * t, segment.diff.z * t });

	// 各エッジベクトルと、対応する頂点から交点pへのベクトルを計算
	Vector3 v12 = Subtract(triangle.vertices[2], triangle.vertices[1]);
	Vector3 v20 = Subtract(triangle.vertices[0], triangle.vertices[2]);

	Vector3 v0p = Subtract(p, triangle.vertices[0]);
	Vector3 v1p = Subtract(p, triangle.vertices[1]);
	Vector3 v2p = Subtract(p, triangle.vertices[2]);

	// 各辺を結んだベクトルと、頂点と衝突点pを結んだベクトルのクロス積を取る
	Vector3 cross01 = Cross(v01, v0p);
	Vector3 cross12 = Cross(v12, v1p);
	Vector3 cross20 = Cross(v20, v2p);

	// すべての小三角形のクロス積と法線が同じ方向を向いていたら衝突
	if (Dot(cross01, normal) >= 0.0f &&
		Dot(cross12, normal) >= 0.0f &&
		Dot(cross20, normal) >= 0.0f) {
		// 衝突
		return true;
	}

	return false;
}

// ★追加：三角形を描画するDrawTriangle関数
void DrawTriangle(
	const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	Vector3 screenVertices[3];
	for (int i = 0; i < 3; ++i)
	{
		// 三角形の各頂点をスクリーン座標系まで変換してNovice::DrawTriangleを利用する
		Vector3 ndc = Transform(triangle.vertices[i], viewProjectionMatrix);
		screenVertices[i] = Transform(ndc, viewportMatrix);
	}

	Novice::DrawTriangle(
		int(screenVertices[0].x), int(screenVertices[0].y),
		int(screenVertices[1].x), int(screenVertices[1].y),
		int(screenVertices[2].x), int(screenVertices[2].y),
		color, kFillModeWireFrame
	);
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	// 1. 中心点を決める
	Vector3 center = PlaneMultiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];

	// 2. 法線と垂直なベクトルを1つ求め、正規化する
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));

	// 3. 2の逆ベクトルを求める
	perpendiculars[1] = Vector3{ -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z };

	// 4. 2と法線とのクロス積を求める
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);

	// 5. 4の逆ベクトルを求める
	perpendiculars[3] = Vector3{ -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z };

	// 6. 2〜5のベクトルを中心点にそれぞれ定数倍（ここでは2.0f）して足すと4頂点が出来上がる
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index)
	{
		Vector3 extend = PlaneMultiply(2.0f, perpendiculars[index]);
		Vector3 point = Add(center, extend);

		// 3D空間上の頂点をスクリーン座標に変換
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}

	// 各頂点を結んでDrawLineで矩形を描画する（0->2->1->3->0 の順で結ぶと綺麗な矩形枠になります）
	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
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

		Vector3 startNdc = Transform(startPos, viewProjectionMatrix);
		Vector3 endNdc = Transform(endPos, viewProjectionMatrix);

		Vector3 startScreen = Transform(startNdc, viewportMatrix);
		Vector3 endScreen = Transform(endNdc, viewportMatrix);

		Novice::DrawLine(
			int(startScreen.x), int(startScreen.y),
			int(endScreen.x), int(endScreen.y),
			0xAAAAAAFF
		);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivisions; ++zIndex)
	{
		float z = -kGridHalfWidth + (zIndex * kGridEvery);

		Vector3 startPos = { -kGridHalfWidth, 0.0f, z };
		Vector3 endPos = { kGridHalfWidth, 0.0f, z };

		Vector3 startNdc = Transform(startPos, viewProjectionMatrix);
		Vector3 endNdc = Transform(endPos, viewProjectionMatrix);

		Vector3 startScreen = Transform(startNdc, viewportMatrix);
		Vector3 endScreen = Transform(endNdc, viewportMatrix);

		Novice::DrawLine(
			int(startScreen.x), int(startScreen.y),
			int(endScreen.x), int(endScreen.y),
			0xAAAAAAFF
		);
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
	const int kWindowWidth = 1280;
	const int kWindowHeight = 720;
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// 変数の初期化（ループの外に出すことで毎フレーム初期化されるのを防ぐ）
	Vector3 cameraTranslate = { 0.0f, 2.5f, -10.0f };
	Vector3 cameraRotate = { 0.26f, 0.0f, 0.0f };
	Sphere sphere = { {0.0f, 0.0f, 0.0f}, 1.0f };

	Plane plane = { {0.0f, 1.0f, 0.0f}, 0.0f };

	// ★初期値設定（三角形）
	Triangle triangle{ {
		{ 0.0f,  1.0f, 0.0f },
		{ 1.0f, -0.5f, 0.0f },
		{-1.0f, -0.5f, 0.0f }
	} };

	// 線分の初期化（始点と差分）
	Segment segment{ { 0.0f, 0.5f, -2.0f }, { 0.0f, 0.0f, 4.0f } };
	Vector3 point{ -1.5f,0.6f,0.6f };

	Vector3 project = Project(Subtract(point, segment.origin), segment.diff);

	Vector3 closestPoint = ClosestPoint(point, segment);

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

		ImGui::Begin("Window");

		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);

		// ImGuiで線の始点と差分、三角形の各頂点を変更できるようにする
		ImGui::Separator();
		ImGui::Text("Triangle Vertices");
		ImGui::DragFloat3("Vertex0", &triangle.vertices[0].x, 0.01f);
		ImGui::DragFloat3("Vertex1", &triangle.vertices[1].x, 0.01f);
		ImGui::DragFloat3("Vertex2", &triangle.vertices[2].x, 0.01f);

		ImGui::Separator();
		ImGui::Text("Line Segment");
		ImGui::DragFloat3("Segment.Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment.Diff", &segment.diff.x, 0.01f);

		ImGui::End();

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

		// 三角形と線を表示し、衝突していたら線を赤く表示する。衝突していなければ白。
		uint32_t color = 0xFFFFFFFF; // 初期値は白色

		// 三角形と線分の衝突判定
		if (IsCollision(triangle, segment))
		{
			color = 0xFF0000FF; // 赤色
		}

		// 三角形を描画
		DrawTriangle(triangle, viewProjectionMatrix, viewportMatrix, color);

		// 線分（Segment）を画面に描画して視覚化する
		Vector3 startNdc = Transform(segment.origin, viewProjectionMatrix);
		Vector3 startScreen = Transform(startNdc, viewportMatrix);

		Vector3 endPos = Add(segment.origin, segment.diff);
		Vector3 endNdc = Transform(endPos, viewProjectionMatrix);
		Vector3 endScreen = Transform(endNdc, viewportMatrix);

		// 線分を描画（判定結果の色を適用）
		Novice::DrawLine(int(startScreen.x), int(startScreen.y), int(endScreen.x), int(endScreen.y), color);

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