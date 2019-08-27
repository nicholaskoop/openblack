#include <Entities/EntityID.h>
#include <Entities/Components/Villager.h>
#include <optional>

namespace OpenBlack
{
namespace Entities
{
class EntityBuilder
{
  public:
	EntityBuilder& SetBuilding(VillageEthnicities ethnicity);
	EntityBuilder& SetVillager(VillageEthnicities ethnicity, VillagerTypes type, uint32_t age);
	EntityBuilder& SetPosition();
	EntityId Create();

	struct EntityResult
	{
		EntityId id;
		std::optional<Villager> villager;
	};
  private:

};
} // namespace Entities
} // namespace OpenBlack
