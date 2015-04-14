#ifndef AIR_MODULE_H
#define AIR_MODULE_H

#include "libs/Module.h"
#include "Pwm.h"

class Air : public Module{
    public:
        Air();
        virtual ~Air() {};
        void on_module_loaded();
        void on_block_end(void* argument);
        void on_block_begin(void* argument);
        void on_play(void* argument);
        void on_pause(void* argument);
        void on_gcode_execute(void* argument);

    private:
        Pwm air_pin;
        struct {
            bool air_on:1;     // Air status
        };
};

#endif
