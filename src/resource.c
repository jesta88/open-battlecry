#include "resource.h"

void resource_init(resource_bank* bank, int32_t gold, int32_t metal, int32_t crystal, int32_t stone)
{
	bank->amount[RES_GOLD] = gold;
	bank->amount[RES_METAL] = metal;
	bank->amount[RES_CRYSTAL] = crystal;
	bank->amount[RES_STONE] = stone;
}

bool resource_can_afford(const resource_bank* bank, resource_cost cost)
{
	return bank->amount[RES_GOLD] >= cost.gold &&
	       bank->amount[RES_METAL] >= cost.metal &&
	       bank->amount[RES_CRYSTAL] >= cost.crystal &&
	       bank->amount[RES_STONE] >= cost.stone;
}

void resource_deduct(resource_bank* bank, resource_cost cost)
{
	bank->amount[RES_GOLD] -= cost.gold;
	bank->amount[RES_METAL] -= cost.metal;
	bank->amount[RES_CRYSTAL] -= cost.crystal;
	bank->amount[RES_STONE] -= cost.stone;
}

void resource_add(resource_bank* bank, uint8_t type, int32_t amount)
{
	if (type < RES_COUNT)
		bank->amount[type] += amount;
}
