#pragma once
#include "types.h"


namespace aos::amstrad::disk
{
	struct SectorInfo
	{
		uint8_t status;
		uint8_t track;
		uint8_t side;
		uint8_t sector;
		uint8_t size;
		uint8_t status1;
		uint8_t status2;
	};

	class Sector
	{
	public:
		Sector() {}
		~Sector();
		Sector(const uint8_t, const uint8_t, const uint16_t, const uint8_t, const uint8_t);
		uint8_t id() const { return _id; };
		uint8_t* data() const { return _data; };

		using Ptr = std::unique_ptr<Sector>;

		void FillData(const uint8_t*);
		SectorInfo sector_info();
	protected:
		uint8_t _id{ 0 };
		uint8_t _size{ 0 };
		uint16_t _realsize{ 0 };
		uint8_t _status1{ 0 };
		uint8_t _status2{ 0 };

		uint8_t* _data{ nullptr };
	};
	class Track
	{
	public:
		Track(const uint8_t);
		void FillSector(Sector::Ptr);

		SectorInfo sector_info();
		uint8_t* read_data(const uint8_t, const uint8_t);

		using Ptr = std::unique_ptr<Track>;
	protected:
		uint8_t _nb_sectors{ 0 };
		uint8_t _current_sector{ 0 };

		vector<Sector::Ptr> _sectors;
	};
	class Disk
	{
	public:
		Disk(const uint8_t, const uint8_t);
		void FillTrack(const uint8_t, const uint8_t, Track::Ptr);

		bool seek(const uint8_t);
		SectorInfo sector_info(const uint8_t);
		uint8_t* read_data(const uint8_t, const uint8_t, const uint8_t);
	protected:
		uint8_t _nb_tracks{ 0 };
		uint8_t _nb_sides{ 0 };

		vector<Track::Ptr> _tracks_1;
		vector<Track::Ptr> _tracks_2;

		uint8_t _current_track{ 0 };
	};
}
