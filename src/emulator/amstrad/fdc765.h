#pragma once
#include "types.h"
#include "disk/disk.h"


namespace aos::amstrad
{
	class FDC765
	{
	public:
		FDC765();
		void insert(disk::Disk*);

		uint8_t read(const uint8_t);
		void write(const uint8_t, const uint8_t);
		void motor(const uint8_t);

	protected:
		disk::Disk* _disk{ nullptr };

		// Main Status Register
		uint8_t intstatus{ 0 };
		enum mainregister_flags { D0B = 1,
			   D1B = 2,
			   D2B = 4,
		       D3B = 8,
		       CB = 16,
		       EXM = 32,
		       DIO = 64,
		       RQM = 128 };
		uint8_t mainregister{ RQM };
		uint8_t status0{ 0 };
		uint8_t status1{ 0 };
		uint8_t status2{ 0 };
		disk::SectorInfo sectorInfo;
		uint8_t physical_track{ 0 };

		enum phase {
			IDLE,
			COMMAND,
			EXECUTION,
			RESULTS
		} current_phase{ IDLE };

		enum command {
			SPECIFY = 0x03,
			SENSE_DRIVE = 0x04,
			READ_DATA = 0x06,
			RECALIBRATE = 0x07,
			SENSE_INT_STATE = 0x08,
			READ_ID = 0x0a,
			SEEK_TRACK = 0x0f
		} current_command;
		uint8_t command_args[10];
		uint8_t command_arg_idx{ 0 };
		uint8_t command_args_expected{ 0 };
		void decode_command(const uint8_t);
		void decode_command_args(const uint8_t);
		void execute_command();
		void recalibrate();
		void sense_int();
		void read_id();
		void read_data();

		uint8_t* data;
		uint16_t data_size{ 0 };
		uint16_t data_idx{ 0 };
		uint8_t results[10];
		uint8_t current_result_idx{ 0 };
		uint8_t nb_results{ 0 };

		void phase_idle();
		void phase_results();
	};
}
