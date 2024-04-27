#pragma once
#include "emulator.h"

using aos::emulator::GameConfiguration;


static aos::emulator::GameConfiguration jumpshot_configuration = {
		.switches = {{ "dsw1-0", 3, "Time", {"N/A", "4mins", "2mins", "3mins"}},
					 { "dsw1-2", 0, "Player 1 Skin", {"Darker", "Lighter"} },
					 { "dsw1-3", 0, "Player 2 Skin", {"Darker", "Lighter"} },
					 { "dsw1-4", 1 },
					 { "dsw1-5", 0 },
					 { "dsw1-6", 3 },
		  }
};
static aos::emulator::GameConfiguration jumpshotp_configuration = {
		.switches = {{ "dsw1-0", 3, "Time", {"N/A", "Short", "Average", "Above average"}},
					 { "dsw1-3", 0, "Player Skin", {"Darker", "Lighter"} },
					 { "dsw1-4", 1 },
					 { "dsw1-5", 0 },
					 { "dsw1-6", 3 },
		  }
};
static aos::emulator::GameConfiguration mrtnt_configuration = {
		.switches = {{ "dsw1-0", 3 },
					 { "dsw1-2", 2, "Lives", {"5", "4", "3", "2"} },
					 { "dsw1-4", 0, "Bonus", {"150000 points", "125000 points", "100000 points", "75000 points"} },
					 { "dsw1-6", 1 },
					 { "dsw1-7", 1 },
		  }
};

static aos::emulator::GameConfiguration mspacman_configuration = {
		.switches = {{ "dsw1-0", 1 },
					 { "dsw1-2", 2, "Lives", {"1", "2", "3", "5"} },
					 { "dsw1-4", 0, "Bonus", {"10000 points", "15000 points", "20000 points", "no"} },
					 { "dsw1-6", 1, "Difficulty", {"Hard", "Normal"} },
					 { "dsw1-7", 1}
		  }
};

static aos::emulator::GameConfiguration woodpecker_configuration = {
	.switches = {{ "dsw1-0", 1},
				 { "dsw1-2", 2, "Lives", {"1", "2", "3", "5"} },
				 { "dsw1-4", 1, "Bonus", {"5000", "10000", "15000", "None"} }
	}
};
static aos::emulator::GameConfiguration lizwiz_configuration = {
	.switches = {{ "dsw1-0", 3},
				 { "dsw1-2", 2, "Lives", {"5", "4", "3", "2"} },
				 { "dsw1-4", 1, "Bonus", {"150000", "125000", "100000", "75000"} },
				 { "dsw1-6", 1, "Difficulty", {"Hard", "Normal"} },
				 { "dsw1-7", 1}
	}
};
static aos::emulator::GameConfiguration ponpoko_configuration = {
	.switches = {{ "dsw1-0", 2, "Bonus", {"None", "10000", "30000", "50000"} },
				 { "dsw1-2", 3 },
				 { "dsw1-4", 1, "Lives", {"2", "3", "4", "5"}},
				 { "dsw1-6", 1},
				 { "dsw1-7", 1}
	}
};