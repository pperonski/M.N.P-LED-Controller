#pragma once

#include "Mode.hpp"
#include "modes/Straps.hpp"
#include "modes/Static.hpp"
#include "modes/HTMLControl.hpp"
#include "modes/APIControl.hpp"
#include "modes/Pulse.hpp"

template <typename T>
mode::Mode *createMode()
{
    return new T();
}

typedef mode::Mode *(*ModeCreator)();

const ModeCreator modes_list[] = {
    // createMode<mode::Straps>,
    createMode<mode::HTML>,
    createMode<mode::Static>,
    createMode<mode::Pulse>,
    createMode<mode::API>};

extern mode::Mode *current_mode;

#define MODES_LIST_SIZE sizeof(modes_list) / sizeof(ModeCreator)