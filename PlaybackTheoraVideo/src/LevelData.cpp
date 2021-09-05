#include "LevelData.h"
#include "TVComponent.h"



CLevelData::CLevelData(Context* context) : 
	Object(context),
	scene(nullptr),
	camera(nullptr),
	cameraNode(nullptr),
	playerNode(nullptr),
	yaw_(0.0f),
	pitch_(0.0f)
{
	TVComponent::RegisterObject(context);
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CLevelData, HandleUpdate));
}

CLevelData::~CLevelData()
{
	scene.Reset();
	camera.Reset();
	cameraNode.Reset();
}


bool CLevelData::InitScene(String sceneName)
{
	return LoadSceneFromXMLFile(sceneName);
}
bool CLevelData::InitCameraFromSceneData(String camName) 
{
	bool ret = false;

	if (scene)
	{
		cameraNode = scene->GetChild(camName, true);
		if (cameraNode) 
		{
			camera = cameraNode->GetComponent<Camera>();
			if (camera) 
			{
				SharedPtr<Viewport> viewport = SharedPtr<Viewport>(new Viewport(context_, scene, camera));
				Renderer* renderer = GetSubsystem<Renderer>();
				renderer->SetViewport(0, viewport);
				ret = true;
			}
		}

		playerNode = scene->GetChild("Player", true);
	}
	return ret;
}

bool CLevelData::LoadSceneFromXMLFile(String sceneFileName) 
{
	scene = SharedPtr<Scene>(new Scene(context_));

	File sceneFile(context_,
		GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/" + sceneFileName,
		FILE_READ);

	return scene->LoadXML(sceneFile);
}

bool CLevelData::InitTVComponentForSceneNode(String nodeName)
{
	bool ret = false;
	if (scene) 
	{
		Node* tvNode = scene->GetChild(nodeName, true);
		if (tvNode)
		{
			tvc = tvNode->CreateComponent<TVComponent>();
			tvc->OpenFileName("bbb_theora_486kbit.ogv");
			//tvc->OpenFileName("trailer_400p.ogv"); //error!

			StaticModel* sm = tvNode->GetComponent<StaticModel>();
			tvc->SetOutputModel(sm);
			ret = true;
		}
	
	}

	return ret;

}


void CLevelData::ViewRotate(float timeStep)
{
	Input* input = GetSubsystem<Input>();

	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.1f;

	// Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
	IntVector2 mouseMove = input->GetMouseMove();

	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);

	// Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
	cameraNode->SetWorldRotation(Quaternion(pitch_, yaw_, 0.0f));
}

void CLevelData::ViewMove(float timeStep)
{
	if (!playerNode) return;
	const float MOVE_SPEED = 0.5f;

	Input* input = GetSubsystem<Input>();
	
	Quaternion worldRotation = cameraNode->GetWorldRotation();
	worldRotation.z_ = 0.0f;
	
	if (input->GetKeyDown(KEY_W))
		playerNode->Translate(worldRotation * Vector3::FORWARD * MOVE_SPEED * timeStep);

	if (input->GetKeyDown(KEY_S))
		playerNode->Translate(worldRotation * Vector3::BACK * MOVE_SPEED * timeStep);

	if (input->GetKeyDown(KEY_A))
		playerNode->Translate(worldRotation * Vector3::LEFT * MOVE_SPEED * timeStep);

	if (input->GetKeyDown(KEY_D))
		playerNode->Translate(worldRotation * Vector3::RIGHT * MOVE_SPEED * timeStep);

	if (input->GetKeyDown(KEY_1))
		tvc->Play();

	if (input->GetKeyDown(KEY_2))
		tvc->Pause();

	if (input->GetKeyDown(KEY_3))
		tvc->Stop();



}

void CLevelData::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;
	float timeStep = eventData[P_TIMESTEP].GetFloat();

	ViewRotate(timeStep);
	ViewMove(timeStep);
}
