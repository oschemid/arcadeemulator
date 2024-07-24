#include "loader_dsk.h"
#include "disk.h"
#include <filesystem>


using namespace aos::amstrad::disk;


Disk* LoaderDsk::load(const string& filename)
{
	File file(filename);
	uint8_t* buffer = new uint8_t[file.getSize()];
	file.read(0, file.getSize(), buffer);

	Disk* disk = nullptr;
	if (std::equal(buffer, buffer + 8, "MV - CPC")) {
		disk = load_standardformat(buffer);
	}
	else if (std::equal(buffer, buffer+8, "EXTENDED"))
	{
		disk = load_extendedformat(buffer);
	}
	delete[] buffer;
	return disk;
}

Disk* LoaderDsk::load_standardformat(const uint8_t* buffer)
{
	uint8_t nb_tracks = buffer[0x30];
	uint8_t nb_sides = buffer[0x31];

	Disk* disk = new Disk(nb_tracks, nb_sides);

	uint16_t track_size = buffer[0x32] + (buffer[0x33] << 8);

	for (uint8_t i = 0; i < nb_tracks * nb_sides; ++i)
	{
		if (!load_track(disk, buffer + 0x100 + i * track_size)) {
			delete disk;
			return nullptr;
		}
	}
	return disk;
}

Disk* LoaderDsk::load_extendedformat(const uint8_t* buffer)
{
	uint8_t nb_tracks = buffer[0x30];
	uint8_t nb_sides = buffer[0x31];

	Disk* disk = new Disk(nb_tracks, nb_sides);

	uint16_t track_size = 0;
	uint32_t offset = 0x100;
	for (uint8_t i = 0; i < nb_tracks * nb_sides; ++i)
	{
		track_size = buffer[0x34 + i] << 8;
		if (!load_track(disk, buffer + offset, false)) {
			delete disk;
			return nullptr;
		}
		offset += track_size;
	}
	return disk;
}

bool LoaderDsk::load_track(Disk* disk, const uint8_t* buffer, bool fixed_size)
{
	if (std::equal(buffer, buffer + 0x0c, "Offset-Info\r\n"))
		return true;
	if (!std::equal(buffer, buffer + 0x0b, "Track-Info\r\n"))
		return false;
	uint8_t track_number = buffer[0x10];
	uint8_t side_number = buffer[0x11];
//	uint16_t sector_size = 128<<buffer[0x14];
	uint8_t nb_sectors = buffer[0x15];

	Track::Ptr track = std::make_unique<Track>(nb_sectors);
	uint16_t offset = 0x100;
	for (uint8_t i = 0; i < nb_sectors; ++i)
	{
		offset += load_sector(track, buffer + 0x18 + 8 * i, buffer + offset, fixed_size);
	}

	disk->FillTrack(side_number, track_number, std::move(track));
	return true;
}

uint16_t LoaderDsk::load_sector(Track::Ptr& track, const uint8_t* info, const uint8_t* data, bool fixed_size)
{
	uint8_t track_number = info[0];
	uint8_t side_number = info[1];
	uint8_t sector_number = info[2];
	uint8_t sector_size = info[3];
	uint8_t status_1 = info[4];
	uint8_t status_2 = info[5];
	uint16_t data_size = (fixed_size)? 128<<sector_size : info[7] << 8 | info[6];

	Sector::Ptr sector = std::make_unique<Sector>(sector_number, sector_size, data_size, status_1, status_2);
	sector->FillData(data);
	track->FillSector(std::move(sector));
	return data_size;
}