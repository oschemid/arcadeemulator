#include "disk.h"
#include "assert.h"


using namespace aos::amstrad::disk;


Sector::Sector(const uint8_t id, const uint8_t size, const uint16_t realsize, const uint8_t status1, const uint8_t status2) :
	_id{ id }, _size { size }, _status1{ status1 }, _status2{ status2 }, _realsize { realsize }
{
	_data = new uint8_t[_realsize];
}

Sector::~Sector()
{
	if (_data)
		delete[] _data;
}

void Sector::FillData(const uint8_t* data)
{
	std::copy(data, data + _realsize, _data);
}

SectorInfo Sector::sector_info()
{
	return { .sector = _id,
			 .size = _size,
		 	 .status1 = _status1,
			 .status2 = _status2 };
}

Track::Track(const uint8_t nb_sectors) :
	_nb_sectors{ nb_sectors }
{
	_sectors.resize(0);
}

void Track::FillSector(Sector::Ptr sector)
{
	_sectors.push_back(std::move(sector));
}

Disk::Disk(const uint8_t nb_tracks, const uint8_t nb_sides) :
	_nb_tracks{ nb_tracks }, _nb_sides{ nb_sides }
{
	assert((_nb_sides == 1)||(_nb_sides == 2));

	_tracks_1.resize(_nb_tracks);
	if (_nb_sides == 2)
		_tracks_2.resize(_nb_tracks);
}

void Disk::FillTrack(const uint8_t side, const uint8_t trackid, Track::Ptr track)
{
	if (side == 0)
		_tracks_1[trackid] = std::move(track);
	if ((side == 1) && (_nb_sides == 2))
		_tracks_2[trackid] = std::move(track);
}

SectorInfo Track::sector_info()
{
	return _sectors[_current_sector]->sector_info();
}

uint8_t* Track::read_data(const uint8_t sector, const uint8_t size)
{
	for (size_t idx = 0; auto& sect : _sectors)
	{
		SectorInfo si = sect->sector_info();
		if ((si.sector == sector)&&(si.size == size)) {
			_current_sector = idx;
			return sect->data();
		}
		++idx;
	}
	return nullptr;
}

bool Disk::seek(const uint8_t track)
{
	if (track >= _nb_tracks)
		return false;
	_current_track = track;
	return true;
}

SectorInfo Disk::sector_info(const uint8_t side)
{
	if (side >= _nb_sides)
		return { .status = 0 };

	SectorInfo result = (side == 0) ? _tracks_1[_current_track]->sector_info() : _tracks_2[_current_track]->sector_info();
	result.status = 1;
	result.track = _current_track;
	result.side = side;
	return result;
}

uint8_t* Disk::read_data(const uint8_t side, const uint8_t sector, const uint8_t size)
{
	if (side >= _nb_sides)
		return nullptr;
	return (side == 0) ? _tracks_1[_current_track]->read_data(sector, size) : _tracks_2[_current_track]->read_data(sector, size);
}