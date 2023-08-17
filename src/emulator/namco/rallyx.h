#pragma once
#include "pacmansystem.h"
#include "emulator.h"
#include "gpu/rallyxgpu.h"


namespace aos::namco
{
	class RallyX : public PacmanSystem<RallyXGpu>
	{
	public:
		RallyX(const vector<aos::emulator::RomConfiguration>&,
			const aos::emulator::GameConfiguration&);
		virtual ~RallyX();

	protected:
		void mapping() override;

		uint8_t _radarattr[0x10];
	};
}
