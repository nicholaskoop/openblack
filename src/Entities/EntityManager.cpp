#include "EntityManager.h"

#include "AllMeshes.h"

#include <3D/Camera.h>
#include <3D/MeshPack.h>
#include <Entities/Components/Villager.h>
#include <Game.h>
#include <Graphics/ShaderManager.h>
#include <vector>

namespace OpenBlack::Entities
{
void EntityManager::AddVillager(EntityId id, Villager villager, Model model, Marker marker)
{
	_villagerComponents.insert(std::make_pair(id, villager));
	_modelComponents.insert(std::make_pair(id, model));
	_markerComponents.insert(std::make_pair(id, marker));
}

void EntityManager::DebugCreateEntities(float x, float y, float z)
{
	auto game = Game::instance();
	auto meshPack = game->GetMeshPack();

	std::shared_ptr<SkinnedModel> skinnedModel(meshPack.GetModel(362));

	auto id       = _villagerComponents.size();
	auto villager = Villager(id, VillageEthnicities::CELTIC, VillagerTypes::FARMER, VillagerTasks::IDLE, 18, 100, 0);
	auto model    = Model(id, skinnedModel);
	auto marker   = Marker(id, x, y, z);
	AddVillager(id, villager, model, marker);
}

void EntityManager::DrawModels(const Camera& camera, ShaderManager& shaderManager)
{
	for (const auto& kv : _modelComponents)
	{
		auto com    = kv.second;
		auto marker = _markerComponents.find(com.getEntityId())->second;

		auto _modelPosition = glm::vec3(marker.x, marker.y, marker.z);
		auto _modelRotation = glm::vec3(180.0f, 111.0f, 0.0f);
		auto _modelScale    = glm::vec3(2.5f);

		auto model            = com.GetModel();
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix           = glm::translate(modelMatrix, _modelPosition);

		modelMatrix = glm::rotate(modelMatrix, glm::radians(_modelRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(_modelRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(_modelRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		modelMatrix = glm::scale(modelMatrix, _modelScale);

		ShaderProgram* objectShader = shaderManager.GetShader("SkinnedMesh");
		objectShader->Bind();
		objectShader->SetUniformValue("u_viewProjection", camera.GetViewProjectionMatrix());
		objectShader->SetUniformValue("u_modelTransform", modelMatrix);
		model->Draw(objectShader);
	}
}

void EntityManager::Update()
{
	RemoveInactive();
}

void EntityManager::RemoveInactive()
{
	std::vector<EntityId> to_remove;

	for (const auto& kv : _entities)
	{
		auto id     = kv.first;
		auto entity = kv.second;

		if (!entity.active)
		{
			to_remove.push_back(id);
		}
	}

	for (const auto& id : to_remove)
	{
		_entities.erase(id);
		_villagerComponents.erase(id);
		_buildingComponents.erase(id);
		_modelComponents.erase(id);
		_markerComponents.erase(id);
	}
}

void EntityManager::UpdateVillagers()
{
	for (const auto& kv : _villagerComponents)
	{
		auto com = kv.second;
		// Update age
		// Update health
		// Update hunger
		// Update task
	}
}

void EntityManager::UpdateBuildings()
{
	for (const auto& kv : _buildingComponents)
	{
		auto com = kv.second;
	}
}

} // namespace OpenBlack::Entities
