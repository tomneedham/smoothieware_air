#include "libs/Module.h"
#include "libs/Kernel.h"
#include "modules/communication/utils/Gcode.h"
#include "modules/robot/Stepper.h"
#include "Air.h"
#include "libs/nuts_bolts.h"
#include "Config.h"
#include "StreamOutputPool.h"
#include "Block.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "libs/Pin.h"
#include "Gcode.h"

#define air_module_enable_checksum		CHECKSUM("air_module_enable")
#define air_module_pin_checksum		   CHECKSUM("air_module_pin")

Air::Air(){}

void Air::on_module_loaded() {

	// Check if the module should be loaded
	if( !THEKERNEL
		->config
		->value(air_module_enable_checksum)
		->by_default(false)
		->as_bool()
	){
		delete this;
		return;
	}

	// Get the pin variable from config
	this->valve_pin.from_string(
		THEKERNEL
		->config
		->value(air_module_pin_checksum)
		->by_default("nc")
		->as_string()
	)->as_output();


	if (!this->valve_pin.connected()){
		THEKERNEL->streams->printf("Error using this pin number. Air module disabled.\n");
		delete this;
		return;
	}

	// Register for kernel events
	this->register_for_event(ON_GCODE_EXECUTE);
	this->register_for_event(ON_PLAY);
	this->register_for_event(ON_PAUSE);
	this->register_for_event(ON_BLOCK_BEGIN);
	this->register_for_event(ON_BLOCK_END);
}

// Turn of the valve at the end of blocks
void  Air::on_block_end(void* argument){

	this->valve_pin.set(false);

}

// Turn on the valve if needed at the beginning of a block
void Air::on_block_begin(void* argument){

	if(this->valve_on && THEKERNEL->stepper->get_current_block()){
		this->valve_pin.set(true);
	} else {
		this->valve_pin.set(false);
	}

}

// Turn off the valve on pause
void Air::on_pause(void* argument){

	this->valve_pin.set(false);

}

// Turn on the valve if needed on the play event
void Air::on_play(void* argument){

	if(this->valve_on && THEKERNEL->stepper->get_current_block()){
		this->valve_pin.set(true);
	} else {
		this->valve_pin.set(false);
	}

}

// Turn valve on / off depending on the G Code supplied
void Air::on_gcode_execute(void* argument){

	Gcode* gcode = static_cast<Gcode*>(argument);
	this->valve_on = false;
	if( gcode->has_g){
		int code = gcode->g;
		if( code == 0 ){
			// No ink, since G0 code
			this->valve_pin.set(false);
			this->valve_on =  false;
		}else if( code >= 1 && code <= 3 ){
			// Ink flow, since G1,G2 or G3 code
			this->valve_on =  true;
		}
	}

}
