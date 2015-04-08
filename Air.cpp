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
#include "PwmOut.h" // mbed.h lib

#define air_module_enable_checksum        CHECKSUM("air_module_enable")
#define air_module_pin_checksum           CHECKSUM("air_module_pin")

Air::Air(){
}

void Air::on_module_loaded() {
    if( !THEKERNEL->config->value( air_module_enable_checksum )->by_default(false)->as_bool() ){
        // as not needed free up resource
        delete this;
        return;
    }

    // Get smoothie-style pin from config
    Pin* air_pin = new Pin();
    air_pin->from_string(THEKERNEL->config->value(air_module_pin_checksum)->by_default("nc")->as_string())->as_output();

    if (air_pin == NULL)
    {
        THEKERNEL->streams->printf("Error: Air module cannot use: P%d.%d - Please use one of: (P2.0 - P2.5, P1.18, P1.20, P1.21, P1.23, P1.24, P1.26, P3.25, P3.26 only). Air module disabled.\n", air_pin->port_number, air_pin->pin);
        delete air_pin;
        delete this;
        return;
    }

    this->air_pin->set(false);

    //register for events
    this->register_for_event(ON_GCODE_EXECUTE);
    this->register_for_event(ON_PLAY);
    this->register_for_event(ON_PAUSE);
}

// Turn off air at end of block
void Air::on_block_end(void* argument){
    this->air_pin->set(false);
}

// Set laser power at the beginning of a block
void Air::on_block_begin(void* argument){
    if( this->air_on && THEKERNEL->stepper->get_current_block() ){
        this->air_pin->set(true);
    }
}

// Turn off air when paused
void Air::on_pause(void* argument){
    this->air_pin->set(false);
}

// On resume, turn on air if needed
void Air::on_play(void* argument){
    if( this->air_on && THEKERNEL->stepper->get_current_block() ){
        this->air_pin->set(true);
    }
}

// Turn on/off air as needed
void Air::on_gcode_execute(void* argument){
    Gcode* gcode = static_cast<Gcode*>(argument);
    this->air_on = false;
    if( gcode->has_g){
        int code = gcode->g;
        if( code == 0 ){                    // G0
            this->air_pin->set(false);
            this->air_on =  false;
        }else if( code >= 1 && code <= 3 ){ // G1, G2, G3
            this->air_on =  true;
        }
    }

}




