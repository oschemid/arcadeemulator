# Space Invaders Hardware Description

## References
[Computer Archeology](http://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html)

## Architecture
### CPU
- Intel 8080
### Memory Map
- 0000-1FFF 8K ROM
- 2000-23FF 1K RAM
- 2400-3FFF 7K Video RAM
- 4000-5FFF RAM mirror
### Video
- Resolution 256x224
- Frequency 60Hz.
- Black & White : one bit per pixel
- Monitor is rotated 90 degrees counter-clockwise: first byte is lower left, first row ends upper left,...
- Game uses colored transparencies : players's area is green and flying saucer line is red.
### Dedicated Shift Hardware
The machine uses a hardware shift register to help CPU.

	16 bit shift register:
	----------------------
	f              0	bit
	xxxxxxxxyyyyyyyy
	
	Writing to port 4 shifts x into y, and the new value into x, eg.
	$0000,
	write $aa -> $aa00,
	write $ff -> $ffaa,
	write $12 -> $12ff, ..
	
	Writing to port 2 (bits 0,1,2) sets the offset for the 8 bit result, eg.
	offset 0:
	rrrrrrrr		result=xxxxxxxx
	xxxxxxxxyyyyyyyy
	
	offset 2:
	  rrrrrrrr	result=xxxxxxyy
	xxxxxxxxyyyyyyyy
	
	offset 7:
	       rrrrrrrr	result=xyyyyyyy
	xxxxxxxxyyyyyyyy
	
	Reading from port 3 returns said result.

### Sound (WIP)
- Sounds are all discrete analog circuitry
- Triggered by bits in two output ports.
### Tilt
Such as on a flipper, if you shake the machine, you will get a TILT message and the game will end.
## Timing
- Based on an interrupt from the video system.
- When the video rendering reaches the middle of the screen an RST 1 is generated.
- When the rendering reaches the end of the screen an RST 2 is generated.
- The screen refresh rate is 60Hz. So each interrupt is executed 60 times a second.
## IO Ports (WIP)
### Inputs
Port 0 (Never used by the code)
- bit 0 DIP4 (Seems to be self-test-request read at power up)
- bit 1 Always 1
- bit 2 Always 1
- bit 3 Always 1
- bit 4 Fire
- bit 5 Left
- bit 6 Right
- bit 7 ? tied to demux port 7 ?

Port 1
- bit 0 = CREDIT (1 if deposit)
- bit 1 = 2P start (1 if pressed)
- bit 2 = 1P start (1 if pressed)
- bit 3 = Always 1
- bit 4 = 1P shot (1 if pressed)
- bit 5 = 1P left (1 if pressed)
- bit 6 = 1P right (1 if pressed)
- bit 7 = Not connected

Port 2
- bit 0 = DIP3 00 = 3 ships  10 = 5 ships
- bit 1 = DIP5 01 = 4 ships  11 = 6 ships
- bit 2 = Tilt
- bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
- bit 4 = P2 shot (1 if pressed)
- bit 5 = P2 left (1 if pressed)
- bit 6 = P2 right (1 if pressed)
- bit 7 = DIP7 Coin info displayed in demo screen 0=ON

Port 3
-  bit 0-7 Shift register data

### Outputs
Port 2:
- bit 0,1,2 Shift amount

Port 3: (discrete sounds)
- bit 0=UFO (repeats)        SX0 0.raw
- bit 1=Shot                 SX1 1.raw
- bit 2=Flash (player die)   SX2 2.raw
- bit 3=Invader die          SX3 3.raw
- bit 4=Extended play        SX4
- bit 5= AMP enable          SX5
- bit 6= NC (not wired)
- bit 7= NC (not wired)

Port 4
- bit 0-7 shift data (LSB on 1st write, MSB on 2nd)

Port 5:
- bit 0=Fleet movement 1     SX6 4.raw
- bit 1=Fleet movement 2     SX7 5.raw
- bit 2=Fleet movement 3     SX8 6.raw
- bit 3=Fleet movement 4     SX9 7.raw
- bit 4=UFO Hit              SX10 8.raw
- bit 5= NC (Cocktail mode control ... to flip screen)
- bit 6= NC (not wired)
- bit 7= NC (not wired)

Port 6:
- Watchdog ... read or write to reset