#pragma once
#include "types.h"
#include "disk.h"


namespace aos::amstrad::disk
{
	class LoaderDsk
	{
	public:
		Disk* load(const string&);

	protected:
		Disk* load_standardformat(const uint8_t*);
		Disk* load_extendedformat(const uint8_t*);
		bool load_track(Disk*, const uint8_t*, bool=true);
		uint16_t load_sector(Track::Ptr&, const uint8_t*, const uint8_t*, bool=true);
	};
}