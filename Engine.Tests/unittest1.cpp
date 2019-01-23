#include "stdafx.h"
#include "CppUnitTest.h"
#include "../Engine.Serialization/SceneSerDe.h"
#include "../Engine.Serialization/ResourcePackSerDe.h"
#include <DirectXMath.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EngineTests
{
	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(SaveScene)
		{
			std::vector<EntityType> entities;
			std::vector<std::string> mats = { ("floor"), ("wood"), ("scratched"), "bronze" };
			for (int i = 0; i < 10; ++i)
			{
				entities.push_back(EntityType
					{
						XMFLOAT3((float)i,0.f,0.f),
						XMFLOAT3(1.f,1.f,1.f),
						XMFLOAT3(),
						mats[i % 4],
						"sphere",
						true
					}
				);
			}

			entities[0].MaterialID = "floor";

			entities[8].MeshID = "hammer";
			entities[7].MeshID = "sw";
			entities[6].MeshID = "sw";

			entities[8].Rotation = Vector3{ (XMFLOAT3(-XM_PIDIV2 / 4, -XM_PIDIV2 / 4, 0.f)) };
			entities[8].Position = Vector3{ XMFLOAT3(0, -1, 0) };
			entities[7].Position = Vector3{ XMFLOAT3(15, 1, 5) };
			entities[6].Position = Vector3{ XMFLOAT3(0, 2, 5) };

			//entities[.->SetScale(XMFLOAT3(0.01f, 0.01f, 0.01f));
			entities[7].Scale = { (XMFLOAT3(0.01f, 0.01f, 0.01f)) };
			entities[6].Scale = { (XMFLOAT3(0.01f, 0.01f, 0.01f)) };

			entities[8].MaterialID = "hammer";
			entities[7].MaterialID = "arc";
			entities[6].MaterialID = "arc";

			entities[9].CastShadows = (false);
			entities[9].MeshID = "quad";
			entities[9].Position = Vector3{ (XMFLOAT3(5, -1, -5)) };
			entities[9].Scale = Vector3{ (XMFLOAT3(15, 15, 15)) };
			entities[9].MaterialID = "cement";

			Scene scene = { entities };
			SceneSerDe::SaveScene(scene, "scene.json");
			// TODO: Your test code here
		}

		TEST_METHOD(LoadScene)
		{
			auto scene = SceneSerDe::LoadScene("scene.json");
		}

		TEST_METHOD(SaveResources)
		{
			std::vector<MaterialType> materials =
			{
				{
					"floor",
					"../../Assets/Textures/floor_albedo.png" ,
					"../../Assets/Textures/floor_normals.png" ,
					"../../Assets/Textures/floor_roughness.png" ,
					"../../Assets/Textures/floor_metal.png"
				},
				{
					"wood",
					"../../Assets/Textures/wood_albedo.png" , //1
					"../../Assets/Textures/wood_normals.png" ,
					"../../Assets/Textures/wood_roughness.png" ,
					"../../Assets/Textures/wood_metal.png",
				},
				{
					"scratched",
					"../../Assets/Textures/scratched_albedo.png" , //2
					"../../Assets/Textures/scratched_normals.png" ,
					"../../Assets/Textures/scratched_roughness.png" ,
					"../../Assets/Textures/scratched_metal.png"
				},
				{
					"bronze",
					"../../Assets/Textures/bronze_albedo.png" , //3
					"../../Assets/Textures/bronze_normals.png" ,
					"../../Assets/Textures/bronze_roughness.png" ,
					"../../Assets/Textures/bronze_metal.png",
				},
				{
					"cement",
					"../../Assets/Textures/cement_albedo.png" , //4
					"../../Assets/Textures/cement_normals.png" ,
					"../../Assets/Textures/cement_roughness.png" ,
					"../../Assets/Textures/cement_metal.png",
				},
				{
					"arc",
					"../../Assets/Textures/asw_albedo.png" , //5
					"../../Assets/Textures/marble_normals.jpg" ,
					"../../Assets/Textures/marble_roughness.jpg",
					"../../Assets/Textures/marble_metal.png",
				},
				{
					"hammer",
					"../../Assets/Textures/hammer_albedo.png" , //6
					"../../Assets/Textures/hammer_normals.png" ,
					"../../Assets/Textures/cement_roughness.png",
					"../../Assets/Textures/hammer_metal.png"
				}
			};

			std::vector<MeshType> meshes = 
			{
				MeshType {"sphere", "../../Assets/sphere.obj"},
				MeshType {"quad", "../../Assets/quad.obj" },
				MeshType {"sw", "../../Assets/sw.obj"  },
				MeshType {"hammer", "../../Assets/hammer.obj"}
			};

			Resources rc = {materials, meshes};
			ResourcePackSerDe::SaveResourcePack("resources.json", rc);
			
		}

		TEST_METHOD(LoadResources)
		{
			auto rc = ResourcePackSerDe::LoadResources("resources.json");
		}

	};
}