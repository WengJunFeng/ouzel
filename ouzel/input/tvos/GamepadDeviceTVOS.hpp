// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_GAMEPADDEVICETVOS_HPP
#define OUZEL_INPUT_GAMEPADDEVICETVOS_HPP

#include <string>
#if defined(__OBJC__)
#  include <GameController/GameController.h>
typedef GCController* GCControllerPtr;
#else
#  include <objc/objc.h>
typedef id GCControllerPtr;
#endif

#include "input/GamepadDevice.hpp"
#include "input/Gamepad.hpp"

namespace ouzel
{
    namespace input
    {
        class GamepadDeviceTVOS final: public GamepadDevice
        {
        public:
            GamepadDeviceTVOS(InputSystem& initInputSystem,
                              uint32_t initId,
                              GCControllerPtr initController);

            void setAbsoluteDPadValues(bool absoluteDPadValues);
            bool isAbsoluteDPadValues() const;

            int32_t getPlayerIndex() const;
            void setPlayerIndex(int32_t playerIndex);

            inline GCControllerPtr getController() const { return controller; }

        private:
            GCControllerPtr controller;
            std::string name;
            bool attached = false;
        };
    } // namespace input
} // namespace ouzel

#endif // OUZEL_INPUT_GAMEPADDEVICETVOS_HPP
