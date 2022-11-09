#include <glew.h>
#include "CameraActor.h"
#include "TransformComponent.h"
#include "MMath.h"
#include "QMath.h"
#include "Debug.h"
#include "UBO_Padding.h"

CameraActor::CameraActor(Component* parent_):Actor(parent_) {
	prehab = false;
	uboMatricesID = NULL;
}

CameraActor::~CameraActor() {
	OnDestroy();
}

bool CameraActor::OnCreate() {
	if (isCreated) return isCreated; //return true
	Debug::Info("Creating values for CameraActor: ", __FILE__, __LINE__);
	//buffer
	size_t bufferSize = 2 * UBO_PADDING::MAT4; //*2 because we want both projectionMatrix & viewMatrix
	glGenBuffers(1, &uboMatricesID); //generate a buffer
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatricesID); //I'm talking to you uboMatricesID
	glBufferData(GL_UNIFORM_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW); //generate the size, allocate memory - GL_STATIC_DRAW is one way to the gpu
	glBindBuffer(GL_UNIFORM_BUFFER, 0); //I'm no longer talking to uboMatricesID - every ubo has a unique ID, 0 is nothing
	//binding - https://www.khronos.org/opengl/wiki/Uniform_Buffer_Object
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uboMatricesID); //giving it a binding point, so it can be found later by gpu

	UpdateProjectionMatrix(45.0f, (16.0f / 9.0f), 0.5f, 100.0f);
	UpdateViewMatrix();
	isCreated = true;
	return isCreated;
}

void CameraActor::UpdateProjectionMatrix(const float fovy, const float aspectRatio, const float near, const float far) {
	projectionMatrix = MMath::perspective(fovy, aspectRatio, near, far);

	CalculateFrustumPlanes();

	//buffer update
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatricesID); //I'm talking to you uboMatricesID
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4), projectionMatrix); //offset = 0 because it is the start of the buffer
	glBindBuffer(GL_UNIFORM_BUFFER, 0); //unbind buffer
}

void CameraActor::UpdateViewMatrix() {
	Ref<TransformComponent> transformComponent = GetComponent<TransformComponent>();
	if (transformComponent == nullptr) {
		viewMatrix = MMath::lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
	}
	else {
		Quaternion orientation = transformComponent->GetQuaternion();
		Vec3 position = transformComponent->GetPosition();
		viewMatrix = MMath::translate(position) * QMath::toMatrix4(orientation);
	}

	CalculateFrustumPlanes();

	//buffer update
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatricesID); //I'm talking to you uboMatricesID
	glBufferSubData(GL_UNIFORM_BUFFER, UBO_PADDING::MAT4, sizeof(Matrix4), viewMatrix); //offset = sizeof(Matrix4) because we put the projection in before
	glBindBuffer(GL_UNIFORM_BUFFER, 0); //unbind buffer
}

GEOMETRY::Ray CameraActor::WorldSpaceRayFromMouseCoords(float mouseX, float mouseY) {
	Vec3 mouseCoords(mouseX, mouseY, 1.0f);
	//Get a ray pointing into the world
	//We have the x, y pixel coordinates
	//Need to convert this into world space to build our ray
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	Matrix4 ndc = MMath::viewportNDC(viewport[2], viewport[3]);
	Matrix4 rayTransform = MMath::inverse(ndc * projectionMatrix);

	Vec3 rayWorldStart = Vec3();
	//Vec3 rayWorldStart = GetComponent<TransformComponent>()->GetPosition();
	Vec3 rayWorldDirection = VMath::normalize(rayTransform * mouseCoords);

	GEOMETRY::Ray rayWorldSpace{ rayWorldStart, rayWorldDirection };
	return rayWorldSpace;
}

void CameraActor::CalculateFrustumPlanes() {
	//make the planes out of the matrix of perspective * view
	Matrix4 cameraMatrix = projectionMatrix * viewMatrix;

	//http://apollo.humber.ca/~fielder/Literature/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	Plane plane;
	//left clipping plane
	plane.x = cameraMatrix.m[3] + cameraMatrix.m[0];
	plane.y = cameraMatrix.m[7] + cameraMatrix.m[4];
	plane.z = cameraMatrix.m[11] + cameraMatrix.m[8];
	plane.d = cameraMatrix.m[15] + cameraMatrix.m[12];
	frustumPlanes[0] = plane;
	//right clipping plane
	plane.x = cameraMatrix.m[3] - cameraMatrix.m[0];
	plane.y = cameraMatrix.m[7] - cameraMatrix.m[4];
	plane.z = cameraMatrix.m[11] - cameraMatrix.m[8];
	plane.d = cameraMatrix.m[15] - cameraMatrix.m[12];
	frustumPlanes[1] = plane;
	//bottom clipping plane
	plane.x = cameraMatrix.m[3] + cameraMatrix.m[1];
	plane.y = cameraMatrix.m[7] + cameraMatrix.m[5];
	plane.z = cameraMatrix.m[11] + cameraMatrix.m[9];
	plane.d = cameraMatrix.m[15] + cameraMatrix.m[13];
	frustumPlanes[2] = plane;
	//top clipping plane
	plane.x = cameraMatrix.m[3] - cameraMatrix.m[1];
	plane.y = cameraMatrix.m[7] - cameraMatrix.m[5];
	plane.z = cameraMatrix.m[11] - cameraMatrix.m[9];
	plane.d = cameraMatrix.m[15] - cameraMatrix.m[13];
	frustumPlanes[3] = plane;
	//near clipping plane
	plane.x = cameraMatrix.m[3] + cameraMatrix.m[2];
	plane.y = cameraMatrix.m[7] + cameraMatrix.m[6];
	plane.z = cameraMatrix.m[11] + cameraMatrix.m[10];
	plane.d = cameraMatrix.m[15] + cameraMatrix.m[14];
	frustumPlanes[4] = plane;
	//far clipping plane
	plane.x = cameraMatrix.m[3] - cameraMatrix.m[2];
	plane.y = cameraMatrix.m[7] - cameraMatrix.m[6];
	plane.z = cameraMatrix.m[11] - cameraMatrix.m[10];
	plane.d = cameraMatrix.m[15] - cameraMatrix.m[14];
	frustumPlanes[5] = plane;
}

void CameraActor::OnDestroy() {
	glDeleteBuffers(1, &uboMatricesID); //protect the memory and delete the buffer
	isCreated = false;
}