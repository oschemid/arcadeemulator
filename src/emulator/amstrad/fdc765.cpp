#include "fdc765.h"


using namespace aos::amstrad;


FDC765::FDC765()
{
}

void FDC765::insert(disk::Disk* disk)
{
	_disk = disk;
}

uint8_t FDC765::read(const uint8_t address)
{
	if ((address & 0x81) == 0)
		return mainregister;
	if (current_phase == RESULTS)
	{
		if (current_result_idx == nb_results - 1)
			phase_idle();
		return results[current_result_idx++];
	}
	else if ((current_phase == EXECUTION)&&(data_size>0))
	{
		if (data_idx == data_size - 1)
			phase_results();
		return data[data_idx++];
	}
	return 0xff;
}

void FDC765::write(const uint8_t address, const uint8_t data)
{
	if ((address & 0x81) != 1)
		return;

	switch (current_phase)
	{
	case IDLE:
		current_phase = COMMAND;
		decode_command(data);
		break;
	case COMMAND:
		decode_command_args(data);
		break;
	}
}

void FDC765::motor(const uint8_t flag)
{
	intstatus |= flag << 4;
}

void FDC765::decode_command(const uint8_t data)
{
	switch (data & 0x1f)
	{
	case 0x03:
		current_command = SPECIFY;
		command_args_expected = 3;
		break;
	case 0x04:
		current_command = SENSE_DRIVE;
		command_args_expected = 2;
		break;
	case 0X06:
		current_command = READ_DATA;
		command_args_expected = 8;
		break;
	case 0x07:
		current_command = RECALIBRATE;
		command_args_expected = 2;
		break;
	case 0x08:
		current_command = SENSE_INT_STATE;
		command_args_expected = 1;
		break;
	case 0x0a:
		current_command = READ_ID;
		command_args_expected = 2;
		break;
	case 0x0f:
		current_command = SEEK_TRACK;
		command_args_expected = 3;
		break;
	default:
		current_command = RECALIBRATE;
		break;
	}
	command_arg_idx = 0;
	mainregister |= CB;
	decode_command_args(data);
}

void FDC765::decode_command_args(const uint8_t data)
{
	command_args[command_arg_idx++] = data;
	if (command_arg_idx == command_args_expected) {
		current_phase = EXECUTION;
		mainregister |= EXM | DIO;
		execute_command();
	}
}

void FDC765::execute_command()
{
	switch (current_command)
	{
	case SPECIFY:
		phase_idle();
		break;
	case SENSE_DRIVE:
		results[0] = 0x20;
		nb_results = 1;
		current_result_idx = 0;
		phase_results();
		break;
	case READ_DATA:
		read_data();
		break;
	case RECALIBRATE:
		recalibrate();
		phase_idle();
		break;
	case SENSE_INT_STATE:
		sense_int();
		phase_results();
		break;
	case READ_ID:
		read_id();
		break;
	case SEEK_TRACK:
		_disk->seek(command_args[2]);
		physical_track = command_args[2];
		intstatus |= 1;
		phase_idle();
		break;
	}
}

void FDC765::read_id()
{
	const int fdd_index = command_args[1] & 3;
	const uint8_t side = (command_args[1] & 4) >> 2;
	sectorInfo = _disk->sector_info(side);

	status0 = fdd_index | 1 << 5;
	if (sectorInfo.status == 0) {
		status0 |= 1 << 3;
		status0 |= 1 << 6;
	}
	status1 = sectorInfo.status1;
	status2 = sectorInfo.status2;

	results[0] = status0;
	results[1] = status1;
	results[2] = status2;
	results[3] = sectorInfo.track;
	results[4] = sectorInfo.side;
	results[5] = sectorInfo.sector;
	results[6] = sectorInfo.size;
	nb_results = 7;
	current_result_idx = 0;
	phase_results();
}

void FDC765::read_data()
{
	if (data_size == 0)
	{
		const int fdd_index = command_args[1] & 3;
		const uint8_t side = (command_args[1] & 4) >> 2;
		const uint8_t sector = command_args[4];

		data = _disk->read_data(side, sector, command_args[5]);
		if (!data)
		{
			results[0] = 0x40;
			results[1] = 0x04;
			results[2] = 0;
			results[3] = command_args[2];
			results[4] = command_args[3];
			results[5] = command_args[4];
			results[5] = command_args[5];
			nb_results = 7;
			current_result_idx = 0;
			phase_results();
		}
		else
		{
			data_size = 128 << sectorInfo.size;
			data_idx = 0;
			sectorInfo = _disk->sector_info(side);

			status0 = fdd_index | 1 << 5;
			if (sectorInfo.status == 0) {
				status0 |= 1 << 3;
				status0 |= 1 << 6;
			}
			status1 = sectorInfo.status1;
			status2 = sectorInfo.status2;
			results[3] = sectorInfo.track;
			results[4] = sectorInfo.side;
			results[5] = sectorInfo.sector;
			results[6] = sectorInfo.size;
			results[0] = status0;
			results[1] = status1;
			results[2] = status2;
			nb_results = 7;
			current_result_idx = 0;
		}
	}
}
void FDC765::sense_int()
{
	if (intstatus == 0)
	{
		results[0] = 0x80;
		nb_results = 1;
		current_result_idx = 0;
		return;
	}
	for (uint8_t i = 0; i < 4; ++i)
	{
		if (intstatus & (1 << i))
		{
			results[0] = status0 + i;
			results[1] = physical_track;
			nb_results = 2;
			current_result_idx = 0;
			intstatus &= ~(0x11 << i);
			break;
		}
		if (intstatus & (0x10 << i))
		{
			results[0] = 0xc0 + i;
			results[1] = (i == 0) ? physical_track : 0;
			nb_results = 2;
			current_result_idx = 0;
			intstatus &= ~(0x10 << i);
			break;
		}
	}
}
void FDC765::recalibrate()
{
	const int fdd_index = command_args[1] & 3;
	status0 = fdd_index | 1 << 5;

	if ((!_disk)||(!_disk->seek(0))) {
		status0 |= 1 << 3;
		status0 |= 1 << 6;
	}
	physical_track = 0;
	intstatus |= 1 << fdd_index;
}

void FDC765::phase_idle()
{
	current_phase = IDLE;
	mainregister = RQM;
	data_size = 0;
}
void FDC765::phase_results()
{
	current_phase = RESULTS;
	mainregister = CB | RQM | DIO;
}
